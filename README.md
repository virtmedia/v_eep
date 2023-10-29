# v_eep
 Simple >>Virtual EEPROM<< library for STM32.

 Library uses last FLASH page to store one(!) data
structure in non-volatile memory.
Library allows to use only one data structure in the
project. It stores 2 copies of the data, and manages
data corruption.
You can store us to 508B. One FLASH page is 1024B, and
the library stores 2 copies of the data + 2x4B CRC.




[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://choosealicense.com/licenses/mit/)



## Usage/Examples

Remember to change following define in v_eep.h to FLASH page of your selection:

```c
#define V_EEP_PAGE	0x08007C00
```
### How to find a proper FLASH page address?
Choose last page of your FLASH. Please refer to the MCU documentation, or you can also
look on the ST-Link Utility -> Target -> Erase sectors to determine the
last page address.


```c

#include "v_eep.h"

struct myImportantData {
    int someParameter;
    int someArray[10];
    uint32_t timer;
} myImportantData;

main(){
	HAL_IWDG_Refresh(&hiwdg);
	v_eep_read_result_t res = v_eep_read_verified(&myImportantData, sizeof(myImportantData), &hcrc);
	HAL_IWDG_Refresh(&hiwdg);
	if( res == V_EEP_RESULT_OK ){

		sprintf(tbuf,"Data restored from Virtual EEPROM!\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t*)tbuf, strlen(tbuf), HAL_MAX_DELAY);
	}else if( res == V_EEP_RESULT_1STOK ){

		sprintf(tbuf,"Data restored from 1st copy of Virtual EEPROM!\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t*)tbuf, strlen(tbuf), HAL_MAX_DELAY);
	}else if( res == V_EEP_RESULT_2NDOK ){

		sprintf(tbuf,"Data restored from 2nd copy of Virtual EEPROM!\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t*)tbuf, strlen(tbuf), HAL_MAX_DELAY);
	}else if( res == V_EEP_RESULT_FAIL ){

		sprintf(tbuf,"Both copies of the data corrupted! Initializing with default values.\r\n");
		HAL_UART_Transmit(&huart2, (uint8_t*)tbuf, strlen(tbuf), HAL_MAX_DELAY);
	}
	HAL_IWDG_Refresh(&hiwdg);
	///...

	myImportantData.someParameter += 1;

	v_eep_write(&myImportantData, sizeof(myImportantData), &hcrc);
	HAL_IWDG_Refresh(&hiwdg);
	sprintf(tbuf,"someParameter = %d\r\n", myImportantData.someParameter);
	HAL_UART_Transmit(&huart2, (uint8_t*)tbuf, strlen(tbuf), HAL_MAX_DELAY);
}

```

