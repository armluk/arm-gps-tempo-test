# Trägerplatine für M5Stack ATOM Lite – Durchfluss-Sender Güllefass

Arm Eggiwil AG · Entwurf V2, 13.10.2026 · Stand: Konzept, noch nicht gefertigt/geprüft

## Anforderungen

- **Anschluss mit angelöteten Kabellitzen**, Platine wird **vergossen/versiegelt** und mit **Schrumpfschlauch** eingepackt
  (keine Schraubklemmen, keine Stecker nach aussen).
- Drei Anschlüsse nach aussen:
  1. **VIN** – Versorgung **8 … 30 V DC**
  2. **GND** – Masse (gemeinsam für Versorgung und Signal)
  3. **SIG** – Impulssignal vom Klemme **D** des KROHNE OPTIFLUX 1000 (1 Impuls = 1 Liter).
     **D–** liegt auf +, der **1-kΩ-Pulldown von D nach GND sitzt am Sensor** (nicht in Serie, nicht auf der Platine).
     Die Platine greift das Signal nur hochohmig ab.

Schema zur Kontrolle: [platine-schema.svg](platine-schema.svg)
- Der ATOM Lite wird über ein **gekürztes Grove-Kabel (HY2.0-4P)** angelötet und bekommt darüber 5 V und das
  Signal auf **G26** – dem Pin, den `durchfluss_ble.ino` bereits verwendet.

## Schaltplan

```
 Litze VIN (8–30 V) ──F1──►|D1──┬──────┬───────────────┐
                                │      │        ┌──────┴───────┐
                               D2     C1        │ U2 TSR 1-2450│
                              TVS   10µ/50V     │ VIN  GND VOUT│── +5V ──┬── C2 22µ/10V ──┐
                                │      │        └──────┬───────┘         │                │
 Litze GND ─────────────────────┴──────┴───────────────┴──────── GND ────┼────────────────┴──┐
                                                                         │                   │
                                                                   Grove rot (5V)      Grove schwarz (GND)
 Litze SIG ────────────── R1 3.3k ──┬──► U1 PC817 Pin 1 (Anode)
                                    D3 1N4148 (Kathode an Pin 1, Anode an Pin 2)
                         GND ───────┴──► U1 Pin 2 (Kathode)

 (1-kΩ-Pulldown D → GND sitzt am Sensor, siehe platine-schema.svg)
                                         U1 Pin 4 (Kollektor) ──► Grove gelb (G26)
                                         U1 Pin 3 (Emitter)   ──► GND
```

Funktion: Impuls «high» (8–30 V) → LED im Optokoppler leuchtet → Transistor leitet → G26 wird LOW.
Der ESP32 nutzt den internen Pull-up an G26 und zählt fallende Flanken (wie in der Software umgesetzt).
Der Optokoppler schützt den ESP32 (3.3 V) vor dem 8–30-V-Signal.

## Netzliste (Verbindungen)

| Netz | Verbunden mit |
|---|---|
| **VIN_RAW** | Lötpad P1 (Litze VIN) · F1 Pin 1 |
| **VIN_F** | F1 Pin 2 · D1 Anode |
| **VIN** | D1 Kathode · D2 Kathode · C1 + · U2 Pin 1 (VIN) |
| **GND** | Lötpad P2 (Litze GND) · D2 Anode · C1 − · U2 Pin 2 (GND) · C2 − · U1 Pin 2 · D3 Anode · U1 Pin 3 · Lötpad P5 (Grove schwarz) |
| **+5V** | U2 Pin 3 (VOUT) · C2 + · Lötpad P4 (Grove rot) |
| **SIG** | Lötpad P3 (Litze SIG) · R1 Pin 1 |
| **LED_A** | R1 Pin 2 · U1 Pin 1 · D3 Kathode |
| **G26** | U1 Pin 4 · Lötpad P6 (Grove gelb) |

Grove-Ader Weiss (G32) wird nicht verwendet – kurz abschneiden und isolieren.

