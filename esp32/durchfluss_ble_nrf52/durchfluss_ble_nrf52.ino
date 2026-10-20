// Durchfluss-Sender Güllefass – Arm Eggiwil AG – Version für Seeed XIAO nRF52840
//
// Zählt die Impulse des KROHNE OPTIFLUX 1000 (Impulsausgang 8–30 V DC, 1 Impuls = 1 Liter)
// und sendet jede Sekunde den Durchfluss per Bluetooth Low Energy an die Karten-Seite.
// Funkprotokoll verschlüsselt (siehe unten) – die ESP32-Version sendet noch Klartext und wird von der App
// nicht mehr angenommen.
//
// Hardware: Seeed XIAO nRF52840 (Distrelec 303-83-819) auf der Trägerplatine (PLATINE.md),
//           Impuls über Optokoppler an D10.
// Software: Arduino IDE, Boardverwalter-URL von Seeed eintragen und «Seeed nRF52 Boards» installieren,
//           Board «Seeed XIAO nRF52840» wählen (nicht die «mbed»-Variante – nur diese enthält «bluefruit.h»).
//
// Gesendete Werte: Liter pro Minute, Liter gesamt seit Einschalten, Schwankung ± m³/h.
// Die Schwankung kommt aus den Abständen der einzelnen Impulse in der letzten Sekunde. Schwankt der
// Durchfluss in Sekundenbruchteilen stark (z. B. ±20 m³/h), wird Luft zum Reinigen eingeblasen
// («Ausblasen») – die App wertet das dann nicht als Durchfluss.
//
// Verschlüsselung (nur unsere App kann mitlesen), AES-128 mit dem festen Schlüssel aus schluessel.h:
//  0. Bis zu 3 Handys gleichzeitig, jedes mit eigener Sitzung. Nichts wird dauerhaft gespeichert.
//  1. Beim Verbinden erzeugt das Modul 8 Zufallsbytes Ns für dieses Handy (Anmelde-Kennung …0003 lesen).
//  2. Die App liest Ns, wählt eigene 8 Zufallsbytes Na und schreibt Na + Nachweis (8 Bytes)
//     = AES(KA, Ns|Na). Falscher Nachweis oder keine Anmeldung innert 5 s: Modul trennt die Verbindung.
//  3. Sitzungsschlüssel KSe = AES(KE, Ns|Na), KSm = AES(KM, Ns|Na); KA/KE/KM = AES(Schlüssel, 'A'/'E'/'M').
//  4. Jede Sekunde eine Meldung (20 Bytes): Zähler (4) | verschlüsselte Werte (8) | Prüfzeichen (8).
//     Werte = l/min (u16) | Liter gesamt (u32) | Schwankung (u16), XOR AES(KSe, Zähler|0…).
//     Prüfzeichen = AES(KSm, Zähler|verschlüsselte Werte|0…). Zahlen jeweils Little Endian.
// Ohne schluessel.h lässt sich das Programm nicht übersetzen (Datei wird bewusst nicht veröffentlicht).

#include <bluefruit.h>
#include "schluessel.h"
#include "sitzung.h"

// ---------- Einstellungen ----------
const int      IMPULS_PIN       = D10;   // Optokoppler-Ausgang
const float    LITER_PRO_IMPULS = 1.0;   // am Messumformer eingestellt: 1 Impuls = 1 Liter
const uint32_t ENTPRELL_US      = 2000;  // Impulse kürzer als 2 ms Abstand ignorieren
const uint32_t FENSTER_MS       = 3000;  // Durchfluss = Mittelwert über die letzten 3 Sekunden
const uint32_t SLOT_MS          = 250;

// Bluetooth-Name «SpreadLink-1234»: die 4-stellige Nummer kommt aus der einmaligen Seriennummer des Chips
// (FICR DEVICEID) – jedes Modul hat so automatisch seine eigene Nummer, die auch nach Updates gleich bleibt.
#define NAMENSTEIL "SpreadLink-"
char geraetename[24];
uint16_t geraeteNummer() {
  uint32_t id = NRF_FICR->DEVICEID[0] ^ NRF_FICR->DEVICEID[1];
  return id % 10000;
}

