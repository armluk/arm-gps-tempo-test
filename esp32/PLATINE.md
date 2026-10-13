# Trägerplatine mit Seeed XIAO ESP32-C3 – Durchfluss-Sender Güllefass

Arm Eggiwil AG · Entwurf V5, 13.10.2026 · Stand: Konzept, noch nicht gefertigt/geprüft

Schema zur Kontrolle: [platine-schema.svg](platine-schema.svg)

## Anforderungen

- **Anschluss mit angelöteten Kabellitzen**, Platine wird **vergossen/versiegelt** und mit **Schrumpfschlauch** eingepackt.
- Drei Litzen nach aussen:
  1. **VIN** – Versorgung **8 … 30 V DC**
  2. **GND** – Masse (gemeinsam für Versorgung und Signal)
  3. **SIG** – Impuls von Klemme **D** des KROHNE OPTIFLUX 1000 (1 Impuls = 1 Liter).
     **D–** liegt auf +. Der **1-kΩ-Pulldown von D nach GND sitzt auf der Platine (R2)** – parallel, nicht in Serie.
     Am Sensor muss kein Widerstand mehr angeschlossen werden.
- **Seeed XIAO ESP32-C3** (Bluetooth LE) wird **direkt auf die Platine gelötet** (Randkontakte). Kein Kabel, kein Stecker.
  Impuls auf **D10 (GPIO10)** – kein Strapping-Pin, keine Sonderfunktion beim Start.

## Schaltplan

```
 Litze VIN (8–30 V) ──F1──►|D1──┬──────┬───────────────┐
                                │      │        ┌──────┴───────┐
                               D2     C1        │ U2 TSR 1-2450│
                              TVS   10µ/50V     │ VIN  GND VOUT│── +5V ──┬── C2 22µ/10V
                                │      │        └──────┬───────┘         │
 Litze GND ─────────────────────┴──────┴───────────────┴──────── GND     D4 1N5819 (Anode +5V, Kathode XIAO 5V)
                                                                         │
                                                                   XIAO Pin «5V»

 Litze SIG ──┬─────────── R1 3.3k ──┬──► U1 PC817 Pin 1 (Anode)
             │                      D3 1N4148 (Kathode an Pin 1, Anode an Pin 2)
             R2 1k / 2 W            │
             (Pulldown)  GND ───────┴──► U1 Pin 2 (Kathode)
             │
            GND                          U1 Pin 4 (Kollektor) ──► XIAO Pin «D10»
                                         U1 Pin 3 (Emitter)   ──► GND = XIAO Pin «GND»
```

Funktion: Impuls «high» (8–30 V) → LED im Optokoppler leuchtet → Transistor leitet → D10 wird LOW.
Interner Pull-up an D10, die Software zählt fallende Flanken.

**D4** ist nötig, weil der 5V-Pin des XIAO direkt mit der USB-Buchse verbunden ist. Laut Seeed-Wiki darf man dort
nur über eine Diode einspeisen – sonst würde beim Programmieren per USB Strom in den TSR zurückfliessen.
Am 5V-Pin liegen danach ca. 4.6 V; der XIAO macht daraus selbst 3.3 V.

## Netzliste (Verbindungen)

| Netz | Verbunden mit |
|---|---|
| **VIN_RAW** | Lötpad P1 (Litze VIN) · F1 Pin 1 |
| **VIN_F** | F1 Pin 2 · D1 Anode |
| **VIN** | D1 Kathode · D2 Kathode · C1 + · U2 Pin 1 (VIN) |
| **GND** | Lötpad P2 (Litze GND) · D2 Anode · C1 − · U2 Pin 2 (GND) · C2 − · R2 Pin 2 · U1 Pin 2 · D3 Anode · U1 Pin 3 · XIAO «GND» |
| **+5V** | U2 Pin 3 (VOUT) · C2 + · D4 Anode |
| **5V_XIAO** | D4 Kathode · XIAO «5V» |
| **SIG** | Lötpad P3 (Litze SIG) · R2 Pin 1 · R1 Pin 1 |
| **LED_A** | R1 Pin 2 · U1 Pin 1 · D3 Kathode |
| **D10** | U1 Pin 4 · XIAO «D10» |

