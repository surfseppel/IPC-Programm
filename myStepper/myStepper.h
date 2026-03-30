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


typedef enum {
    MODE_VELOCITY,   // Geschwindigkeitsmodus
    MODE_POSITION    // Positioniermodus
} MotionMode;

// =============================================================================
// TMC5160 Basisfunktionen
// =============================================================================
//Einstellung des Motortreibers
void initStepper(void);
//setzen des Modus Geschwindigkeitsmodus/Positionierungsmodus mit
//MODE_VELOCITY/MODE_POSITION
void setMode(MotionMode mode);
//Setzen der Richtung mit RIGHT/LEFT
void writeDirection(bool direction);
//Setzen der Geschwindikeit im VMAX Register
void writeVelocity(double vel);
//Setzen der Beschleunigung im AMAX Register
void writeAcceleration(double acceleration);
// Stoppen des Motors mit max. Beschleunigung
void quickStop();
// Motor deaktivieren
void disableMotor();
// Motor aktivieren
void enableMotor();
// =============================================================================
// TMC5160 Funktionen für Mode_Position
// =============================================================================
//Aktuelle Position des Motors auslesen
double getCurrentPosition(void);
//Zielposition des Motors auslesen
double getTargetPosition(void);
//Den Motor in der aktuellen Position auf einen Wert in Metern referenzieren. 
void setReference(double position);
//Verfahren auf Position in der Anlage. Nur möglich, wenn Anlage referenziert
void goAbsolute(double position);

// =============================================================================
// TMC5160 Funktionen zur Regelung des Pendels
// =============================================================================
//Rückgabe der Geschwindigkeit in m/s
double getActualVelocity();
// Geschwindigkeit setzen mit max. Beschleunigung und pos. und neg. Richtung
void setVelocity(double vel);
// Beschleunigung setzen mit max. Geschwindigkeit und pos. und neg. Richtung
void setAcceleration(double acc);
// Geschwindigkeit und Beschleunigung setzen und Richtungskontrolle
void setVelocityAcceleration(double vel, double acc);
//Rückgabe der aktuellen Sollgeschwindigkeit (nicht Istgeschwindigkeit)
double getSetVelocity();
//Rückgabe der aktuellen Sollbeschleunigung (nicht Istbeschleunigung)
double getSetAcceleration();
// =============================================================================
// TMC5160 Debug Funktionen
// =============================================================================
//Ausgabe des motion control registers
void print_motion_ctl_register(void);
//Ausgabe des driver status registers
void print_drv_status_register(void);
// =============================================================================
// TMC5160 Nicht implementierte Funktionen
// =============================================================================
//StallGuard Konfiguration
void initStallGuard(void);
//Referenzfahrt: Testen der Endlagen und Ausrichten des Wagens
void referenceIPC(void);
// automatische Einstellung der StallGuard
void autoTuneStallGuard(void);

#endif  // _MY_STEPPER_H_