// UUIDs wie in der ESP32-Version (6b1f0001-/6b1f0002-4a3b-4d52-9a2e-3c1d5e7f8a90), Bytes rückwärts
const uint8_t SERVICE_UUID[16] = { 0x90, 0x8a, 0x7f, 0x5e, 0x1d, 0x3c, 0x2e, 0x9a, 0x52, 0x4d, 0x3b, 0x4a, 0x01, 0x00, 0x1f, 0x6b };
const uint8_t WERT_UUID[16]    = { 0x90, 0x8a, 0x7f, 0x5e, 0x1d, 0x3c, 0x2e, 0x9a, 0x52, 0x4d, 0x3b, 0x4a, 0x02, 0x00, 0x1f, 0x6b };

const uint8_t ANMELDE_UUID[16] = { 0x90, 0x8a, 0x7f, 0x5e, 0x1d, 0x3c, 0x2e, 0x9a, 0x52, 0x4d, 0x3b, 0x4a, 0x03, 0x00, 0x1f, 0x6b };

BLEService        dienst(SERVICE_UUID);
BLECharacteristic wert(WERT_UUID);
BLECharacteristic anmeldung(ANMELDE_UUID);

// ---------- Verschlüsselung ----------
// Bis zu 3 Handys gleichzeitig, jedes mit eigener Anmeldung und eigenen Sitzungsschlüsseln.
// Alles nur im Arbeitsspeicher (keine Kopplung, nichts im Flash): aus- und einstecken = alles vergessen.
// (struct Sitzung steht in sitzung.h, weil die Arduino-IDE Funktionsdeklarationen vor den Code setzt)

// AES-128 macht die Funkeinheit des nRF52840 (über das SoftDevice) – kein Zusatzprogramm nötig
void aes(const uint8_t key[16], const uint8_t ein[16], uint8_t aus[16]) {
  nrf_ecb_hal_data_t d;
  memcpy(d.key, key, 16); memcpy(d.cleartext, ein, 16);
  sd_ecb_block_encrypt(&d);
  memcpy(aus, d.ciphertext, 16);
}
void zufall(uint8_t* p, uint8_t n) {
  while (sd_rand_application_vector_get(p, n) != NRF_SUCCESS) yield();
}
void le32(uint8_t* p, uint32_t v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; p[3] = v >> 24; }
void le16(uint8_t* p, uint16_t v) { p[0] = v; p[1] = v >> 8; }

uint8_t kA[16], kE[16], kM[16];          // aus dem Schlüssel abgeleitet
Sitzung sitz[MAX_HANDYS];

Sitzung* finde(uint16_t h) {
  for (int i = 0; i < MAX_HANDYS; i++) if (sitz[i].h == h) return &sitz[i];
  return nullptr;
}

void ableiten(char art, uint8_t aus[16]) {
  uint8_t b[16] = { 0 }; b[0] = art;
  aes(SCHLUESSEL, b, aus);
}

void beiVerbindung(uint16_t h) {
  Sitzung* s = finde(BLE_CONN_HANDLE_INVALID);
  if (!s) { Bluefruit.disconnect(h); return; }
  s->angemeldet = false;
  zufall(s->ns, 8);
  s->seit = millis();
  s->h = h;
  // weiter sichtbar bleiben, solange noch ein Platz frei ist
  if (Bluefruit.Periph.connected() < MAX_HANDYS) Bluefruit.Advertising.start(0);
}
void beiTrennung(uint16_t h, uint8_t grund) {
  Sitzung* s = finde(h);
  if (s) { memset(s, 0, sizeof(Sitzung)); s->h = BLE_CONN_HANDLE_INVALID; }
}
// Lesen der Anmelde-Kennung: jedes Handy bekommt seine eigene Zufallszahl Ns
void beiAnmeldeLesen(uint16_t h, BLECharacteristic* c, ble_gatts_evt_read_t* req) {
  static uint8_t leer[8] = { 0 };
  Sitzung* s = finde(h);
  ble_gatts_rw_authorize_reply_params_t r = { .type = BLE_GATTS_AUTHORIZE_TYPE_READ };
  r.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
  r.params.read.update = 1;
  r.params.read.offset = 0;
  r.params.read.len = 8;
  r.params.read.p_data = s ? s->ns : leer;
  sd_ble_gatts_rw_authorize_reply(h, &r);
}
void beiAnmeldung(uint16_t h, BLECharacteristic* c, uint8_t* d, uint16_t n) {
  Sitzung* s = finde(h);
  if (!s || s->angemeldet) return;
  uint8_t b[16], p[16], diff = 0;
  memcpy(b, s->ns, 8);
  if (n == 16) {
    memcpy(b + 8, d, 8);
    aes(kA, b, p);
    for (int i = 0; i < 8; i++) diff |= p[i] ^ d[8 + i];   // ohne frühen Abbruch vergleichen
  }
  if (n != 16 || diff) { Serial.println("Anmeldung abgelehnt"); Bluefruit.disconnect(h); return; }
  aes(kE, b, s->kSe); aes(kM, b, s->kSm);
  s->nr = 0;
  s->angemeldet = true;
  Serial.println("App angemeldet");
}

