// Eine Bluetooth-Verbindung zu einem Handy (bis zu 3 gleichzeitig), nur im Arbeitsspeicher
#pragma once
#include <stdint.h>

const uint8_t MAX_HANDYS = 3;
struct Sitzung {
  uint16_t h;                            // Verbindung, BLE_CONN_HANDLE_INVALID = frei
  bool angemeldet;
  uint32_t seit;                         // Zeitpunkt der Verbindung (für die 5-s-Frist)
  uint8_t ns[8];                         // Zufallszahl des Moduls für diese Verbindung
  uint8_t kSe[16], kSm[16];              // Sitzungsschlüssel
  uint32_t nr;                           // Meldungszähler
};
