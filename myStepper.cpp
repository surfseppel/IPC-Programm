//Einbinden der Bibliotheken
#include "myGlobals.h"
#include "myStepper.h"
#include <math.h>  // für fabs()

//Globale Varialben
TMC5160Stepper driver(CSPIN, 0.075);  //Hardware SPI, RMS Strom 3,11A
bool motorDisable = true;             //Motortreiber deaktiviert
bool motorDirection = true;           //Motorrichtung true=Rechtslauf, false=Linkslauf
double absVelocitySet = 0;            //Betrag der aktuell gesetzten Geschwindigkeit (muss noch nicht erreicht sein)
double absAccelerationSet = 0;        //Betrag der aktuell gesetzten Bschleunigung (muss noch nicht erreicht sein)
uint8_t rampModeSetting = 1;          //1=Rechtslauf, 2=Linkslauf
bool referenced = false;               //Anlage referenziert
MotionMode currentMode = MODE_VELOCITY;  // Standard: Geschwindigkeit

//Einstellung des Motortreibers
void initStepper(void) {
  if (DEBUG) {
    Serial.println("Initiating stepper");
  }
  //GPIO Konfiguration
  pinMode(VCC_IO_PIN, OUTPUT);
  pinMode(CLOCKPIN, OUTPUT);
  pinMode(ENABLEPIN, OUTPUT);
  pinMode(CSPIN, OUTPUT);

  //SPI starten, 5 MHz, MSB first, Mode 3 (TMC5160 erwartet Mode 3)
  driver.setSPISpeed(TMC5160_SPI_CLK);
  SPI.begin();

  //Setzen der Clock für den TMC5160Stepper mit ESP32 oder Arduino
  digitalWrite(CLOCKPIN, LOW);  //intere Clock (12MHz) aktiviert
  //???
  //Setzen der Clock für den TMC5160Stepper mit Teensy 4.1 
  //analogWriteFrequency(CLOCKPIN, F_CLK);
  //analogWrite(CLOCKPIN, 128);
  //??? ende

  //Reset Treiber
  digitalWrite(ENABLEPIN, HIGH);  //Treiber deaktivieren (low active)
  writeAcceleration(0);
  writeVelocity(0);
  digitalWrite(VCC_IO_PIN, LOW);  // Toggle VCC PIN => Reset driver
  delay(2);
  digitalWrite(VCC_IO_PIN, HIGH);
  digitalWrite(ENABLEPIN, LOW);  //Treiber aktivieren
  motorDisable = false;
  delay(5);

  //spreadCycle Mode
  driver.en_pwm_mode(false);  // Deaktiviert StealthChop → aktiviert SpreadCycle
  driver.TPWMTHRS(0);         //wenn = 0, kein stealthChop

  //spreadCycle Chopper Konfiguration
  driver.chm(0);  // chopper mode 0 = standard mode (spreadCycle)

  // Start excel sheet Berechnung
  driver.tbl(2);    // blanking time
  driver.toff(3);   // Stellt die Abschaltzeit (Off-Time) für den Chopper ein.
  driver.hstrt(0);  // Startpunkt Hysterese => über Excel Sheet bestimmen
  driver.hend(0);   // Endpunkt Hyterese => über Excel Sheet bestimmen
  //Ende Excel Sheet Berechnung

  //Umschaltung auf Vollschritt bei hohen Geschwindigkeiten
  //Aufgrund der Regelung sollte man das wahrscheinlich nicht ändern
  //driver.vhighchm(1);  // bei hoher Geschwindigkeit Vollschrittmodus
  //driver.vhighfs(1);   // bei hoher Geschwindigkeit Vollschrittmodus
  //driver.THIGH()          // Wenn die Geschwindigkeit > THIGH, wird in den Vollschrittmodus umgeschaltet, um volles Moment zu bekommen.

  //Einstellung der Anzahl der µSchritte
  driver.microsteps(U_STEPS);  //µSchritte pro Vollschritt

  //Motorstrom Konfiguration
  driver.rms_current(RMS_CURRENT);  // Stellt den effektiven (RMS) Motorstrom ein.
  //???driver.irun(8);         //100% = 31 des Motortreibers => 3,11A
  //???driver.ihold(4);        //30%  = 10 des Motortreibers => 1A
  driver.iholddelay(6);   //sanfte Stromreduzierung von Lauf- zu Haltestrom
  driver.TPOWERDOWN(10);  //Zeit vor Shutdown

  //Rampengenerator Begrenzung (Datenblatt S. 82)
  driver.AMAX(0);       //max pos. Beschleunigung
  driver.DMAX(0);       //max neg. Beschleunigung
  driver.VMAX(0);       // Maximale Geschwindigkeit in steps/s
  driver.VSTART(0);     // Startgeschwindigkeit der Rampe hier nicht genutzt
  driver.VSTOP(10);     // Geschwindigkeit, bei der der Motor stoppt, wenn: v < 10, dann v = 0
  driver.TZEROWAIT(0);  // Wartezeit zwischen Stoppen und starten des Motors

  //Setzen der aktuelle/Zielposition
  driver.XACTUAL(0);   //Aktuelle Position auf 0 setzen
  driver.XTARGET(0);   //Zielpositione auf 0 setzen
  driver.RAMPMODE(1);  //Rechtslauf setzen
  delay(5000);
  Serial.println("stepper initialized and enable");
}