// eine verschlüsselte Meldung (20 Bytes) für eine Sitzung bilden
void verpacken(Sitzung* s, uint16_t lmin, uint32_t total, uint16_t spread, uint8_t pk[20]) {
  uint8_t w[8], b[16] = { 0 }, x[16];
  le16(w, lmin); le32(w + 2, total); le16(w + 6, spread);
  le32(b, ++s->nr);
  aes(s->kSe, b, x);
  memcpy(pk, b, 4);
  for (int i = 0; i < 8; i++) pk[4 + i] = w[i] ^ x[i];
  memset(b, 0, 16); memcpy(b, pk, 12);
  aes(s->kSm, b, x);
  memcpy(pk + 12, x, 8);
}

void druckeHex(const char* name, const uint8_t* p, int n) {
  Serial.print(name);
  for (int i = 0; i < n; i++) { if (p[i] < 16) Serial.print('0'); Serial.print(p[i], HEX); }
  Serial.println();
}
// Selbsttest beim Start: AES nach FIPS-197 und eine Testmeldung (mit der App vergleichbar)
void selbsttest() {
  uint8_t k[16], t[16], c[16];
  for (int i = 0; i < 16; i++) { k[i] = i; t[i] = i * 0x11; }
  aes(k, t, c);
  druckeHex("AES-Test (soll 69C4E0D86A7B0430D8CDB78070B4C55A): ", c, 16);
  uint8_t s[16], pk[20];
  Sitzung test = {};
  for (int i = 0; i < 16; i++) s[i] = i + 1;   // Ns = 01..08, Na = 09..10
  aes(kA, s, c); druckeHex("Test-Nachweis: ", c, 8);
  aes(kE, s, test.kSe); aes(kM, s, test.kSm);
  verpacken(&test, 2950, 18420, 3, pk); druckeHex("Test-Meldung: ", pk, 20);
}

// ---------- Impulse zählen ----------
volatile uint32_t impulse = 0;
volatile uint32_t letzterImpulsUs = 0;
const int NZEIT = 256;                       // Zeitpunkte der letzten Impulse (reicht bis ca. 900 m³/h)
volatile uint32_t impulsZeit[NZEIT];

void beiImpuls() {
  uint32_t jetzt = micros();
  if (jetzt - letzterImpulsUs >= ENTPRELL_US) {
    impulsZeit[impulse % NZEIT] = jetzt;
    impulse++;
    letzterImpulsUs = jetzt;
  }
}

uint32_t leseImpulse() {
  return impulse;  // 32-Bit-Lesen ist auf dem nRF52840 atomar
}

// Schwankung des Durchflusses in der letzten Sekunde, in ± m³/h:
// Momentan-Durchfluss aus jedem Impulsabstand, dann halbe Spanne zwischen 10 % und 90 % (robust gegen Ausreisser).
float schwankung() {
  static float q[NZEIT];
  uint32_t n = leseImpulse(), jetzt = micros();
  int k = 0;
  for (int32_t j = 0; j < NZEIT - 2 && (int32_t)n - 1 - j >= 1; j++) {
    uint32_t i = n - 1 - j;
    uint32_t t1 = impulsZeit[i % NZEIT], t0 = impulsZeit[(i - 1) % NZEIT];
    if (jetzt - t0 > 1000000UL) break;       // nur Abstände ganz innerhalb der letzten Sekunde
    uint32_t d = t1 - t0;
    if (d > 0) q[k++] = 3.6e6f * LITER_PRO_IMPULS / d;   // 1 Liter in d µs -> m³/h
  }
  if (k < 4) return 0;
  for (int a = 1; a < k; a++) { float v = q[a]; int b = a - 1; while (b >= 0 && q[b] > v) { q[b + 1] = q[b]; b--; } q[b + 1] = v; }
  return (q[k * 9 / 10] - q[k / 10]) / 2;
}

// ---------- Ringpuffer für den Mittelwert ----------
const int SLOTS = FENSTER_MS / SLOT_MS + 1;
uint32_t zaehler[SLOTS];
uint32_t zeit[SLOTS];
int pos = 0;
bool voll = false;

