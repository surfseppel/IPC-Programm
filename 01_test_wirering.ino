#include "myGlobals.h"
#include "myStepper.h"

//toDo:
/*
- Taktfrequenz noch mal für den TMC5160 einstellen ???
- Stepper lib testen mit motor Referenzfahrt programmieren

-teste 40 µSteps
-Anlage aktuell referenziert
*/

unsigned long startTime = millis();
 unsigned long intervalTime = 0;
double targetPosition = 0;
double velocity = 1.0;
double acceleration = 1.0;


void setup() {
  Serial.begin(230400);
  while (!Serial && millis() - startTime < 5000);
  initStepper();

  Serial.println("======================================");
  Serial.println("       Test Rampen                    ");
  Serial.println("======================================");

  //Messung der Zeit für das langsame Verfahren tges=10 sek.
  startTime = millis();
  delay(1000);
  Serial.println("");
  Serial.println("Fahren: langsam rechts");
  Serial.println("In 1 sek. auf 0.1 m/s beschleunigen");
  Serial.println("   2 sek. mit 0.1 m/s fahren");
  Serial.println("In 1 sek. auf 0.0 m/s bremsen");
  writeDirection(1);
  writeAcceleration(0.1);
  writeVelocity(0.1);
  delay(4000);
  writeVelocity(0.0);
  delay(1000);
  Serial.println("");
  Serial.println("Fahren: langsam links");
  Serial.println("In 1 sek. auf 0.1 m/s beschleunigen");
  Serial.println("   2 sek. mit 0.1 m/s fahren");
  Serial.println("In 1 sek. auf 0.0 m/s bremsen");
  writeDirection(0);
  writeAcceleration(0.1);
  writeVelocity(0.1);
  delay(4000);
  writeVelocity(0.0);
  while (getActualVelocity() >= 0.01);
  intervalTime = millis() - startTime;
  Serial.println("");
  Serial.print("Fahrdiagramm langsam tges: ");
  Serial.println(intervalTime);


  //Messung der Zeit für das schnelle Verfahren tges=30 sek.
  startTime = millis();
  delay(1000);
  Serial.println("");
  Serial.println("Fahren: schnell rechts");
  Serial.println("In 4 sek. auf 1 m/s beschleunigen");
  Serial.println("   4 sek. mit 1 m/s fahren");
  Serial.println("In 4 sek. auf 0 m/s bremsen");
  writeDirection(1);
  writeAcceleration(0.25);
  writeVelocity(1.0);
  delay(4000);
  writeVelocity(0.0);
  delay(1000);
  Serial.println("");
  Serial.println("Fahren: schnell links");
  Serial.println("In 1 sek. auf 0.1 m/s beschleunigen");
  Serial.println("   2 sek. mit 0.1 m/s fahren");
  Serial.println("In 1 sek. auf 0.0 m/s bremsen");
  writeDirection(0);
  writeAcceleration(0.25);
  writeVelocity(1.0);
  delay(4000);
  writeVelocity(0.0);
  while (getActualVelocity() >= 0.01);
  intervalTime = millis() - startTime;
  Serial.println("");
  Serial.print("Fahrdiagramm schnell tges: ");
  Serial.println(intervalTime);
  delay(2000);

  Serial.println("======================================");
  Serial.println("       Test Motor disable             ");
  Serial.println("======================================");
  delay(1000);
  setVelocity(1.0);
  delay(1000);
  //test disable motor
  Serial.println("Motor deaktivieren => Motor bleibt stehen, enablePin = HIGH");
  delay(1000);
  disableMotor();
  delay(1000);
  Serial.println("Motor aktivieren => Motor fährt wieder, enablePin = LOW");
  enableMotor();
  Serial.println("");
  Serial.println("======================================");
  Serial.println("       Test Motor quickstop           ");
  Serial.println("======================================");
  delay(1000);
  Serial.println("Schnellstopp => Motor steht");
  delay(1000);
  quickStop();
  delay(1000);

  Serial.println("======================================");
  Serial.println("       Test Motor Positionieren       ");
  Serial.println("======================================");

  setPosition(0);  
  targetPosition = 1.0;

  Serial.printf("Aktuelle Position: %f m", getPosition());
  startTime = millis();
}

void loop() {
  unsigned long currTime = millis();
  goAbsolute(targetPosition, velocity, acceleration); 

  if(currTime >= startTime + 1000){
    startTime = millis();
    Serial.print("Position: "); Serial.print (getPosition()); Serial.print("Geschwindigkeit: "); Serial.println(getActualVelocity());

  }
  
  if (getPosition() == targetPosition) {
    targetPosition = -targetPosition;
  }
}
/*
void log() {

  if (micros() > log_t_micros + 5'000) {
    log_t_micros = micros();
    SerialUSB1.print(millis() - time);
    SerialUSB1.print("\t");
    SerialUSB1.print(cart.get_position(), 4);
    SerialUSB1.print("\t");
    SerialUSB1.print(motor.get_actual_velocity(), 4);
    SerialUSB1.print("\t");
    SerialUSB1.print(pendulum.get_position(), 4);
    SerialUSB1.print("\t");
    SerialUSB1.print(pendulum.get_velocity(), 1);
    SerialUSB1.print("\t");
    SerialUSB1.print(ctrl.rtY.x_hat[0], 4);
    SerialUSB1.print("\t");
    SerialUSB1.print(ctrl.rtY.x_hat[1], 4);
    SerialUSB1.print("\t");
    SerialUSB1.print(ctrl.rtY.x_hat[2], 4);
    SerialUSB1.print("\t");
    SerialUSB1.print(ctrl.rtY.x_hat[3], 4);
    SerialUSB1.print("\t");
    SerialUSB1.println(motor.get_set_acceleration(), 4);
    SerialUSB1.send_now();
  }
  return;
 
}
*/