//setzen des Modus Geschwindigkeitsmodus/Positionierungsmodus
void setMode(MotionMode mode) {
  currentMode = mode;
  if (currentMode == MODE_POSITION) {
    quickStop();  //??? Motor stoppen
    driver.RAMPMODE(0);
  } else if (currentMode == MODE_VELOCITY) {
    quickStop();         //??? Motor stoppen
    driver.RAMPMODE(1);  //??? auf Rechtslauf setzen
  } else {
    Serial.println("invalide mode");
  }
}

//Aktuelle Position des Motors auslesen
double getCurrentPosition(void) {
  return (float)driver.XACTUAL() * PULLEY_CIRCUM / double(CART_COUNT);  // Liest die aktuelle Position aus dem XACTUAL Register
}

//Zielposition des Motors auslesen
double getTargetPosition(void) {
  return (float)driver.XTARGET() * PULLEY_CIRCUM / double(CART_COUNT);  // Liest die Zielposition aus dem XTARGET Register
}

//Aktuelle Position des Motors mit einem Wert in Metern beschreiben,
//um den Verfahrweg zu referenzieren.
void setReference(double position) {
  //Begrenzung auf +/-1000 m
  if (position > 1000) position = 1000;
  if (position < -1000) position = -1000;
  driver.XACTUAL((int32_t)(position * CART_COUNT / PULLEY_CIRCUM));  // Setzt die aktuelle Position im XACTUAL Register
  referenced = true;
}

//Verfahren auf Position in der Anlage. Nur möglich, wenn Anlage referenziert ist
void goAbsolute(double position) {
  if (motorDisable) {
    if (DEBUG) {
      Serial.println("motor disable");
    }
    return;
  }
  if (!referenced) {
    if (DEBUG) {
      Serial.println("plant not referenced");
    }
    return;
  }
  //setzen auf Positionierungsmodus
  if (currentMode == MODE_VELOCITY) {
    setMode(MODE_POSITION);
  }
  driver.VSTART(0);
  driver.VSTOP(10);
  driver.TZEROWAIT(0);
  //Begrenzung auf +-1000 m
  if (position > 1000) position = 1000;
  if (position < -1000) position = -1000;
  driver.XTARGET((int32_t)(position * CART_COUNT / PULLEY_CIRCUM));  // Zielposition umwandeln in Inkremente
}