void startWerbung() {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addService(dienst);
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);  // nach Verbindungsabbruch wieder sichtbar
  Bluefruit.Advertising.setInterval(160, 800);      // Einheiten à 0.625 ms
  Bluefruit.Advertising.start(0);                   // dauerhaft
}

void setup() {
  Serial.begin(115200);
  pinMode(IMPULS_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IMPULS_PIN), beiImpuls, FALLING);

  for (int i = 0; i < MAX_HANDYS; i++) { memset(&sitz[i], 0, sizeof(Sitzung)); sitz[i].h = BLE_CONN_HANDLE_INVALID; }
  Bluefruit.begin(MAX_HANDYS, 0);                    // bis zu 3 Handys gleichzeitig
  snprintf(geraetename, sizeof(geraetename), NAMENSTEIL "%04u", geraeteNummer());
  Bluefruit.setName(geraetename);
  Bluefruit.setTxPower(4);  // maximale Sendeleistung für Reichweite bis in die Kabine

  ableiten('A', kA); ableiten('E', kE); ableiten('M', kM);
  Bluefruit.Periph.setConnectCallback(beiVerbindung);
  Bluefruit.Periph.setDisconnectCallback(beiTrennung);

  dienst.begin();
  wert.setProperties(CHR_PROPS_NOTIFY);              // nur verschlüsselte Meldungen, kein Klartext zum Auslesen
  wert.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  wert.setFixedLen(20);
  wert.begin();
  anmeldung.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE);
  anmeldung.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  anmeldung.setMaxLen(16);
  anmeldung.setWriteCallback(beiAnmeldung);
  anmeldung.setReadAuthorizeCallback(beiAnmeldeLesen, false);   // Ns pro Handy, direkt im Bluetooth-Ablauf
  anmeldung.begin();

  delay(1500);                                       // Zeit, um den seriellen Monitor zu öffnen
  selbsttest();
  startWerbung();

  zaehler[0] = 0;
  zeit[0] = millis();
  pos = 1;
  Serial.print("Durchfluss-Sender bereit: ");
  Serial.println(geraetename);
}

uint32_t naechsterSlot = 0;
uint32_t naechsteMeldung = 0;

void loop() {
  uint32_t ms = millis();

  if ((int32_t)(ms - naechsterSlot) >= 0) {
    naechsterSlot = ms + SLOT_MS;
    zaehler[pos] = leseImpulse();
    zeit[pos] = ms;
    pos = (pos + 1) % SLOTS;
    if (pos == 0) voll = true;
  }

  if ((int32_t)(ms - naechsteMeldung) >= 0) {
    naechsteMeldung = ms + 1000;
    uint32_t n = leseImpulse();
    int alt = voll ? pos : 0;                 // ältester Eintrag im Puffer
    uint32_t dt = ms - zeit[alt];
    float lmin = dt > 0 ? (n - zaehler[alt]) * LITER_PRO_IMPULS * 60000.0f / dt : 0;

    uint32_t q = (uint32_t)(lmin + 0.5f), total = (uint32_t)(n * LITER_PRO_IMPULS), sp = (uint32_t)(schwankung() + 0.5f);
    if (q > 65535) q = 65535;
    if (sp > 65535) sp = 65535;
    for (int i = 0; i < MAX_HANDYS; i++) {   // jedem angemeldeten Handy seine eigene verschlüsselte Meldung
      Sitzung* s = &sitz[i];
      if (s->h == BLE_CONN_HANDLE_INVALID || !s->angemeldet || !wert.notifyEnabled(s->h)) continue;
      uint8_t pk[20];
      verpacken(s, q, total, sp, pk);
      wert.notify(s->h, pk, 20);
    }
    char text[32];
    snprintf(text, sizeof(text), "%lu;%lu;%lu", (unsigned long)q, (unsigned long)total, (unsigned long)sp);
    Serial.println(text);                     // Klartext nur über USB
  }

  // fremde Geräte, die sich nicht anmelden, belegen keinen Platz – nach 5 s trennen
  for (int i = 0; i < MAX_HANDYS; i++) {
    Sitzung* s = &sitz[i];
    uint16_t h = s->h;
    if (h != BLE_CONN_HANDLE_INVALID && !s->angemeldet && millis() - s->seit > 5000) {
      Serial.println("keine Anmeldung - getrennt");
      s->seit = millis();
      Bluefruit.disconnect(h);
    }
  }
}
