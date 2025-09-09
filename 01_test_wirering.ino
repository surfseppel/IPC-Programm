#include "myGlobals.h"
#include "myStepper.h"

//toDo:
/*
- Es wird erforderlich sich eine Funktion zu schreiben, die Positionieren kann.
setPositionMode() ???
goToPositon(double position)


- Taktfrequenz noch mal für den TMC5160 einstellen ???
- Stepper lib testen mit motor Referenzfahrt programmieren

??? vernüftige log Funktion programmieren mit den erforderlichen Prozessdaten

General Configuration Registers
	GCONF – Global configuration flags 	=> Bit 0 - Bit 17 
	GSTAT – Global status flags			=> Bit 0 - Bit 2
	INPUT								=> Bit 0 - Bit 7

global status flags

GSTAT – Global status flags 
Bit 0
Bit 1
Bit 2

*/

void setup() {
  Serial.begin(230400);
  delay(5000);
  initStepper();
  delay(1000);
  /*
  //langsam
  Serial.println("langsam fahren");
  driver.RAMPMODE(1); 
  writeAcceleration(0.1);
  writeVelocity(0.1);
  
  delay(2000);
  
  driver.RAMPMODE(2); 
  writeAcceleration(0.1);
  writeVelocity(0.1);

  delay(2000);

   //test disable motor
   Serial.println("Motor deaktivieren => Motor bleibt stehen, enablePin = HIGH");
   disableMotor();
   
   delay(5000);

   Serial.println("Motor aktivieren => Motor fährt wieder, enablePin = LOW"); 
   enableMotor();


   delay(5000);

  //schnell
  Serial.println("schnell fahren");
  driver.RAMPMODE(1); 
  writeAcceleration(1);
  writeVelocity(1);
  
  delay(2000);
  
  driver.RAMPMODE(2); 
  writeAcceleration(1);
  writeVelocity(1);

  delay(2000);

  //test stop motor
  Serial.println("Schnellstopp => Motor steht");
  quickStop();

  delay(2000);
*/
  //positionieren
  Serial.println("Positionierung");
 
 driver.RAMPMODE(0);

	driver.AMAX(1000);       //2. Beschleunigung in steps/s² wenn: v > v1  
	driver.DMAX(1000);       //1. Verzögerung in steps/s² wenn: v > v1
  driver.VMAX(1000);    // Maximale Geschwindigkeit in steps/s 
  driver.VSTART(0);       // Startgeschwindigkeit der Rampe hier nicht genutzt
	driver.VSTOP(10);       // Geschwindigkeit, bei der der Motor stoppt, wenn: v < 10, dann v = 0 
  driver.TZEROWAIT(0);    // Wartezeit zwischen Stoppen und starten des Motors
  driver.a1(1000);        //1. Beschleunigung in steps/s², wenn: v > VSTART und v < v1
  driver.d1(1000);        //2. Verzögerung in steps/s², wenn v < v1
	driver.v1(1000);       //Geschwindigkeit bei der die Beschleunigung umschaltet


  driver.XACTUAL(0);       //Aktuelle Position auf 0 setzen
  driver.XTARGET(-51200);  // Zielposition auf -51200 setzen
}

void loop() {
  delay(1000);

  auto xactual = driver.XACTUAL();  //aktuelle Position
  auto xtarget = driver.XTARGET();  //Zielposition
  auto vactual = driver.VACTUAL();  //aktuelle Geschwingigkeit

  char buffer[256];
  sprintf(buffer, "ioin=%#-10lx xactual=%7ld vactual=%7ld xtarget=%7ld\n", driver.IOIN(), xactual, vactual, xtarget);
  Serial.print(buffer);

  if (xactual == xtarget) {
    driver.XTARGET(-xactual);
  }

}



void print_drv_status_register(void) {
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

  // Nur SG_RESULT und StallGuard Flag ausgeben
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