void writeDirection(bool direction) {
  // 1 ist positiv, 0 ist negative Richtung

  if (DEBUG) {
    Serial.print("setting dir: ");
    Serial.println(direction);
    Serial.print("motor dir: ");
    Serial.println(motorDirection);
  }
  //nur setzen, wenn eine Drehrichtungsänderung erforderlich
  if (direction == NEG_DIR && rampModeSetting != RAMPMODE_NEG) {
    rampModeSetting = RAMPMODE_NEG;  //Linkslauf setzen
    direction = 0;
  } else if (direction == POS_DIR && rampModeSetting != RAMPMODE_POS) {
    rampModeSetting = RAMPMODE_POS;  //Rechtslauf setzen
    direction = 1;
  } else {  //Drehrichtung ist korrekt
    return;
  }
  //Drehrichtung ändern
  driver.RAMPMODE(rampModeSetting);

  if (DEBUG) {
    Serial.print("RAMP mode: ");
    Serial.print(rampModeSetting, HEX);
    Serial.println();
    Serial.println();
    motorDirection = direction;
  }
  return;
}
//Setzen der Geschwindikeit im VMAX Register
void writeVelocity(double vel) {
  if (motorDisable) {
    return;  // Motor deaktiviert, erfordert Reset
  }
  double abs_vel = fabs(vel);  //Betrag bestimmen
  //Filter für minimale Abweichungen
  if (fabs(absVelocitySet - abs_vel) < 0.001) {
    return;  //Geschwindikeit ist ausreichend gesetzt
  }
  //Begrenzung der Geschwindigkeit zwischen 0 und max. Geschwindigkeit
  if (abs_vel > MAX_VEL) { abs_vel = MAX_VEL; }  //obere Begrenzung auf max. Geschwindigkeit
  abs_vel = abs_vel * (abs_vel > 0.00009);       // untere Begrenzung, ab wann 0 sein soll

  //Berechnung des VMAX Parameters im Motortreibers
  double vel_ustep = (abs_vel * MOTOR_STEPS * U_STEPS) / PULLEY_CIRCUM;  //  v in µsteps/s
  uint32_t VMAX_setting = (vel_ustep * pow(2, 24)) / F_CLK;              //VMAX Parameter
  if (VMAX_setting > 0x7FFE00) { VMAX_setting = 0x7FFE00; }              //VMAX auf Register Maximum begrenzen (2^23)-512
  driver.VMAX(VMAX_setting);                                             //Parameter schreiben
  //für Positionierung erforderlich
  if (currentMode == MODE_POSITION) { driver.v1(VMAX_setting); }
  //aktuell gesetzten Wert speichern, erforderlich für Filterung minimaler Abweichungen
  absVelocitySet = abs_vel;
  if (DEBUG) {
    Serial.print("vel: ");
    Serial.print(abs_vel);
    Serial.print("   ");
    Serial.println(VMAX_setting);
  }
}
//Setzen der Beschleunigung im AMAX Register
void writeAcceleration(double acceleration) {
  double abs_acc = fabs(acceleration);  //Betrag bestimmen

  //Filter für minimale Abweichungen
  if (fabs(absAccelerationSet - abs_acc) < 0.001) {
    return;  //Beschleunigung ist ausreichend gesetzt
  }
  //Begrenzung der Beschleunigung zwischen 0 und max. Beschleunigung
  if (abs_acc > MAX_ACC) { abs_acc = MAX_ACC; }  //obere Begrenzung auf max. Beschleunigung
  abs_acc = abs_acc * (abs_acc > 0.001);         //untere Begrenzung, ab wann 0 sein soll

  //Berechnung des AMAX Parameters im Motortreiber
  double acc_ustep = (abs_acc * MOTOR_STEPS * U_STEPS) / PULLEY_CIRCUM;                //a in µsteps/s²
  uint32_t AMAX_setting = (acc_ustep * pow(2, 41)) / ((double)F_CLK * (double)F_CLK);  //AMAX Parameter
  if (AMAX_setting > 0xFFFF) { AMAX_setting = 0xFFFF; }                                //AMAX auf Register Maximum begrenzen  0…(2^16)-1
  driver.AMAX(AMAX_setting);                                                           //Parameter schreiben
  //für Positionierung erforderlich
  if (currentMode == MODE_POSITION) {
    driver.DMAX(AMAX_setting);
    driver.a1(AMAX_setting);
    driver.d1(AMAX_setting);
  }

  //aktuell gesetzten Wert speichern, erforderlich für Filterung minimaler Abweichungen
  absAccelerationSet = abs_acc;
  if (DEBUG) {
    Serial.print("acc: ");
    Serial.print(abs_acc);
    Serial.print("   ");
    Serial.println(AMAX_setting);
  }
}
//Rückgabe der Geschwindigkeit in m/s
double getActualVelocity() {
  //Auslesen der aktuellen Geschwindigkeit
  uint32_t vActual = driver.VACTUAL();
  //Geschwindigkeit in µSteps/s
  double vel_ustep = (double)(vActual)*F_CLK / pow(2, 24);
  //Geschwindigkeit in m/s
  double vel = double(vel_ustep) * PULLEY_CIRCUM / double(MOTOR_STEPS * U_STEPS);
  return vel;
}
// Geschwindigkeit setzen mit max. Beschleunigung und pos. und neg. Richtung
void setVelocity(double vel) {
  //IEEE-754-Standard unterscheidet -0.0 und +0.0
  writeDirection(vel >= -0.000);  // -0.0 = true, -0.001 = false
  writeVelocity(vel);
  writeAcceleration(MAX_ACC);  //max. phys. Beschleunigung
}
// Beschleunigung setzen mit max. Geschwindigkeit und pos. und neg. Richtung
void setAcceleration(double acc) {
  //IEEE-754-Standard unterscheidet -0.0 und +0.0
  writeDirection(acc >= -0.000);  // -0.0 = true, -0.001 = false
  writeVelocity(MAX_VEL);         //max. phys. Geschwindigkeit
  writeAcceleration(acc);
}
// Geschwindigkeit und Beschleunigung setzen und Richtungskontrolle
void setVelocityAcceleration(double vel, double acc) {
  writeDirection(vel >= -0.000);
  writeVelocity(vel);
  writeAcceleration(acc);
}
// Stoppen des Motors mit max. Beschleunigung
void quickStop() {
  if (motorDisable) { return; }
  driver.AMAX(AMAX_MAX_ACC);  //max. Beschleunigung
  driver.VMAX(0);             //Geschwindigkeit auf 0
  if (DEBUG) { Serial.println("quick stop"); }
  return;
}
// Motor deaktivieren
void disableMotor() {
  digitalWrite(ENABLEPIN, HIGH);  //Motor deaktiviert
  referenced = false,                 //Referenz geht verloren
  motorDisable = true;
  if (DEBUG) { Serial.println("DRV disable"); }
}

