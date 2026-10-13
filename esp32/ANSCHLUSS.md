# Durchfluss-Sender Güllefass – Anschluss für die Werkstatt

Arm Eggiwil AG · Güllefass-Rechner · Stand 13.10.2026

## Zweck

Ein kleiner ESP32 zählt die Impulse des Durchflussmessers **KROHNE OPTIFLUX 1000**
(Impulsausgang des Messumformers, **12 V DC, 1 Impuls = 1 Liter**) und sendet den Durchfluss
per Bluetooth an das Handy (Web-Seite «Karte»).

Der Impulsausgang ist bereits an ein anderes Gerät angeschlossen. Der ESP32 wird **parallel**
und **galvanisch getrennt** (Optokoppler) angeschlossen und belastet die Leitung nur mit wenigen mA.
Das bestehende Gerät läuft unverändert weiter.

## Teileliste

| Teil | Zweck | Hinweis |
|---|---|---|
| M5Stack ATOM Lite **oder** ESP32 DevKitC | Mikrocontroller mit Bluetooth | ATOM Lite: Impuls an Grove-Buchse (G26) |
| Optokoppler-Modul mit PC817, Eingang für 12 V | trennt 12-V-Impuls vom ESP32 | Modul mit Vorwiderstand für 12 V wählen, sonst 2.2 kΩ in Serie |
| Spannungswandler 12 V → 5 V (USB), min. 1 A | Stromversorgung ESP32 | z. B. USB-Einbaubuchse für 12 V oder DC/DC-Wandler |
| Feinsicherung 1 A mit Halter | Schutz 12-V-Zuleitung | |
| Gehäuse IP67 mit 2 Kabelverschraubungen | Schutz vor Wasser und Gülle | |
| Kabel 2 × 0.5 mm² (Impuls), 2 × 0.75 mm² (12 V) | | |

## Anschlussplan

```
 Messumformer IFC (Impulsausgang, 12 V DC)                 bestehendes Gerät
   Impuls + ────────────────┬──────────────────────────────── (bleibt wie es ist)
   Impuls − / 0 V ──────────┼──┬───────────────────────────── (bleibt wie es ist)
                            │  │
                   ┌────────┴──┴────────┐
                   │ Optokoppler PC817  │
                   │  IN+         IN−   │
                   │                    │
                   │  OUT (Kollektor)   ├──────── ESP32 GPIO 26
                   │  GND (Emitter)     ├──────── ESP32 GND
                   └────────────────────┘

 Traktor / Fass 12 V + ── Sicherung 1 A ── Wandler 12→5 V ── ESP32 5 V (USB)
 Traktor / Fass 0 V  ───────────────────── Wandler GND
```

- **IN+** an den Impuls-Plus-Ausgang, **IN−** an Impuls-Minus bzw. 0 V des Umformers.
- Der ESP32 nutzt den internen Pull-up an GPIO 26: Impuls kommt → Optokoppler schaltet → GPIO wird LOW.
- Impuls-Seite und ESP32-Seite **nicht** verbinden (Trennung durch den Optokoppler).

## Vor dem Anschliessen prüfen

1. Ist der Impulsausgang **aktiv** (liefert selbst 12 V) oder **passiv** (Open Collector)?
   Bei passivem Ausgang braucht es zusätzlich einen Pull-up-Widerstand (z. B. 2.2 kΩ) von Impuls+ an 12 V –
   meist ist dieser schon im bestehenden Gerät vorhanden. Mit dem Messgerät prüfen: zwischen Impuls+ und
   Impuls− sollten bei Durchfluss Impulse zwischen ca. 0 V und 12 V messbar sein.
2. Bei 1 Impuls pro Liter und z. B. 3'000 l/min kommen **50 Impulse pro Sekunde** – unkritisch.

## Software aufspielen

1. Arduino IDE installieren, im Boardverwalter «esp32 by Espressif Systems» installieren.
2. Datei `durchfluss_ble/durchfluss_ble.ino` öffnen, Board «M5Stack-ATOM» bzw. «ESP32 Dev Module» wählen, hochladen.
3. Im seriellen Monitor (115200 Baud) erscheint jede Sekunde z. B. `2950;18420` (l/min; Liter gesamt).

## Auf dem Handy

1. Web-Seite «Karte» öffnen (https://armluk.github.io/arm-gps-tempo-test/karte.html), in Chrome.
2. «Durchflussmesser verbinden» antippen, Gerät **Guelle-Durchfluss** wählen.
3. Bluetooth und Standort müssen am Handy eingeschaltet sein.
