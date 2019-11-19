/*
 * cli.c
 *
 * Created: 11/18/2019 12:51:06 PM
 *  Author: gsuveer
 */ 

#include "cli.h"
#include "main.h"
/*
COMMAND LINE INTERFACE COMMANDS
*/
const char bkspc[3]={0x20 ,0x08}; //backspace for printing to terminal
void ReadIntoBuffer(){
	if(rxBuffRdy > 0){
		if(SerialConsoleReadCharacter(&command[cmditer]) == 0){
			if(command[cmditer] == '\r'){ //carriage return detect
				SerialConsoleWriteString("\r\n"); // fixing mirroring for carriage return
				
				command[cmditer] = '\0'; // turn into c string for processing
				
				//execute appropriate command here
				execConsoleCommand(&command);
				memset(command, 0, 20);//clear command buffer when done with command
				cmditer = -1; //reset index once buffer is cleared
			}
			if(command[cmditer] == '\b'){
				SerialConsoleWriteString(bkspc);
				command[cmditer] = '\0';
				cmditer --;
				command[cmditer]= '\0';
				cmditer --;
			}
			//increment array index and decrement rx flag
			cmditer ++;
			rxBuffRdy --;
		}
		
	}
	
}



void help(){
	SerialConsoleWriteString("\r\n");
	SerialConsoleWriteString(
	"	help                        Prints this message\r\n\
	ver_bl                      Prints the bootloader firmware version\r\n\
	ver_app                     Prints the application code firmware version\r\n\
	mac                         Prints the mac address of device\r\n\
	ip                          Prints the ip  address of device\r\n\
	devName                     Prints the name of the developer\r\n\
	setDeviceName <string name> Sets the name of the device to the given string\r\n\
	getDeviceName               gets the name of the device\r\n");
}
//static strings are just in there to make the assignment complete. cleaning up when I get enough time
char outstring[30];
void devName(){
	SerialConsoleWriteString("Brian Kwon\r\n");
}
void ver_bl(){
	SerialConsoleWriteString("12.1.1\r\n");
}
void ver_app(){
	SerialConsoleWriteString("18.3.8\r\n");
}
void mac(){
	SerialConsoleWriteString("91-26-96-C3-F5-7B\r\n");
}
void ip(){
	SerialConsoleWriteString("8.8.8.8\r\n");
}
void setDeviceName(){
	strcpy(nameofdevice, varinp);
	strcpy(outstring,"Device name set to ");
	strcat(outstring, nameofdevice);
	SerialConsoleWriteString(outstring);
	SerialConsoleWriteString("\r\n");
	memset(outstring, 0, 30); //clear outstring when done with it
	
}
void getDeviceName(){
	strcpy(outstring, "Device name is ");
	strcat(outstring, nameofdevice);
	SerialConsoleWriteString(outstring);
	SerialConsoleWriteString("\r\n");
	memset(outstring, 0, 30); //clear outstring when done with it
}

void start_buzzing()
{
	port_pin_set_output_level(buzzer_pin, true);
}

void stop_buzzing()
{
	port_pin_set_output_level(buzzer_pin, false);
}

void read_ir()
{
	bool level=port_pin_get_input_level(ir_pin);
	SerialConsoleWriteString("Current value of IR_Sensor is:");
	if (level == false ) SerialConsoleWriteString("true\r\n"); //Why?
	else SerialConsoleWriteString("false\r\n");
}

//create name to function struct
struct cmdname{
	const char *name;
	int (*cmd) (void);
};

//array of functions/commands
struct cmdname commandarr[] =
{
	{ "help", help},
	{ "ver_bl", ver_bl},
	{ "ver_app", ver_app},
	{ "mac", mac },
	{ "ip", ip},
	{ "devName", devName},
	{ "setDeviceName", setDeviceName},
	{ "getDeviceName", getDeviceName},
	{"start_buzzing" , start_buzzing},
	{"stop_buzzing", stop_buzzing},
	{"read_ir", read_ir},
	{"pub_battery", pub_battery},
	{"request_pill", request_pill},
	{"sound_alarm", sound_alarm}		
};
enum {NUMOFCMDS = sizeof(commandarr)/sizeof(commandarr[0])};

//loops through array of functions, matches name, calls corresponding function
void execConsoleCommand(char *name){
	//first handle spaces and multi-input
	inp = NULL;
	inp = strchr(name,' '); //get pointer of space character
	if(inp != NULL){ // if we find a space
		memset(varinp, 0, 20); // clear input variable
		strcpy(varinp, (inp +1)); //copy everything after the space into new variable
		*inp = '\0'; //terminate the string on the space
	}
	
	for (int i = 0; i < NUMOFCMDS; i++){
		if (strcmp(name, commandarr[i].name) == 0){
			commandarr[i].cmd();
			memset(varinp, 0, 20); // clear input variable -- prevents using the <devicename> input from the previous function, still bugs on error <deviceName>
			return;
		}
	}
	SerialConsoleWriteString("ERROR\r\n"); // if we get to here there's no match
}


void cli_init(struct mqtt_module *mqtt_inst_ref){
	mqtt_instance = *mqtt_inst_ref;
	cmditer=0; //location in read in command buffer
	rxBuffRdy = 0;
	strcpy(nameofdevice, "not yet set. Use setDeviceName"); //device name stored in RAM
	inp = NULL; //pointer for input variable
	
}

void pub_battery(){
	volatile char mqtt_msg[64]= "{\"d\":{\"bat_level\":50}}";
	mqtt_publish(&mqtt_instance, BATTERY_TOPIC, mqtt_msg, strlen(mqtt_msg), QOS, RETAIN);	// change qos = and retain dynamicall
}

void request_pill(){
	volatile char mqtt_msg[64]= "{\"d\":{\"pill_request\":true}}";
	mqtt_publish(&mqtt_instance, PILL_REQUEST_TOPIC, mqtt_msg, strlen(mqtt_msg), QOS, RETAIN);
}

void sound_alarm(){
	volatile char mqtt_msg[64]= "{\"d\":{\"alarm\":true}}";
	mqtt_publish(&mqtt_instance, ALARM_TOPIC, mqtt_msg, strlen(mqtt_msg), QOS, RETAIN);
}