// Motor aktivieren
void enableMotor() {
  digitalWrite(ENABLEPIN, LOW);  //Motor aktiviert
  motorDisable = false;
  if (DEBUG) { Serial.println("DRV enable"); }
}
//Rückgabe der aktuellen Sollgeschwindigkeit (nicht Istgeschwindigkeit)
double getSetVelocity() {
  if (motorDirection) {     //Auswertung der Drehrichtung
    return absVelocitySet;  //Rechtslauf
  } else {
    return -absVelocitySet;  //Linkslauf
  }
}
//Rückgabe der aktuellen Istbeschleunigung (nicht statischer Registerwert)
double getSetAcceleration() {
  if (motorDisable) {
    return 0;
  }
  //Wenn Geschwindigkeit erreicht, ist die Beschleunigung = 0
  else if (getActualVelocity() > absVelocitySet - 0.01) {
    return 0;
  }
  //Rückgabe der Zielbeschleunigung
  else if (motorDirection) {    //Auswertung der Drehrichtung
    return absAccelerationSet;  //Rechtslauf
  } else {
    return -absAccelerationSet;  //Linkslauf
  }
}
//Ausgabe des motion control registersn
void print_motion_ctl_register(void) {
  int32_t xactual = driver.XACTUAL();  //aktuelle Position
  int32_t xtarget = driver.XTARGET();  //Zielposition
  int32_t vactual = driver.VACTUAL();  //aktuelle Geschwingigkeit
  char buffer[256];
  sprintf(buffer, "ioin=%#-10lx xactual=%7ld vactual=%7ld xtarget=%7ld \n", driver.IOIN(), xactual, vactual, xtarget);
  Serial.println(buffer);
}

