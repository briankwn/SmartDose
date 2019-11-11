/**
 * \file
 *
 * \brief SD/MMC card example with FatFs
 *
 * Copyright (c) 2014-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/**
 * \mainpage SD/MMC Card with FatFs Example
 *
 * \section Purpose
 *
 * This example shows how to implement the SD/MMC stack with the FatFS.
 * It will mount the file system and write a file in the card.
 *
 * The example outputs the information through the standard output (stdio).
 * To know the output used on the board, look in the conf_example.h file
 * and connect a terminal to the correct port.
 *
 * While using Xplained Pro evaluation kits, please attach I/O1 Xplained Pro
 * extension board to EXT1.
 *
 * \section Usage
 *
 * -# Build the program and download it into the board.
 * -# On the computer, open and configure a terminal application.
 * Refert to conf_example.h file.
 * -# Start the application.
 * -# In the terminal window, the following text should appear:
 *    \code
 *     -- SD/MMC Card Example on FatFs --
 *     -- Compiled: xxx xx xxxx xx:xx:xx --
 *     Please plug an SD, MMC card in slot.
 *    \endcode
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include "conf_example.h"
#include <string.h>
#include "SerialConsole/SerialConsole.h"
#include "sd_mmc_spi.h"
#include "ASF\sam0\drivers\dsu\crc32\crc32.h"

//! Structure for UART module connected to EDBG (used for unit test output)
struct usart_module cdc_uart_module;
static void jumpToApplication(void);


// Prototype for update_firmware
int8_t update_firmware();


int main(void)
{

	//INITIALIZE VARIABLES
	char test_file_name[] = "0:sd_mmc_test.txt";
	Ctrl_status status;
	FRESULT res;
	FATFS fs;
	FIL file_object;


	//INITIALIZE SYSTEM PERIPHERALS
	system_init();
	delay_init();
	InitializeSerialConsole();
	system_interrupt_enable_global();
	/* Initialize SD MMC stack */
	sd_mmc_init();

	irq_initialize_vectors();
	cpu_irq_enable();

	SerialConsoleWriteString("ESE516 - ENTER BOOTLOADER");	//Order to add string to TX Buffer
	 

	SerialConsoleWriteString("\x0C\n\r-- SD/MMC Card Example on FatFs --\n\r");

	//Check SD Card is mounted
	while (1) {
		SerialConsoleWriteString("Please plug an SD/MMC card in slot.\n\r");

		/* Wait card present and ready */
		do {
			status = sd_mmc_test_unit_ready(0);
			if (CTRL_FAIL == status) {
				SerialConsoleWriteString("Card install FAIL\n\r");
				SerialConsoleWriteString("Please unplug and re-plug the card.\n\r");
				while (CTRL_NO_PRESENT != sd_mmc_check(0)) {
				}
			}
		} while (CTRL_GOOD != status);

		
		//Attempt to mount a FAT file system on the SD Card using FATFS
		SerialConsoleWriteString("Mount disk (f_mount)...\r\n");
		memset(&fs, 0, sizeof(FATFS));
		res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
		if (FR_INVALID_DRIVE == res) {
			LogMessage(LOG_INFO_LVL ,"[FAIL] res %d\r\n", res);
			goto main_end_of_test;
		}
		SerialConsoleWriteString("[OK]\r\n");
	


		//Create and open a file
		SerialConsoleWriteString("Create a file (f_open)...\r\n");
		test_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0';
		res = f_open(&file_object,
				(char const *)test_file_name,
				FA_CREATE_ALWAYS | FA_WRITE);
		if (res != FR_OK) {
			LogMessage(LOG_INFO_LVL ,"[FAIL] res %d\r\n", res);
			goto main_end_of_test;
		}
		SerialConsoleWriteString("[OK]\r\n");

		//Write to a file
		SerialConsoleWriteString("Write to test file (f_puts)...\r\n");
		if (0 == f_puts("Test SD/MMC stack\n", &file_object)) {
			f_close(&file_object);
			LogMessage(LOG_INFO_LVL ,"[FAIL]\r\n");
			goto main_end_of_test;
		}
		
		SerialConsoleWriteString("[OK]\r\n");
		f_close(&file_object); //Close file
		SerialConsoleWriteString("Test is successful.\n\r");
		
		//start of params test
		char params_file_name[] = "params.csv";
		FIL params_file;

		// OPEN params.csv
		SerialConsoleWriteString("Opening params.csv\r\n");
		res = f_open(&params_file, (char const *)params_file_name, FA_READ);
		if (res != FR_OK) {
			LogMessage(LOG_INFO_LVL ,"[FAIL: Could not open params file] res %d\r\n", res);
			goto main_end_of_test;
		}
	
		char params[50];
		UINT br;
		res = f_read (&params_file,(char const *)params, 50, &br);
		if (res != FR_OK) {
			LogMessage(LOG_INFO_LVL ,"[FAIL: Could not read Parameters File] res %d\r\n", res);
			goto main_end_of_test;
		}
		LogMessage(LOG_INFO_LVL ,"[Bytes read from params.csv: ] %d\r\n", br);
		
		f_close(&params_file);		

		char * flag = strtok(params, ",");
		char * string_crc_from_file = strtok(NULL, ",");
		char * useless;
		uint32_t crc_from_file = strtoul(string_crc_from_file,useless,16); // could theoretically do this backwards and it'd be cleaner
		// Reading  from params.csv
		SerialConsoleWriteString("Reading from params.csv \r\n");

		SerialConsoleWriteString(flag);
		SerialConsoleWriteString("\r\n");
		SerialConsoleWriteString(crc_from_file);
		SerialConsoleWriteString("\r\n");
		//end of params test
		
		if(port_pin_get_input_level(BUTTON_0_PIN) == BUTTON_0_ACTIVE)
		{
				SerialConsoleWriteString("Button was pressed: UPDATING FIRMWARE \r\n");
				update_firmware(crc_from_file);
		}
		else if(strcmp("1",flag)==0){ 
				SerialConsoleWriteString("Update Flag was set: UPDATING FIRMWARE \r\n");
				update_firmware(crc_from_file);
		}
		else{   // Remember to insert condition when no app is present
				SerialConsoleWriteString("NO REASON TO UPDATE : JUMPING TO APPLICATION \r\n");
		}
		
		
main_end_of_test:
		SerialConsoleWriteString("Please unplug the card.\n\r");


		delay_s(1); //Delay to allow text to print
		cpu_irq_disable();

		//Deinitialize HW
		DeinitializeSerialConsole();
		sd_mmc_deinit();
		//Jump to application
		SerialConsoleWriteString("ESE516 - EXIT BOOTLOADER");	//Order to add string to TX Buffer
		jumpToApplication();

		while (CTRL_NO_PRESENT != sd_mmc_check(0)) {
		}
	}
}