## Stückliste (alles Durchsteck-Bauteile)

| Ref | Bauteil | Wert / Typ | Bemerkung |
|---|---|---|---|
| P1–P3 | Lötpads mit Zugentlastungsbohrung | für Litze 0.5–0.75 mm² | VIN, GND, SIG |
| P4–P6 | Lötpads | für Grove-Adern | 5V, GND, G26 |
| F1 | PTC-Sicherung, radial | 0.5 A Haltestrom, ≥ 60 V | rückstellend |
| D1 | Schottky-Diode | 1N5819 (40 V, 1 A) | Verpolschutz |
| D2 | TVS-Diode | P6KE36A | begrenzt Spannungsspitzen; liegt über 30 V, damit sie im Normalbetrieb nicht leitet |
| C1 | Elko | 10 µF / 50 V, radial | |
| U2 | Schaltregler-Modul | **TRACO TSR 1-2450** (6.5–36 V ein, 5 V / 1 A aus) | deckt 8–30 V mit Reserve ab |
| C2 | Elko | 22 µF / 10 V, radial | |
| R1 | Widerstand | 3.3 kΩ, 0.5 W | LED-Strom ≈ 2 mA bei 8 V, ≈ 9 mA bei 30 V |
| D3 | Diode | 1N4148 | schützt die Optokoppler-LED |
| U1 | Optokoppler | PC817, DIP-4 | |
| – | M5Stack ATOM Lite + Grove-Kabel | | Kabel auf ca. 5 cm kürzen, Adern verzinnen |

## Wichtige Hinweise

- **Belastung des Impulsausgangs:** Der bestehende 1-kΩ-Pulldown am Sensor bleibt wie er ist. Die Platine braucht
  zusätzlich nur ca. 2–9 mA (über R1 in den Optokoppler).
- **Spannungsspitzen:** Der TSR 1-2450 ist bis 36 V angegeben. D2 begrenzt grössere Spitzen, kann sie bei sehr
  energiereichen Störungen aber nicht ganz unter 36 V halten. Für ein Traktor-Bordnetz ist das in der Regel
  ausreichend; eine Garantie ist es nicht.
- **Grove-Farben:** Laut M5Stack-Doku ist die Reihenfolge GND · 5V · G26 · G32 = schwarz · rot · gelb · weiss.
  **Vor dem Anlöten mit dem Durchgangsprüfer bestätigen**, welche Ader an welchem Pin der Buchse liegt.
- **Vor dem Vergiessen:**
  1. Software auf den ATOM Lite laden (per USB-C) und mit Signal testen – nach dem Einpacken ist USB nicht mehr zugänglich.
  2. Funktion am Fass prüfen (Durchfluss in der Karten-App sichtbar).
- **Versiegeln:** Platine mit weichbleibender Vergussmasse (Silikon oder Polyurethan) vergiessen, nicht mit
  hartem Epoxid direkt auf Bauteilen (Spannungsrisse bei Kälte/Wärme). Danach **Schrumpfschlauch mit Innenkleber**
  (dual wall) über Platine und Litzenaustritte. Der ATOM Lite kann mit eingeschrumpft werden – Bluetooth funkt durch
  Schrumpfschlauch und Vergussmasse; nicht in Metall einpacken.

## Platine

Grösse ca. **35 × 22 mm**, 2 Lagen, alle Bauteile auf einer Seite, Lötpads an den Schmalseiten:
links **VIN · GND · SIG** (Litzen zum Fass), rechts **5V · GND · G26** (Grove zum ATOM).
Masse als Fläche auf der Unterseite. Zeichnen in KiCad oder EasyEDA, fertigen z. B. bei JLCPCB, Aisler oder PCBWay.

Alternative ohne Platinenbestellung: Aufbau auf einer kleinen **Lochrasterplatine** – da ohnehin vergossen wird,
ist das für ein Einzelstück gleichwertig.
