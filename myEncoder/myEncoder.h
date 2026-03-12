// myEncoder.h
// ===================================================================================
// ENCODER-WRAPPER KLASSE FÜR QUADRATUR-ENCODER
// ===================================================================================
// Diese Klasse stellt eine plattformunabhängige Schnittstelle zu Quadratur-Encodern dar.
// Sie funktioniert sowohl mit Arduino Encoder als auch mit Teensy QuadEncoder Bibliothek.
// Die Klasse ermöglicht es, Encoder-Schritte in gewünschte Einheiten umzurechnen
// und bietet Funktionen für Position und Geschwindigkeitsmessung.

#ifndef _MY_ENCODER_H_
#define _MY_ENCODER_H_

#include <Arduino.h>

// Plattformspezifische Includes
#if defined(TEENSYDUINO)
    #include <QuadEncoder.h>
#else
    #include <Encoder.h>
#endif

class myEncoder
{

public:
	// -------------------------------------------------------------------------
	// KONSTRUKTOR
	// -------------------------------------------------------------------------
	// A_pin:       GPIO-Pin für Encoder-Signal A
	// B_pin:       GPIO-Pin für Encoder-Signal B  
	// channel:      Bis zu 4 Encoder möglich 1,2,3,4  
	// out_scaling: Skalierungsfaktor für Ausgabewerte in Meter oder Rad
	myEncoder(uint8_t A_pin, uint8_t B_pin, uint8_t chanel, double out_scaling);
	
	// Encoder initialisieren
	void init();
	// Position in skalierten Einheiten lesen
	double get_position();
	// Position in skalierten Einheiten setzen
	void set_position(double position);
	// Geschwindigkeit ermitteln (update_stored_pos() regelmäßig aufrufen!)
	double get_velocity();
	// Position für Geschwindigkeitsberechnung aktualisieren
	void update_stored_pos();
	
	// Plattformspezifisches Encoder-Objekt
	#if defined(TEENSYDUINO)
		QuadEncoder encoder;
	#else
		Encoder encoder;
	#endif

protected:
	double output_scaling;    // Skalierungsfaktor für Ausgabewerte	
	int previous_position_0;           // Aktuelle Position (Encoder-Schritte)
	unsigned long previous_time_micros_0;  // Zeitstempel der aktuellen Position	
	int previous_position_1;           // Vorherige Position
	unsigned long previous_time_micros_1;  // Zeitstempel der vorherigen Position	
	int previous_position_2;           // Vorvorherige Position  
	unsigned long previous_time_micros_2;  // Zeitstempel der vorvorherigen Position
	unsigned long update_period;      // Mindest-Zeitabstand zwischen Updates (Mikrosekunden)
};

#endif 