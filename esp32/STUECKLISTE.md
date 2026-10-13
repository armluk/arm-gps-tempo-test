# Stückliste Durchfluss-Sender (Variante 2: Modul ausserhalb des Sensorgehäuses)

Arm Eggiwil AG · Stand 13.10.2026 · gehört zu [PLATINE.md](PLATINE.md) (Entwurf V5) und [platine-schema.svg](platine-schema.svg)

Die vergossene Platine sitzt **aussen** am bzw. neben dem KROHNE-Gehäuse. Nur das 3-adrige Kabel (VIN, GND, SIG)
geht durch eine **bestehende Kabelverschraubung** ins Gehäuse. Die Antenne des XIAO bleibt im Modul und funkt frei.

## A. Elektronik (für 1 Stück)

| Pos | Ref | Menge | Bauteil | Spezifikation | Hinweis |
|---|---|---|---|---|---|
| 1 | U3 | 1 | **Seeed XIAO ESP32-C3** | Bluetooth LE, inkl. Flex-Antenne (U.FL) | flach auflöten |
| 2 | U2 | 1 | **TRACO TSR 1-2450** | DC/DC 6.5–36 V → 5 V / 1 A, SIP-3 | |
| 3 | U1 | 1 | Optokoppler **PC817** | DIP-4 | oder LTV-817 |
| 4 | D1, D4 | 2 | Schottky-Diode **1N5819** | 40 V / 1 A, DO-41 | Verpolschutz, Rückspeiseschutz |
| 5 | D2 | 1 | TVS-Diode **P6KE36A** | 600 W, unidirektional | Überspannungsschutz |
| 6 | D3 | 1 | Diode **1N4148** | DO-35 | Schutz Optokoppler-LED |
| 7 | F1 | 1 | PTC-Sicherung, radial | Haltestrom 0.5 A, ≥ 60 V | rückstellend |
| 8 | R1 | 1 | Widerstand | 3.3 kΩ, 0.5 W | |
| 9 | R2 | 1 | Widerstand | **1 kΩ, 2 W** | Pulldown SIG → GND, mit Abstand montieren |
| 10 | C1 | 1 | Elko radial | 10 µF / 50 V | |
| 11 | C2 | 1 | Elko radial | 22 µF / 10 V (oder 25 V) | |
| 12 | – | 1 | Leiterplatte | ca. 45 × 25 mm, 2 Lagen | oder Lochraster-Platine |

## B. Anschluss und Verpackung

| Pos | Menge | Teil | Spezifikation | Hinweis |
|---|---|---|---|---|
| 13 | nach Bedarf (ca. 1–2 m) | Kabel 3-adrig | 3 × 0.5 mm² oder 3 × 0.75 mm², **öl- und UV-beständig** (z. B. PUR-Mantel) | Adern: VIN, GND, SIG |
| 14 | 3 | Aderendhülsen | passend zum Querschnitt | für die Klemmen im KROHNE-Gehäuse |
| 15 | 1 | Vergussmasse | **weichbleibend**, 2K-Silikon oder Polyurethan | kein hartes Epoxid |
| 16 | 1 | Schrumpfschlauch mit Innenkleber | 3:1, Ø ca. 24 mm geschrumpft auf ≤ 8 mm, ca. 80 mm lang | über Platine und Kabelaustritt |
| 17 | 1 | Kleine Schrumpfschläuche | Ø 3–6 mm | Einzeladern isolieren |
| 18 | 1 | Halterung | Kabelbinder bzw. Kunststoff-Abstandshalter | Modul **mind. 2 cm von Metall** entfernt befestigen |

## C. Fahrzeugseite / Zuleitung

| Pos | Menge | Teil | Spezifikation | Hinweis |
|---|---|---|---|---|
| 19 | 1 | Fliegender Sicherungshalter | KFZ-Flachsicherung, mit **1 A** Sicherung | direkt an der Abzweigung von +12/24 V |

Die Versorgung kann dort abgegriffen werden, wo auch der Messumformer versorgt wird (Klemme L bzw. +).

## D. Zum Programmieren und Testen

| Pos | Teil | Hinweis |
|---|---|---|
| 20 | USB-C-Kabel (Daten) | zum Aufspielen der Software |
| 21 | Arduino IDE + «esp32 by Espressif Systems», Board «XIAO_ESP32C3» | Software: `durchfluss_ble/durchfluss_ble.ino` |
| 22 | Multimeter | Impulse D ↔ GND prüfen (0 / 8–30 V) |

## Ablauf

1. Platine bestücken, XIAO auflöten, Antenne anstecken (Antenne vom Kupfer weg legen).
2. Software aufspielen, im seriellen Monitor erscheint jede Sekunde z. B. `0;0`.
3. Am Fass anschliessen: **VIN → +**, **GND → N / Minus**, **SIG → Klemme D**. Den 1-kΩ-Widerstand am Sensor entfernen.
4. Karten-App öffnen → «Durchflussmesser verbinden» → **Guelle-Durchfluss** wählen → bei laufender Pumpe Durchfluss prüfen
   und mit dem bestehenden Gerät vergleichen. Reichweite bis in die Kabine testen.
5. Erst dann vergiessen und einschrumpfen.
