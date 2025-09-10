#ifndef _MY_GLOBALS_H_
#define _MY_GLOBALS_H_
// =============================================================================
// HARDWARE PIN DEFINITIONS (ARDUINO_AVR_UNO)
// =============================================================================
#if defined(ARDUINO_AVR_UNO)
//TMC5160 
#define CSPIN 		0   //SPI chip select pin
#define VCC_IO_PIN 	0  	//Output 3.3-5V Spannungsversorgung
#define CLOCKPIN 	0  	//Output Takt für den Motortreiber
#define ENABLEPIN 	0	//Output HIGH=Driver disable, LOW=Driver enable 

//input switches
#define HOME_BUTTON 0   //InputPullup auf externer Taster (NO) für die Referenzierung der Anlage          
#define START_BUTTON 0  //InputPullup auf  externer Taster (NO) zum Starten der Anlage
//#define STOP_BUTTON 0//Not-Halt (NC) zum Stoppen der Anlage, direkt mit dem Motortreiber verbunden 
#define LEFT_SWITCH 0	//Endlage links
#define RIGHT_SWITCH 0 	//Endlage rechts
#define JOYSTICK 0		//Analogeingang für JOYSTICK

//Encoder inputs
#define PENDULUM_ENCODER_PIN_A      0       // Input Encoder A pin 
#define PENDULUM_ENCODER_PIN_B      1       // Input Encoder B pin 

// =============================================================================
// HARDWARE PIN DEFINITIONS (ESP32)
// =============================================================================
#elif defined(ESP32)
//TMC5160 
#define CSPIN 		5   //SPI chip select pin
#define VCC_IO_PIN 	21  //Output 3.3-5V Spannungsversorgung
#define CLOCKPIN 	22  //Output Takt für den Motortreiber
#define ENABLEPIN 	4	//Output HIGH=Driver disable, LOW=Driver enable 

//input switches
#define HOME_BUTTON 26  //InputPullup auf externer Taster (NO) für die Referenzierung der Anlage          
#define START_BUTTON 27 //InputPullup auf  externer Taster (NO) zum Starten der Anlage
//#define STOP_BUTTON 0//Not-Halt (NC) zum Stoppen der Anlage, direkt mit dem Motortreiber verbunden 
#define LEFT_SWITCH 25	//Endlage links
#define RIGHT_SWITCH 33 //Endlage rechts
#define JOYSTICK 36		//Analogeingang für JOYSTICK
 
//Encoder inputs
#define PENDULUM_ENCODER_PIN_A      0       // Input Encoder A pin 
#define PENDULUM_ENCODER_PIN_B      1       // Input Encoder B pin 
// =============================================================================
// HARDWARE PIN DEFINITIONS (TEENSYDUINO)
// =============================================================================
#elif defined(TEENSYDUINO)
//TMC5160 
#define CSPIN 		10	//SPI chip select pin
#define VCC_IO_PIN 	15 	//Output 3.3-5V Spannungsversorgung
#define CLOCKPIN 	13	//Output Takt für den Motortreiber
#define ENABLEPIN 	19	//Output HIGH=Driver disable, LOW=Driver enable 

//input switches
#define HOME_BUTTON 4   //InputPullup auf externer Taster (NO) für die Referenzierung der Anlage          
#define START_BUTTON 5  //InputPullup auf  externer Taster (NO) zum Starten der Anlage
//#define STOP_BUTTON 0//Not-Halt (NC) zum Stoppen der Anlage, direkt mit dem Motortreiber verbunden 
#define LEFT_SWITCH 35	//Endlage links
#define RIGHT_SWITCH 33 //Endlage rechts
#define JOYSTICK 23		//Analogeingang für JOYSTICK 

^//Encoder inputs
#define PENDULUM_ENCODER_PIN_A      0       // Input Encoder A pin 
#define PENDULUM_ENCODER_PIN_B      1       // Input Encoder B pin 

#else
    #error "Unbekanntes Board!"
#endif
// =============================================================================
// MECHANICAL PARAMETERS
// =============================================================================
#define PULLEY_CIRCUM 	0.157				//in m 
#define LENGTH 			1.0                 //in m
#define SAFETYZONE 		0.05                //in m
// =============================================================================
// MOTION LIMITS
// =============================================================================
#define HOME_VELOCIY 0.7     //in m/s
#define MAX_ACC 	13.0     //in m/s² (bei Umfang=0.157 maximal 13 m/s²)
#define MAX_VEL 	18.0     //in m/s (bei Umfang=0.157 maximal 18 m/s)

// =============================================================================
// PENDULUM AND CART ENCODER
// =============================================================================
#define CART_COUNT 			U_STEPS*MOTOR_STEPS			// Steps per round 
#define PENDULUM_COUNT 		8000						// Steps per round
#define CART_SCALING 		PULLEY_CIRCUM/CART_COUNT 	//Weg pro Inkrement	
#define PENDULUM_SCALING 	2*PI/PENDULUM_COUNT			//Radiant pro Inkrement

// =============================================================================
// DIRECTION DEFINITIONS
// =============================================================================
#define POS_DIR 		1            //Rechtslauf
#define NEG_DIR 		0            //Linkslauf
#define RAMPMODE_POS 	1
#define RAMPMODE_NEG 	2

// =============================================================================
// REFERENCE DEFINITIONS
// =============================================================================
#define REFERENCED 0 		//0 Anlage nicht referenziert/1 Anlage referenziert 

// =============================================================================
// TMC5160 SPECIFIC CONSTANTS
// =============================================================================
#define F_CLK 			12000000UL			// TMC5160 clock frequency Hz
#define TMC5160_SPI_CLK 5000000             //5Mhz (max. half F_CLK)
#define RMS_CURRENT     400                 //RMS Motorstrom in mA
#define POW_2_24 		16777216.0  		// 2^24 for calculations
#define POW_2_41 		2199023255552.0  	// 2^41 for calculations
#define AMAX_MAX_ACC 	65535     		  	// Maximum AMAX register value
#define U_STEPS 		256L      			//µSteps pro Vollschritt
#define MOTOR_STEPS 	200L  				//200 Vollschritte

// =============================================================================
// DEBUGGING AND DIAGNOSTICS
// =============================================================================
#define DEBUG 0  //0=off, 1=on
#endif