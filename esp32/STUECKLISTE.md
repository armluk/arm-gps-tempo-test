# Stückliste Durchfluss-Sender – Bestellung bei Distrelec Schweiz

Arm Eggiwil AG · Stand 13.10.2026 · Variante 2 (Modul ausserhalb des Sensorgehäuses)
gehört zu [PLATINE.md](PLATINE.md) und [platine-schema.svg](platine-schema.svg)

Preise und Lagerbestand am 13.10.2026 auf distrelec.ch abgelesen, **exkl. MwSt**, können sich ändern.
Viele Kleinteile gibt es nur in Packungen – die Packung reicht dann für mehrere Module.

## A. Elektronik

| Pos | Ref | Menge | Bauteil | Distrelec Art.-Nr. | Hersteller-Nr. | Preis | Hinweis |
|---|---|---|---|---|---|---|---|
| 1 | U3 | 1 | **Seeed XIAO nRF52840**, Bluetooth LE, **Antenne integriert** | **303-83-819** | 102010448 | CHF 9.15 / Stk. | ersetzt den XIAO ESP32-C3 (bei Distrelec nicht mehr im Sortiment) |
| 2 | U2 | 1 | DC/DC-Wandler TRACO **TSR 1-2450**, 6.5–36 V → 5 V / 1 A | **169-53-484** | TSR 1-2450 | CHF 5.80 / Stk. | an Lager |
| 3 | U1 | 1 | Optokoppler Vishay **CNY17-3**, DIP-6 | **175-20-642** | CNY17-3 | CHF 0.26 / Stk. | ersetzt PC817 (nicht erhältlich), **andere Pinbelegung**, siehe unten |
| 4 | D1, D4 | 2 | Schottky-Diode **1N5819**, 40 V / 1 A, DO-41 | **304-42-973** | 1N5819 (Taiwan Semiconductor) | CHF 13.33 / 100 Stk. | |
| 5 | D2 | 1 | TVS-Diode **SMBJ33A**, 33 V, 600 W, **SMD (DO-214AA/SMB)** | **304-45-455** | SMBJ33A (Littelfuse) | CHF 4.83 / 25 Stk. | ersetzt P6KE36A (nicht erhältlich); grosses SMD-Gehäuse, von Hand lötbar |
| 6 | D3 | 1 | Schottky-Diode **BAT41**, 100 V, DO-35 | **170-12-019** | BAT41 (ST) | CHF 0.025 / Stk. | ersetzt 1N4148 (nicht mehr im Sortiment) |
| 7 | F1 | 1 | Rückstellende Sicherung Bourns **MF-R050**, radial, 60 V, Haltestrom 0.5 A | **304-42-380** | MF-R050 | CHF 3.11 / 10 Stk. | Distrelec nennt «1 A» = Auslösestrom |
| 8 | R1 | 1 | Widerstand **3.3 kΩ**, 0.6 W, 1 %, axial 0207 | **304-43-123** | MBB02070C3301FCT00 (Vishay) | CHF 3.28 / 50 Stk. | |
| 9 | R2 | 1 | Widerstand **1 kΩ, 2 W**, Kohleschicht, axial | **304-98-687** | 7078861 (RS PRO) | CHF 2.19 | Pulldown; Packungsgrösse auf der Produktseite prüfen |
| 10 | C1 | 1 | Elko radial **10 µF / 50 V**, 105 °C | **167-25-806** | ECA1HHG100I (Panasonic) | – | Preis auf der Produktseite prüfen |
| 11 | C2 | 1 | Elko radial **22 µF / 25 V** | **304-42-838** | EEAFC1E220 (Panasonic) | CHF 1.77 | Packungsgrösse prüfen |
| 12 | – | 1 | Leiterplatte ca. 45 × 25 mm oder Lochraster | – | – | – | Platine z. B. bei JLCPCB/Aisler, oder Lochraster aus dem Sortiment |

## B. Verpackung

| Pos | Menge | Teil | Distrelec Art.-Nr. | Hersteller-Nr. | Preis | Hinweis |
|---|---|---|---|---|---|---|
| 13 | 1 | Silikon-Vergussmasse 2K, transparent, 250 g, 1:1 | **304-87-040** | ACC Silicones QSIL 216 | – | weichbleibend, −60 … +204 °C |
| 14 | 1 | Schrumpfschlauch 3:1, 24 → 8 mm, schwarz, 1.2 m | **305-08-226** | TE RNF 3000-24/8-0 | CHF 14.91 | **vor Bestellung prüfen, ob mit Innenkleber**; sonst Platine vor dem Schrumpfen ganz vergiessen |
| 15 | 1 | Kleine Schrumpfschläuche Ø 3–6 mm | – | – | – | Einzeladern |
| 16 | 1 | Kabelbinder / Kunststoff-Abstandshalter | – | – | – | Modul **mind. 2 cm von Metall** |

## C. Nicht bei Distrelec gesucht – im Landmaschinen- oder Autozubehör-Handel

| Pos | Teil | Spezifikation |
|---|---|---|
| 17 | Kabel 3-adrig | 3 × 0.5 oder 3 × 0.75 mm², öl- und UV-beständig (PUR-Mantel), ca. 1–2 m |
| 18 | Aderendhülsen | passend zum Querschnitt |
| 19 | Fliegender Sicherungshalter + Sicherung 1 A | KFZ-Flachsicherung, direkt an der Abzweigung von + |

## Geänderte Bauteile – was das für die Platine bedeutet

1. **XIAO nRF52840 statt XIAO ESP32-C3**
   - Gleiche Grösse und Pinbelegung (5V, GND, D10), Platine bleibt gleich.
   - **Antenne ist auf dem Modul integriert** – kein Antennenkabel, einfacher zu vergiessen.
     Den Antennenbereich des Moduls nicht über die Massefläche der Platine legen (Platinenrand).
   - Eigene Software: `durchfluss_ble_nrf52/durchfluss_ble_nrf52.ino` (Board «Seeed XIAO nRF52840», Paket
     «Seeed nRF52 Boards»). Funkprotokoll identisch – die Karten-App funktioniert unverändert.
   - D4 (Schottky vor dem 5V-Pin) bleibt: auch hier ist 5V direkt mit USB verbunden.
2. **CNY17-3 (DIP-6) statt PC817 (DIP-4)** – Pinbelegung:
   - Pin 1 = Anode (an R1), Pin 2 = Kathode (an GND), Pin 3 = frei
   - Pin 4 = **Emitter** (an GND), Pin 5 = **Kollektor** (an D10), Pin 6 = Basis – **frei lassen**
3. **SMBJ33A statt P6KE36A** – gleiche Funktion, SMD-Gehäuse (ca. 4 × 5 mm), auf der Platine als SMD-Pads vorsehen.
4. **BAT41 statt 1N4148** – gleiche Funktion (Schutz der Optokoppler-LED).

## Ungefähre Kosten Elektronik

Für ein Modul ca. **CHF 45–55** inkl. der angebrochenen Packungen (ohne Platine, Vergussmasse und Kabel),
für jedes weitere Modul deutlich weniger (Packungen reichen für viele Module).
