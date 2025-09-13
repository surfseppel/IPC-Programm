#ifndef _MY_STEPPER_H_
#define _MY_STEPPER_H_

#include <TMCStepper.h>

//Globale Varialben
extern TMC5160Stepper driver;      //Objekt erstellen
extern bool motorDisable;          //Motortreiber deaktiviert
extern bool motorDirection;        //Motorrichtung 1=Rechtslauf, 0=Linkslauf
extern double absVelocitySet;      //Betrag der aktuell gesetzten Geschwindigkeit (muss noch nicht erreicht sein)
extern double absAccelerationSet;  //Betrag der aktuell gesetzten Bschleunigung (muss noch nicht erreicht sein)
extern uint8_t rampModeSetting;    //1=Rechtslauf, 2=Linkslauf

// =============================================================================
// TMC5160 Funktionen
// =============================================================================

//Einstellung des Motortreibers
void initStepper(void);
//Aktuelle Position des Motors auslesen
double getPosition(void);
//Aktuelle Position des Motors auf einen neuen Wert setzen
void setPosition(double position);
//Verfahren auf Position in der Anlage. Nur möglich, wenn Anlage referenziert
void goAbsolute(double position, double vel, double acc);
//Setzen der Richtung
void writeDirection(bool direction);
//Setzen der Geschwindikeit im VMAX Register
void writeVelocity(double vel);
//Setzen der Beschleunigung im AMAX Register
void writeAcceleration(double acceleration);
//Rückgabe der Geschwindigkeit in m/s
double getActualVelocity();
// Geschwindigkeit setzen mit max. Beschleunigung und pos. und neg. Richtung
void setVelocity(double vel);
// Beschleunigung setzen mit max. Geschwindigkeit und pos. und neg. Richtung
void setAcceleration(double acc);
// Geschwindigkeit und Beschleunigung setzen und Richtungskontrolle
void setVelocityAcceleration(double vel, double acc);
// Stoppen des Motors mit max. Beschleunigung
void quickStop();
// Motor deaktivieren
void disableMotor();
// Motor aktivieren
void enableMotor();
//Rückgabe der aktuellen Sollgeschwindigkeit (nicht Istgeschwindigkeit)
double getSetVelocity();
//Rückgabe der aktuellen Sollbeschleunigung (nicht Istbeschleunigung)
double getSetAcceleration();
//Ausgabe des driver status registers
void print_drv_status_register(void);

//----------------------
//noch nicht geschrieben
//----------------------
//StallGuard Konfiguration
void initStallGuard(void);
//Referenzfahrt: Testen der Endlagen und Ausrichten des Wagens
void referenceIPC(void);
// automatische Einstellung der StallGuard
void autoTuneStallGuard(void);

#endif  // _MY_STEPPER_H_