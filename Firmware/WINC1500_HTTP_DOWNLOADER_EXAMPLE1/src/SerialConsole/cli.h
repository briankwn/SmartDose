/*
 * cli.h
 *
 * Created: 11/18/2019 12:51:26 PM
 *  Author: gsuveer
 */ 


#ifndef CLI_H_
#define CLI_H_

#include "SerialConsole.h"

#define ir_pin PIN_PA20
#define buzzer_pin PIN_PB03



/******************************************************************************
* Local Function Declaration
******************************************************************************/

void start_buzzing(void);
void stop_buzzing(void);
void read_ir(void);
void ReadIntoBuffer();
void execConsoleCommand(char *name)

uint8_t command[20]; //array for storing commands
uint8_t rxBuffRdy = 0; //flag for rx buffer being ready   (INCONSISTENTCY)

//uint8_t cmdchar; //character
uint8_t cmditer = 0; //location in read in command buffer
const char bkspc[3] ={0x20,0x08}; //backspace for printing to terminal

char nameofdevice[] = "not yet set. Use setDeviceName"; //device name stored in RAM
char *inp = NULL; //pointer for input variable
char varinp[20]; //array for input variable




#endif /* CLI_H_ */