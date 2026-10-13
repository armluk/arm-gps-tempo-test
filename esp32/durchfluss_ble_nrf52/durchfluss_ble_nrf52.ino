// Durchfluss-Sender Güllefass – Arm Eggiwil AG – Version für Seeed XIAO nRF52840
//
// Zählt die Impulse des KROHNE OPTIFLUX 1000 (Impulsausgang 8–30 V DC, 1 Impuls = 1 Liter)
// und sendet jede Sekunde den Durchfluss per Bluetooth Low Energy an die Karten-Seite.
// Gleiches Funkprotokoll wie die ESP32-Version: die Karten-App muss nicht angepasst werden.
//
// Hardware: Seeed XIAO nRF52840 (Distrelec 303-83-819) auf der Trägerplatine (PLATINE.md),
//           Impuls über Optokoppler an D10.
// Software: Arduino IDE, Boardverwalter-URL von Seeed eintragen und «Seeed nRF52 Boards» installieren,
//           Board «Seeed XIAO nRF52840» wählen (nicht die «mbed»-Variante – nur diese enthält «bluefruit.h»).
//
// Gesendeter Wert (Text): "<Liter pro Minute>;<Liter gesamt seit Einschalten>", z. B. "2950;18420"

#include <bluefruit.h>

// ---------- Einstellungen ----------
const int      IMPULS_PIN       = D10;   // Optokoppler-Ausgang
const float    LITER_PRO_IMPULS = 1.0;   // am Messumformer eingestellt: 1 Impuls = 1 Liter
const uint32_t ENTPRELL_US      = 2000;  // Impulse kürzer als 2 ms Abstand ignorieren
const uint32_t FENSTER_MS       = 3000;  // Durchfluss = Mittelwert über die letzten 3 Sekunden
const uint32_t SLOT_MS          = 250;

#define GERAETENAME "Guelle-Durchfluss"

// UUIDs wie in der ESP32-Version (6b1f0001-/6b1f0002-4a3b-4d52-9a2e-3c1d5e7f8a90), Bytes rückwärts
const uint8_t SERVICE_UUID[16] = { 0x90, 0x8a, 0x7f, 0x5e, 0x1d, 0x3c, 0x2e, 0x9a, 0x52, 0x4d, 0x3b, 0x4a, 0x01, 0x00, 0x1f, 0x6b };
const uint8_t WERT_UUID[16]    = { 0x90, 0x8a, 0x7f, 0x5e, 0x1d, 0x3c, 0x2e, 0x9a, 0x52, 0x4d, 0x3b, 0x4a, 0x02, 0x00, 0x1f, 0x6b };

BLEService        dienst(SERVICE_UUID);
BLECharacteristic wert(WERT_UUID);

// ---------- Impulse zählen ----------
volatile uint32_t impulse = 0;
volatile uint32_t letzterImpulsUs = 0;

void beiImpuls() {
  uint32_t jetzt = micros();
  if (jetzt - letzterImpulsUs >= ENTPRELL_US) {
    impulse++;
    letzterImpulsUs = jetzt;
  }
}

uint32_t leseImpulse() {
  return impulse;  // 32-Bit-Lesen ist auf dem nRF52840 atomar
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

  Bluefruit.begin();
  Bluefruit.setName(GERAETENAME);
  Bluefruit.setTxPower(4);  // maximale Sendeleistung für Reichweite bis in die Kabine

  dienst.begin();
  wert.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  wert.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  wert.setMaxLen(32);
  wert.begin();
  wert.write("0;0");

  startWerbung();

  zaehler[0] = 0;
  zeit[0] = millis();
  pos = 1;
  Serial.println("Durchfluss-Sender bereit");
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

    char text[32];
    snprintf(text, sizeof(text), "%lu;%lu", (unsigned long)(lmin + 0.5f), (unsigned long)(n * LITER_PRO_IMPULS));
    wert.write(text);
    if (Bluefruit.connected()) wert.notify(text);
    Serial.println(text);
  }
}
