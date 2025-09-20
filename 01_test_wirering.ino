#include "myGlobals.h"
#include "myStepper.h"

//toDo:
/*
- Taktfrequenz noch mal für den TMC5160 einstellen ???
- Stepper lib testen mit motor Referenzfahrt programmieren
- Am Ende noch mal nach den ??? schauen
!!! Anlage aktuell referenziert !!!
Referenzflag in disableMotor wirder auf 0  ???ref geht verloren????


*/

unsigned long startTime = millis();
unsigned long intervalTime = 0;
double targetPosition = 0;
double velocity = 1.0;
double acceleration = 1.0;
unsigned long currTime = 0;


void setup() {
  Serial.begin(230400);
  while (!Serial && millis() - startTime < 5000)
    ;
  initStepper();
  Serial.println();
  Serial.println("======================================");
  Serial.println("       Test Rampen                    ");
  Serial.println("======================================");
  delay(5000);

  Serial.println("");
  Serial.println("Fahren: langsam rechts");
  Serial.println("In 1 sek. auf 0.1 m/s beschleunigen");
  Serial.println("   8 sek. mit 0.1 m/s fahren");
  Serial.println("In 1 sek. auf 0.0 m/s bremsen");

  //Messung der Zeit für das langsame Verfahren tges=20 sek.
  startTime = millis();
  writeDirection(1);
  writeAcceleration(0.1);
  writeVelocity(0.1);
  delay(9000);
  writeVelocity(0.0);
  while (getActualVelocity() >= 0.001)
    ;

  Serial.println("");
  Serial.println("Fahren: langsam links");
  Serial.println("In 1 sek. auf 0.1 m/s beschleunigen");
  Serial.println("   8 sek. mit 0.1 m/s fahren");
  Serial.println("In 1 sek. auf 0.0 m/s bremsen");
  writeDirection(0);
  writeAcceleration(0.1);
  writeVelocity(0.1);
  delay(9000);
  writeVelocity(0.0);
  while (getActualVelocity() >= 0.001)
    ;

  intervalTime = millis() - startTime;
  Serial.println("");
  Serial.print("Fahrdiagramm langsam SOLL tges:20 sek. IST tges: ");
  Serial.println(intervalTime);
  delay(10000);

  Serial.println("");
  Serial.println("Fahren: schnell rechts");
  Serial.println("In 4 sek. auf 1 m/s beschleunigen");
  Serial.println("   4 sek. mit 1 m/s fahren");
  Serial.println("In 4 sek. auf 0 m/s bremsen");
  //Messung der Zeit für das schnelle Verfahren tges=24 sek.
  startTime = millis();
  writeDirection(1);
  writeAcceleration(0.25);
  writeVelocity(1.0);
  delay(8000);
  writeVelocity(0.0);
  while (getActualVelocity() >= 0.001);

  Serial.println("");
  Serial.println("Fahren: schnell links");
  Serial.println("In 4 sek. auf 0.1 m/s beschleunigen");
  Serial.println("   4 sek. mit 0.1 m/s fahren");
  Serial.println("In 4 sek. auf 0.0 m/s bremsen");
  writeDirection(0);
  writeAcceleration(0.25);
  writeVelocity(1.0);
  delay(8000);
  writeVelocity(0.0);
  while (getActualVelocity() >= 0.001)
    ;

  intervalTime = millis() - startTime;
  Serial.println("");
  Serial.print("Fahrdiagramm schnell SOLL tges=24 sek.  IST tges: ");
  Serial.println(intervalTime);
  delay(10000);
  Serial.println("");
  Serial.println("======================================");
  Serial.println("       Test Motor disable             ");
  Serial.println("======================================");
  setVelocity(1.0);  //max. Acc
  delay(5000);
  //test disable motor
  Serial.println("Motor deaktivieren => Motor bleibt stehen, enablePin = HIGH");
  disableMotor();
  delay(2000);
  Serial.println("Motor aktivieren => Motor fährt wieder, enablePin = LOW");
  enableMotor();
  Serial.println("");
  delay(10000);

  Serial.println("======================================");
  Serial.println("       Test Motor quickstop           ");
  Serial.println("======================================");
  Serial.println("Schnellstopp => Motor steht");
  quickStop();
  delay(5000);
  Serial.println("");
  Serial.println("======================================");
  Serial.println("       Test Motor Positionieren       ");
  Serial.println("======================================");
  delay(5000);

  //Setzen Mode Positionierung mit Geschwindigkeit, Beschleunigung und Position
  setMode(MODE_POSITION);
  writeVelocity(2);
  writeAcceleration(0.5);
  goAbsolute(0);

  //Motor auf die Position zum Programmstart verfahren
  while (getCurrentPosition() > 0.01) {
    delay(500);
    Serial.printf("Aktuelle Position: %f m | Zielposition: %f m \n", getCurrentPosition(), getTargetPosition());
  }
  Serial.println("");
  Serial.println("Verfahre 2 Umdrehungen - hin/her.");
  Serial.println("");
  delay(5000);
  goAbsolute(0.157); //
  startTime = millis();
}

void loop() {
  currTime = millis();
  if (currTime >= startTime + 100) {
    startTime = currTime;
    double vel = getSetVelocity();
    double acc = getSetAcceleration();
    Serial.print("vel: "); Serial.print(vel); Serial.print(" acc: "); Serial.println(acc);
    
  }

  if (getCurrentPosition() == getTargetPosition() ) {
    goAbsolute(-getCurrentPosition());
    Serial.printf("Aktuelle Position: %f m | Zielposition: %f m \n", getCurrentPosition(), getTargetPosition());
    delay(1000);
  }
}


