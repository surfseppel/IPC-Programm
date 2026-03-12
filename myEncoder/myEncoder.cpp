// ===================================================================================
// ENCODER-INTERFACE IMPLEMENTIERUNG
// ===================================================================================
// Diese Datei enthält die Implementierung der EncoderInterface-Klasse
// für die Verwaltung von Quadratur-Encodern in Arduino-Umgebung
// ===================================================================================

#if defined(TEENSYDUINO)
	#include <QuadEncoder.h>
#else
	#include <Encoder.h>
#endif

#include "myGlobals.h"
#include "myEncoder.h"

// ===================================================================================
// KONSTRUKTOR
// ===================================================================================
// Initialisiert alle Variablen und erstellt das QuadEncoder-Objekt
//
// Parameter:
// - A_pin:       GPIO-Pin für Encoder-Signal A (meist Interrupt-fähiger Pin)
// - B_pin:       GPIO-Pin für Encoder-Signal B (meist Interrupt-fähiger Pin)
// - Enc_channel: Hardware-Timer-Kanal für den Encoder (0, 1, 2, ...)
// - out_scaling: Umrechnungsfaktor von Encoder-Impulsen zu gewünschter Einheit
//                Beispiel: Bei 4000 Impulsen/Umdrehung und gewünschter Ausgabe
//                in Radiant: out_scaling = 2*PI/4000 = 0.00157
myEncoder::myEncoder(uint8_t A_pin, uint8_t B_pin, uint8_t channel, double out_scaling)
  :
#if defined(TEENSYDUINO)
    encoder(channel, A_pin, B_pin, 1)  //Member-Variable der Encoder-Klasse der Teensy-QuadEncoder lib
#else
    encoder(A_pin, B_pin)  //Member-Variable der Encoder-Klasse der Arduino-Encoder lib
#endif
{
	// Skalierungsfaktor speichern
	output_scaling = out_scaling;

	// Alle Positions- und Zeitwerte für Geschwindigkeitsberechnung auf 0 setzen
	previous_position_0 = 0;     // Aktuelle Position
	previous_time_micros_0 = 0;  // Aktueller Zeitstempel
	previous_position_1 = 0;     // Vorherige Position
	previous_time_micros_1 = 0;  // Vorheriger Zeitstempel
	previous_position_2 = 0;     // Vorvorherige Position
	previous_time_micros_2 = 0;  // Vorvorheriger Zeitstempel

	// Update-Periode auf 5ms setzen (5000 Mikrosekunden)
	// Dies verhindert zu häufige Updates und damit Rauschen in der Geschwindigkeitsberechnung
	update_period = 5000;  //micros
}

// ===================================================================================
// ENCODER INITIALISIERUNG
// ===================================================================================
// Initialisiert den Hardware-Encoder mit Standardkonfiguration
void myEncoder::init() {
#if defined(TEENSYDUINO)
	encoder.setInitConfig();
	encoder.init();
#endif
}

// ===================================================================================
// POSITION SPEICHERN FÜR GESCHWINDIGKEITSBERECHNUNG
// ===================================================================================
// Diese Funktion sollte regelmäßig (z.B. in der Hauptschleife) aufgerufen werden
// Sie speichert die aktuellen Positionswerte mit Zeitstempel für spätere 
// Geschwindigkeitsberechnung
void myEncoder::update_stored_pos() {
	// -------------------------------------------------------------------------------
	// ZEIT-CHECK: Zu kurze Zeitabstände vermeiden
	// -------------------------------------------------------------------------------
	// Prüfen ob seit dem letzten Update genügend Zeit vergangen ist
	// Dies verhindert zu häufige Updates und reduziert Rauschen
	if ((micros() - previous_time_micros_0) < update_period) {
		return;  // Zu kurzer Zeitabstand - Update überspringen
	}

	// -------------------------------------------------------------------------------
	// WERTE VERSCHIEBEN: Zeitreihe um einen Schritt nach hinten verschieben
	// -------------------------------------------------------------------------------
	// Die Werte werden in einem "Schieberegister" gespeichert:
	// 0 (aktuell) -> 1 (vorherig) -> 2 (vorvorherig)

	// Position 1 nach Position 2 verschieben
	previous_position_2 = previous_position_1;
	// Position 0 nach Position 1 verschieben
	previous_position_1 = previous_position_0;

	// Zeitstempel entsprechend verschieben
	previous_time_micros_2 = previous_time_micros_1;
	previous_time_micros_1 = previous_time_micros_0;

	// -------------------------------------------------------------------------------
	// NEUE WERTE ERFASSEN
	// -------------------------------------------------------------------------------
	// Aktuelle Position vom Hardware-Encoder lesen
	previous_position_0 = encoder.read();
	// Aktuellen Zeitstempel in Mikrosekunden erfassen
	previous_time_micros_0 = micros();
}

// ===================================================================================
// GESCHWINDIGKEIT BERECHNEN
// ===================================================================================
// Berechnet die Geschwindigkeit aus den gespeicherten Positionswerten
// WICHTIG: update_stored_pos() muss regelmäßig aufgerufen werden für genaue Werte!
double myEncoder::get_velocity() {
	// -------------------------------------------------------------------------------
	// GESCHWINDIGKEITSBERECHNUNG MIT DIFFERENZENQUOTIENT
	// -------------------------------------------------------------------------------
	// Geschwindigkeit = (Position_neu - Position_alt) / (Zeit_neu - Zeit_alt)
	//
	// Hier wird der Unterschied zwischen Position 0 (aktuell) und Position 2 (vorvorherig)
	// verwendet, um über einen längeren Zeitraum zu mitteln und Rauschen zu reduzieren
	//
	// Formel: v = (Δposition * skalierung) / (Δzeit) * 1.000.000
	//         Der Faktor 1.000.000 wandelt von Mikrosekunden zu Sekunden um

	return (float)((previous_position_0 - previous_position_2) * output_scaling) / (float)(previous_time_micros_0 - previous_time_micros_2) * 1000000;

	// Auskommentierte Debug-Zeile:
	//return previous_time_micros_0 - previous_time_micros_2;
}

// ===================================================================================
// AKTUELLE POSITION AUSLESEN
// ===================================================================================
// Liest die aktuelle Encoder-Position und wendet Skalierung/Modulo an
double myEncoder::get_position() {
	//Möglichkeit A: Encoder-Wert direkt lesen und mit Skalierungsfaktor ins Bogenmaß umgewandelt
	return (float)encoder.read() * output_scaling;
	
	//Möglichkeit B: Encoder-Wert nur von 0 - 2pi begrenzen 
	/*
  uint32_t raw = encoder.read() % 8000;
	if (raw < 0) raw += 8000;  // negatives Ergebnis korrigieren
	return (double)raw * output_scaling;
  */
}

// ===================================================================================
// ENCODER-POSITION SETZEN
// ===================================================================================
// Setzt die Encoder-Position auf einen bestimmten Wert
// Der eingegebene Wert wird durch den Skalierungsfaktor geteilt, um den 
// entsprechenden rohen Encoder-Wert zu erhalten
void myEncoder::set_position(double position) {
	// Position durch Skalierungsfaktor teilen und als Integer an Hardware senden
	// Beispiel: Wenn position = 1.57 (π/2 Radiant) und output_scaling = 0.00628,
	//           dann wird encoder.write(250) aufgerufen
	encoder.write((uint32_t)(position / output_scaling));
}