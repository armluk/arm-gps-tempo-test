# Trägerplatine für M5Stack ATOM Lite – Durchfluss-Sender Güllefass

Arm Eggiwil AG · Entwurf 13.10.2026 · Stand: Konzept, noch nicht gefertigt/geprüft

## Idee

Eine kleine Platine (ca. 40 × 30 mm) nimmt die **12 V vom Traktor** und den **12-V-Impuls des KROHNE OPTIFLUX 1000**
(Impulsausgang Messumformer, 1 Impuls = 1 Liter) über Schraubklemmen auf. Der **ATOM Lite** wird mit einem
normalen **Grove-Kabel (HY2.0-4P)** angesteckt. Über dieses Kabel bekommt er 5 V und das Impulssignal auf **G26** –
genau den Pin, den die Software `durchfluss_ble.ino` bereits verwendet. Kein Löten am ATOM, keine Pin-Reihenfolge der
Stiftleiste nötig.

Grove-Buchse des ATOM Lite (laut M5Stack-Doku): **GND · 5V · G26 · G32**.

## Schaltplan

```
 J1 (Schraubklemme 2-pol.)                                         J3 (Grove HY2.0-4P zum ATOM Lite)
 +12V ──F1──►|D1──┬──────┬──────────┐                               1 GND ───────────┐
                  │      │       ┌──┴──────────┐                    2 5V  ◄── +5V    │
                 D2     C1       │ U2 R-78E5.0 │── +5V ──┬──── C2   3 G26 ◄── PULS_OUT
                 TVS   22µ/50V   │  IN  GND OUT│         │          4 G32 (frei)     │
                  │      │       └──────┬──────┘         │                           │
 GND ─────────────┴──────┴──────────────┴────────────────┴───────────────────────────┴── GND (ATOM)

 J2 (Schraubklemme 2-pol.)              U1 PC817 (Optokoppler)
 PULS+ ──R1 2.7k──┬──────────► Anode  1 │      │ 4  Kollektor ── PULS_OUT ──► J3 Pin 3 (G26)
                  D3 1N4148 (antiparallel)    │ 3  Emitter   ── GND (ATOM-Seite)
 PULS− ───────────┴──────────► Kathode 2 │
```

- **Impulsseite (J2) und ATOM-Seite sind galvanisch getrennt** (U1). Das bestehende Gerät am Impulsausgang
  läuft unverändert weiter; die Platine belastet die Impulsleitung mit ca. 4–5 mA.
- Der ESP32 nutzt den **internen Pull-up** an G26: Impuls kommt → U1 leitet → G26 = LOW (Software zählt fallende Flanken).

## Stückliste

| Ref | Bauteil | Wert / Typ | Zweck |
|---|---|---|---|
| J1 | Schraubklemme 2-pol., RM 5.08 | | 12 V Traktor ein |
| J2 | Schraubklemme 2-pol., RM 5.08 | | Impuls vom Messumformer |
| J3 | Grove-Buchse HY2.0-4P, liegend | | Kabel zum ATOM Lite |
| F1 | PTC-Sicherung (rückstellend) | 0.5 A Haltestrom, ≥ 30 V | Kurzschlussschutz |
| D1 | Schottky-Diode | SS34 (40 V, 3 A) | Verpolschutz |
| D2 | TVS-Diode | SMBJ18A | fängt Spannungsspitzen im Bordnetz ab |
| C1 | Elko | 22 µF / 50 V | Eingangspuffer |
| U2 | Schaltregler-Modul | RECOM R-78E5.0-0.5 (7–28 V ein, 5 V / 0.5 A aus) | 12 V → 5 V für den ATOM |
| C2 | Kondensator | 22 µF / 10 V | Ausgangspuffer |
| U1 | Optokoppler | PC817 (oder LTV-817), DIP-4 | Trennung Impuls ↔ ESP32 |
| R1 | Widerstand 0.25 W | 2.7 kΩ | LED-Strom ≈ 4 mA bei 12 V, ≈ 5 mA bei 14.5 V |
| D3 | Diode | 1N4148 | schützt die Optokoppler-LED bei Verpolung |
| – | M5Stack ATOM Lite + Grove-Kabel | | Bluetooth-Sender |
| – | Gehäuse IP67, ca. 80 × 60 × 40 mm, 2 Kabelverschraubungen M12 | | Schutz am Fass |

Alle Bauteile als **Durchsteck-Typ (THT)** wählbar, damit die Platine von Hand bestückt werden kann.
SS34/SMBJ18A gibt es nur als SMD – alternativ 1N5822 (THT) für D1 und P6KE18A (THT) für D2.

## Hinweise zur Auslegung

- **Spannungsspitzen:** Das Traktor-Bordnetz kann Spitzen über 28 V haben (z. B. beim Abklemmen der Batterie bei
  laufendem Motor). D2 begrenzt diese, der R-78E ist bis 28 V spezifiziert. Für mehr Reserve einen Regler mit
  höherer Eingangsspannung (36 V oder mehr) wählen.
- **Impulsausgang aktiv/passiv:** Bei passivem Ausgang (Open Collector) liefert das bestehende Gerät die 12 V –
  mit dem Messgerät prüfen, ob zwischen PULS+ und PULS− bei Durchfluss 0/12-V-Impulse anliegen.
- **Belastung der Impulsleitung:** ca. 5 mA zusätzlich. Im Handbuch des Messumformers prüfen, wie viel Strom der
  Impulsausgang liefern darf (bei passiven Ausgängen meist deutlich mehr).
- **Frequenz:** 1 Impuls/l bei 3'000 l/min = 50 Hz – für PC817 und ESP32 unkritisch.

## Platine fertigen lassen

Die Platine ist einfach (2 Lagen, ca. 40 × 30 mm). Nächster Schritt: Schaltplan und Layout in **KiCad** (gratis)
oder **EasyEDA** zeichnen und bei einem Platinenhersteller (z. B. JLCPCB, Aisler, PCBWay) bestellen – 5 Stück
kosten dort typischerweise nur wenige Franken plus Versand. Bestückung von Hand oder durch den Hersteller.
