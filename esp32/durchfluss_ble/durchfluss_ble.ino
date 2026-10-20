// Durchfluss-Sender Güllefass – Arm Eggiwil AG
//
// VERALTET: sendet unverschlüsselt und wird von der App nicht mehr angenommen – aktuell ist
// durchfluss_ble_nrf52 (XIAO nRF52840, verschlüsselt).
//
// Zählt die Impulse des KROHNE OPTIFLUX 1000 (Impulsausgang 12 V DC, 1 Impuls = 1 Liter)
// und sendet jede Sekunde den Durchfluss per Bluetooth Low Energy an die Karten-Seite.
//
// Hardware: Seeed XIAO ESP32-C3 auf der Trägerplatine (siehe PLATINE.md), Impuls über Optokoppler an D10 (GPIO10).
//           D10 ist kein Strapping-Pin und hat beim Start keine Sonderfunktion.
// Software: Arduino IDE mit «esp32 by Espressif Systems» (Boardverwalter), Board «XIAO_ESP32C3».
//           Keine weiteren Bibliotheken nötig.
//
// Gesendeter Wert (Text): "<Liter pro Minute>;<Liter gesamt seit Einschalten>;<Schwankung ± m³/h>", z. B. "2950;18420;3"
// Die Schwankung kommt aus den Abständen der einzelnen Impulse in der letzten Sekunde (Erkennung «Ausblasen»).

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

// ---------- Einstellungen ----------
const int      IMPULS_PIN       = 10;    // Optokoppler-Ausgang an XIAO-Pin D10 (= GPIO10)
const float    LITER_PRO_IMPULS = 1.0;   // am Messumformer eingestellt: 1 Impuls = 1 Liter
const uint32_t ENTPRELL_US      = 2000;  // Impulse kürzer als 2 ms Abstand ignorieren (max. 500 l/s)
const uint32_t FENSTER_MS       = 3000;  // Durchfluss = Mittelwert über die letzten 3 Sekunden
const uint32_t SLOT_MS          = 250;

// Bluetooth-Name «SpreadLink-1234»: Nummer aus der einmaligen MAC-Adresse des Chips
#define NAMENSTEIL   "SpreadLink-"
#define SERVICE_UUID "6b1f0001-4a3b-4d52-9a2e-3c1d5e7f8a90"
#define WERT_UUID    "6b1f0002-4a3b-4d52-9a2e-3c1d5e7f8a90"

// ---------- Impulse zählen ----------
volatile uint32_t impulse = 0;
volatile uint32_t letzterImpulsUs = 0;
const int NZEIT = 256;                       // Zeitpunkte der letzten Impulse (reicht bis ca. 900 m³/h)
volatile uint32_t impulsZeit[NZEIT];

void IRAM_ATTR beiImpuls() {
  uint32_t jetzt = micros();
  if (jetzt - letzterImpulsUs >= ENTPRELL_US) {
    impulsZeit[impulse % NZEIT] = jetzt;
    impulse++;
    letzterImpulsUs = jetzt;
  }
}

uint32_t leseImpulse() {
  return impulse;  // 32-Bit-Lesen ist auf dem ESP32-C3 atomar
}

// Schwankung des Durchflusses in der letzten Sekunde, in ± m³/h (halbe Spanne zwischen 10 % und 90 %)
float schwankung() {
  static float q[NZEIT];
  uint32_t n = leseImpulse(), jetzt = micros();
  int k = 0;
  for (int32_t j = 0; j < NZEIT - 2 && (int32_t)n - 1 - j >= 1; j++) {
    uint32_t i = n - 1 - j;
    uint32_t t1 = impulsZeit[i % NZEIT], t0 = impulsZeit[(i - 1) % NZEIT];
    if (jetzt - t0 > 1000000UL) break;
    uint32_t d = t1 - t0;
    if (d > 0) q[k++] = 3.6e6f * LITER_PRO_IMPULS / d;
  }
  if (k < 4) return 0;
  for (int a = 1; a < k; a++) { float v = q[a]; int b = a - 1; while (b >= 0 && q[b] > v) { q[b + 1] = q[b]; b--; } q[b + 1] = v; }
  return (q[k * 9 / 10] - q[k / 10]) / 2;
}

// ---------- Bluetooth ----------
BLECharacteristic *wert = nullptr;
bool verbunden = false;

class Verbindung : public BLEServerCallbacks {
  void onConnect(BLEServer *s) override { verbunden = true; }
  void onDisconnect(BLEServer *s) override {
    verbunden = false;
    BLEDevice::startAdvertising();  // wieder sichtbar für das Handy
  }
};

// ---------- Ringpuffer für den Mittelwert ----------
const int SLOTS = FENSTER_MS / SLOT_MS + 1;
uint32_t zaehler[SLOTS];
uint32_t zeit[SLOTS];
int pos = 0;
bool voll = false;

void setup() {
  Serial.begin(115200);
  pinMode(IMPULS_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IMPULS_PIN), beiImpuls, FALLING);

  char geraetename[24];
  uint64_t mac = ESP.getEfuseMac();
  snprintf(geraetename, sizeof(geraetename), NAMENSTEIL "%04u", (unsigned)((uint32_t)(mac ^ (mac >> 32)) % 10000));
  BLEDevice::init(geraetename);
  Serial.print("Name: ");
  Serial.println(geraetename);
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new Verbindung());
  BLEService *dienst = server->createService(SERVICE_UUID);
  wert = dienst->createCharacteristic(WERT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  wert->addDescriptor(new BLE2902());
  wert->setValue("0;0");
  dienst->start();

  BLEAdvertising *werbung = BLEDevice::getAdvertising();
  werbung->addServiceUUID(SERVICE_UUID);
  werbung->setScanResponse(true);
  BLEDevice::startAdvertising();

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
    snprintf(text, sizeof(text), "%.0f;%lu;%.0f", lmin, (unsigned long)(n * LITER_PRO_IMPULS), schwankung());
    wert->setValue(text);
    if (verbunden) wert->notify();
    Serial.println(text);
  }
}