#define APP_START_ADDRESS  ((uint32_t)0xb000) //Must be address of start of main application

/// Main application reset vector address
#define APP_START_RESET_VEC_ADDRESS (APP_START_ADDRESS+(uint32_t)0x04)

/**************************************************************************//**
* function      jumpToApplication()
* @brief        Jumps to main application
* @details      Detailed Description
* @note         Add a note
* @param[in]    arg1 Input parameter description
* @param[out]   arg2 Output parameter description
* @return       Description of return value
******************************************************************************/
static void jumpToApplication(void)
{
/// Function pointer to application section
void (*applicationCodeEntry)(void);

/// Rebase stack pointer
__set_MSP(*(uint32_t *) APP_START_ADDRESS);

/// Rebase vector table
SCB->VTOR = ((uint32_t) APP_START_ADDRESS & SCB_VTOR_TBLOFF_Msk);

/// Set pointer to application section
applicationCodeEntry =
(void (*)(void))(unsigned *)(*(unsigned *)(APP_START_RESET_VEC_ADDRESS));

/// Jump to application
applicationCodeEntry();
}


void configure_nvm(void)
{
	struct nvm_config config_nvm;
	nvm_get_config_defaults(&config_nvm);
	config_nvm.manual_page_write = false;
	nvm_set_config(&config_nvm);
}


