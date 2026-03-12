/*
Das Programm gibt alle 10ms den aktuellen Wert des Inkrementalgebers auf. 
Serial.print() ist per default auf 2 Nachkommastellen konfiguriert bspw. 0.00
Mit Serial.print (variable, 10) können weitere Nachkommastellen angezeigt werden  
*/
#include "myEncoder.h"
#include "myGlobals.h"

unsigned int printPrecision = 3;

//timer
unsigned int intervall = 10;
unsigned long lastPrint = 0;
unsigned long currentTime = 0;


//pendulum position
double pendPos = 0;

unsigned short step = 0;
myEncoder pendulumEncoder(PENDULUM_ENCODER_PIN_A, PENDULUM_ENCODER_PIN_B, 1, PENDULUM_SCALING);

void setup() {
  //wait for serial init
  while (!Serial && millis() < 4000);
  pendulumEncoder.init();
  pendulumEncoder.set_position(0);
}

void loop() {
  //only for velocity test
  //pendulumEncoder.update_stored_pos();
  
  currentTime = millis();
  if (currentTime >= lastPrint + intervall) {
    lastPrint = currentTime;
    //save measurements
    pos_1 = pendPos;
    pendPos = pendulumEncoder.get_position();
	Serial.println(pendPos, printPrecision);      
	//Only for velocity test
	//Serial.print(",");
    //Serial.println(pendulumEncoder.get_velocity(), printPrecision);
    }
}