Übrige XIAO-Pins (D0–D9, 3V3) bleiben frei – Pads trotzdem mitlöten (mechanischer Halt).

## Stückliste

| Ref | Bauteil | Wert / Typ | Bemerkung |
|---|---|---|---|
| P1–P3 | Lötpads mit Zugentlastungsbohrung | für Litze 0.5–0.75 mm² | VIN, GND, SIG |
| F1 | PTC-Sicherung, radial | 0.5 A Haltestrom, ≥ 60 V | rückstellend |
| D1 | Schottky-Diode | 1N5819 (40 V, 1 A) | Verpolschutz |
| D2 | TVS-Diode | P6KE36A | begrenzt Spannungsspitzen; leitet erst über 30 V |
| C1 | Elko | 10 µF / 50 V, radial | |
| U2 | Schaltregler-Modul | **TRACO TSR 1-2450** (6.5–36 V ein, 5 V / 1 A aus) | |
| C2 | Elko | 22 µF / 10 V, radial | |
| D4 | Schottky-Diode | 1N5819 | Rückspeiseschutz zum USB-Port des XIAO |
| R1 | Widerstand | 3.3 kΩ, 0.5 W | LED-Strom ≈ 2 mA bei 8 V, ≈ 9 mA bei 30 V |
| R2 | Widerstand | **1 kΩ, 2 W** | Pulldown SIG → GND (ersetzt den Widerstand am Sensor) |
| D3 | Diode | 1N4148 | schützt die Optokoppler-LED |
| U1 | Optokoppler | PC817, DIP-4 | |
| U3 | **Seeed XIAO ESP32-C3** | mit Antenne | flach aufgelötet (Randkontakte), USB-C zur Platinenkante |

## Wichtige Hinweise

- **Antenne:** Der XIAO ESP32-C3 wird mit einer kleinen Klebe-/Flex-Antenne geliefert (U.FL-Stecker). Antenne
  anstecken und **mit vergiessen**, nicht über Metall oder die Massefläche legen. Bluetooth funkt durch Vergussmasse
  und Schrumpfschlauch, nicht durch Metall.
- **R2 wird warm:** Solange der Impuls ansteht, bei 12 V ≈ 0.14 W, bei 24 V ≈ 0.6 W, bei 30 V ≈ 0.9 W → **2 W**,
  mit 2–3 mm Abstand zur Platine montieren.
- **Bestehendes Gerät:** Der Widerstand am Sensor entfällt, R2 übernimmt. Ist die Platine abgeklemmt, fehlt der
  Pulldown auch dem bestehenden Gerät. **Nicht beide gleichzeitig** anschliessen (sonst 500 Ω).
- **Spannungsspitzen:** TSR 1-2450 bis 36 V angegeben; D2 begrenzt grössere Spitzen, aber nicht garantiert unter 36 V.
- **Pin-Belegung XIAO:** Lage von 5V, GND und D10 am Modul vor dem Layout im Seeed-Pinout prüfen.
- **Vor dem Vergiessen:**
  1. Software per USB-C aufspielen (Board «XIAO_ESP32C3») und mit Signal testen.
  2. Funktion am Fass prüfen (Durchfluss in der Karten-App sichtbar).
  3. Die USB-C-Buchse an die Platinenkante legen. Wer später neue Software aufspielen will, lässt die Buchse frei
     (mit Gummikappe), sonst wird sie mit vergossen.
- **Versiegeln:** weichbleibende Vergussmasse (Silikon oder Polyurethan), kein hartes Epoxid; danach
  **Schrumpfschlauch mit Innenkleber** über Platine und Litzenaustritte.

## Platine

Grösse ca. **45 × 25 mm**, 2 Lagen, alle Bauteile oben. Litzen-Pads an einer Schmalseite (VIN · GND · SIG),
XIAO an der gegenüberliegenden Seite mit USB-C zur Kante. Massefläche unten – **nicht unter der Antenne**.
Zeichnen in KiCad oder EasyEDA, fertigen z. B. bei JLCPCB, Aisler oder PCBWay.