int8_t update_firmware(uint32_t crc_from_file){
	//returns -1 if update failed, 0 if successful and ready to jump
	//find all necessary addresses to start at
			setLogLevel(LOG_INFO_LVL);
	//bomb entire application code region
	
	//for firmware image
			//start of params test
			char firmware_file_name[] = "0:app.bin";
			FIL firmware_file;
			FRESULT res;
			int8_t successful_update = 1;

			// OPEN params.csv
			SerialConsoleWriteString("READING app.bin \r\n");
			firmware_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0' ;
			res = f_open(&firmware_file, (char const *)firmware_file_name, FA_READ);
			if (res != FR_OK) {
				LogMessage(LOG_INFO_LVL ,"[FAIL: Could not open Firmware File] res %d\r\n", res);
				//set result to -1, file not read correctly
				successful_update = -1;
			}

			configure_nvm();
			struct nvm_parameters parameters;
			
			nvm_get_parameters 	( &parameters);				// To fetch parameter From out Device
			SerialConsoleWriteString("GOT NVM PARAMETERS \r\n");
			uint32_t page_size = parameters.page_size;		//Number of bytes per page --//page size is 64 bytes 
			uint32_t row_size = page_size * 4;//1028;				//Calculate row size from page size in bytes 
			uint32_t total_rows = parameters.nvm_number_of_pages /4 - (APP_START_ADDRESS/row_size);	
	
			uint32_t row_address = APP_START_ADDRESS;		//Start Address
			LogMessage(LOG_INFO_LVL,"PAGE SIZE IS %d bytes\r\n",page_size);
			LogMessage(LOG_INFO_LVL,"ROW  SIZE IS %d bytes\r\n",row_size);
			uint32_t crc_on_block=0;
			uint32_t crc_on_nvm=0;
			
			dsu_crc32_init();								//Initializing CRC
			
			SerialConsoleWriteString("ERASING APPLICATION CODE \r\n");			
			for (uint16_t i ; i < total_rows ; i++){ //using total rows means we delete the entire nvm instead of just enough to fit the new program in
				res = nvm_erase_row(row_address + (i * row_size));
				if (res != STATUS_OK) {
					LogMessage(LOG_INFO_LVL ,"[FAIL: NVM ROW DELETION] res %d\r\n", res);
					successful_update = -1; //set result to -1, erase not performed correctly
					break; //this will just move to write, need a better solution since jumping doesn't seem happy 
				}
			}
			
			SerialConsoleWriteString("STARTING MOVE BLOCKS \r\n");
			UINT br;
			char block[page_size];	
			//while(!f_eof(&firmware_file)) // While not end of Firmware file 
			for (uint32_t i ; i < (total_rows * 4) ; i++)
			{
					SerialConsoleWriteString("MOVING BLOCKS \r\n");
								
					res = f_read (&firmware_file,block, page_size, &br);
					if (res != FR_OK) {
						LogMessage(LOG_INFO_LVL ,"[FAIL: Could not read Block from Firmware File] res %d, bytes read %d\r\n", res, br);
						successful_update = -1; //set result to -1, file not read correctly
						break;
					}
					
					// Writing block on NVM
					res = nvm_write_buffer(row_address + (page_size * i), block, page_size);
					if (res != STATUS_OK) {
						LogMessage(LOG_INFO_LVL ,"[FAIL: WRITE ON BUFFER] res %d\r\n", res);
						successful_update = -1; //set result to -1, file not read correctly
						break;
					}
			}
			f_close(&firmware_file); // we're done with this now, although we could do this after the check
			
			//calculate CRC on NVM
			for (uint32_t i ; i < (total_rows * 4) ; i++){
				res = nvm_read_buffer(row_address + (page_size * i), block, page_size);
				if (res != STATUS_OK) {
					LogMessage(LOG_INFO_LVL ,"[FAIL: READ ON BUFFER] res %d\r\n", res);
					successful_update = -1; //set result to -1, file not read correctly
					break;
				}
				res = crc32_recalculate(block,page_size,&crc_on_nvm);
				if (res != STATUS_OK) {
					LogMessage(LOG_INFO_LVL ,"[FAIL: CRC RECALCULATE] res %d\r\n", res);
					successful_update = -1; //set result to -1, file not read correctly
					break;
				}
			}
			
			// CHECKING IF CRCs match
			if (crc_on_nvm == crc_from_file){ //we can rework this as a single if, just inverted							
				// All Went as Planned	
				//successful_update = 1;					
			}
			else
			{
				LogMessage(LOG_INFO_LVL ,"[FAIL: CRC DID NOT MATCH]\r\n");
				LogMessage(LOG_INFO_LVL ,"[FAIL: %d\r\n", crc_on_nvm);
				LogMessage(LOG_INFO_LVL ,"[FAIL: %d\r\n", crc_from_file);
				successful_update = -1; // we can theoretically use this as an informal jump, only continue if previous steps were successful
				//break;
				//Plan B - could recurse once or twice before we give up : )
			}	
			
	//read from flash into buffer
	//crc buffer
	//write buffer to nvm
	
	//for firmware image in nvm
	//read from nvm to buffer
	//crc buffer
	
	//compare crcs and return accordingly
	SerialConsoleWriteString("RETURNING FROM FIRMWARE UPDATE \r\n");
	return successful_update;
}