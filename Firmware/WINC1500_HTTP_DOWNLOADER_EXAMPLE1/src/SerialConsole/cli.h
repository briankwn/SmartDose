/*
 * cli.h
 *
 * Created: 11/18/2019 12:51:26 PM
 *  Author: gsuveer
 */ 


#ifndef CLI_H_
#define CLI_H_

#include "SerialConsole.h"
#include "mqtt.h"
#define ir_pin PIN_PA20
#define buzzer_pin PIN_PB03



/******************************************************************************
* Local Function Declaration
******************************************************************************/

uint8_t command[20]; //array for storing commands
uint8_t rxBuffRdy; //flag for rx buffer being ready   (INCONSISTENTCY)

//uint8_t cmdchar; //character
uint8_t cmditer; //location in read in command buffer


char nameofdevice[50]; //device name stored in RAM
char *inp; //pointer for input variable
char varinp[20]; //array for input variable

/* Instance of MQTT service. */
struct mqtt_module mqtt_instance;

void start_buzzing(void);
void stop_buzzing(void);
void read_ir(void);
void ReadIntoBuffer();
void execConsoleCommand(char *name);
void devName();
void ver_bl();
void help();
void ver_app();
void mac();
void ip();
void setDeviceName();
void getDeviceName();
void cli_init();
void pub_battery();




#endif /* CLI_H_ */