//Ausgabe des driver status registers
void log_drv_status_register(void) {
  uint32_t drv_status = driver.DRV_STATUS();
  // Einzelne Felder extrahieren
  uint16_t sg_result = drv_status & 0x03FF;
  bool s2vsa = drv_status & 0x1000;
  bool s2vsb = drv_status & 0x2000;
  bool stealth = drv_status & 0x4000;
  uint8_t cs_actual = (drv_status >> 16) & 0x1F;
  bool stallGuard = (drv_status >> 24) & 0x01;
  bool ot = (drv_status >> 25) & 0x01;
  bool otpw = (drv_status >> 26) & 0x01;
  bool s2ga = (drv_status >> 27) & 0x01;
  bool s2gb = (drv_status >> 28) & 0x01;
  bool ola = (drv_status >> 29) & 0x01;
  bool olb = (drv_status >> 30) & 0x01;
  bool stst = (drv_status >> 31) & 0x01;

  Serial.print("SG_RESULT: ");
  Serial.println(sg_result);
  Serial.print("s2vsa: ");
  Serial.println(s2vsa);
  Serial.print("s2vsb ");
  Serial.println(s2vsb);
  Serial.print("stealth: ");
  Serial.println(stealth);
  Serial.print("CS_ACTUAL (rms): ");
  Serial.println(cs_actual);  // RMS-Strom umgerechnet
  Serial.print(" StallGuard Flag: ");
  Serial.println(stallGuard);
  Serial.print("Overtemp Error: ");
  Serial.println(ot);
  Serial.print("Overtemp Warn: ");
  Serial.println(otpw);
  Serial.print("Short to GND A: ");
  Serial.println(s2ga);
  Serial.print("Short to GND B: ");
  Serial.println(s2gb);
  Serial.print("Open Load A: ");
  Serial.println(ola);
  Serial.print("Open Load B: ");
  Serial.println(olb);
  Serial.print("Driver Enabled (STST): ");
  Serial.println(stst);
}

//StallGuard Konfiguration
void initStallGuard(void) {
  //stallGuard aktivieren
  driver.TCOOLTHRS(0xFFFFF);  // Stoppen aktivieren wenn TCOOLTHRS ≥ TSTEP
  driver.THIGH(0);            // StallGuard immer aktiv
  driver.sg_stop(1);          //Stoppen des Motors aktieren

  //Parametrierung in der Applikation parametriert werden.
  //sg_result ausgeben => sollte bei Normalbetrieb <= 100 sein
  driver.sgt(4);  // StallGuard-Empfindlichkeit (-64..63)

  //Filterung
  driver.sedn(0b01);  // Anzahl der StallGuard2, 4-Bit Messungen pro Dekrement: 32, 8, 2, 1
  driver.sfilt(1);    //filter for more precision of the measurement.

  //Zusätzliche Parameter (??? zur Zeit noch nicht genutzt)
  //driver.semin(5);            // Untere Schwelle 4-Bit, wenn [sg < SEMIN*32] =>               bei 5 also 160 wird der Strom erhöht
  //driver.semax(2);            // Obere Schwelle, 4-Bit, wenn [sg > (SEMIN + SEMAX + 1)*32] => bei 2 also 256 wird der Strom gesenkt
  Serial.println("stallGuard initialized");
}

//Referenzfahrt: Testen der Endlagen und Ausrichten des Wagens auf die Mitte der Schiene
void referenceIPC(void) {
  /*
1. Nach links, bis stallGuard aktiv
2. Nach rechts fahren bis stallGuard aktiv
3. In die Mitte der Bahn fahren
4. Software Endlagen setzen auf -5 cm der Bahn 
*/
}
// automatische Einstellung der StallGuard
void autoTuneStallGuard(void) {
  /*
   1. normale Betriebsstromstärke und Versorgungsspannung verwenden.
   2. Motor mit niedriger Geschwindigkeit (<10 RPM) betreiben.
   3. In diesem Bereich ist die Motorlast kaum relevant für SG_RESULT.
   4. sfilt einschalten.
   5. SGT von 0 aus schrittweise erhöhen, bis SG_RESULT ansteigt.
   6. SGT dann leicht verringern, bis SG_RESULT gerade bei 0 bleibt.
   => Damit ist SGT optimal eingestellt für eine zuverlässige StallGuard-Erkennung.
   */
}

