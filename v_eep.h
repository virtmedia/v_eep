/**
  ******************************************************************************
  * @file           v_eep.h
  * @brief          Header for v_eep.c file.
  *                   Library uses last FLASH page to store one(!) data
  *                   structure in non-volatile memory.
  *                   Library allows to use only one data structure in the
  *                   project. It stores 2 copies of the data, and manages
  *                   data corruption.
  *                   You can store us to 508B. One FLASH page is 1024B, and
  *                   the library stores 2 copies of the data + 2x4B CRC.
  * @author			Aleksander Kawęczyński e.vt0.pl
  * @date			2023-10-26
  * @version		1.0
  ******************************************************************************
  */

#ifndef INC_V_EEP_H_
#define INC_V_EEP_H_

#include "stm32f0xx_hal.h"

/**
 * Define here starting address of the last FLASH page, that will be used
 * for data storage. Please refer to the MCU documentation, or you can also
 * look on the ST-Link Utility -> Target -> Erase sectors to determine the
 * last page address.
 * \image html memory_map.jpg "Example memory map in ST-Link Utility"
 */
#define V_EEP_PAGE	0x08007C00

/**
 * @brief Reads data from FLASH to RAM to given address
 *
 * @param data		Pointer to the RAM address, when the data will be imported
 * @param data_s	Size of the data in bytes
 */
void v_eep_read(void *data, uint16_t data_s);

/**
 * @brief Writes data from RAM to FLASH. NOTE: data_s must be even!
 *
 * @param data		pointer to data in RAM
 * @param data_s	data size in bytes
 * @param hcrc		CRC peripheral address
 * @return			0-error (data bigger than 508B); 1- OK
 * @attention	Takes about 60ms!
 */
uint8_t v_eep_write(void *data, uint16_t data_s, CRC_HandleTypeDef *hcrc);

/**
 * @brief 	Compare checksum of data with stored CRC.
 * 			Mostly for internal use in the library.
 *
 * @param base_addr	FLASH page base address
 * @param data_s	data size in bytes
 * @param hcrc		CRC peripheral address
 * @return			0 - CRC mismatch, 1 - CRC valid
 */
uint8_t v_eep_verify(uint32_t *base_addr, uint_fast16_t data_s,
		CRC_HandleTypeDef *hcrc);

/**
 * @brief 	Read and verify data from the FLASH.
 * 			If corrupted, try to read copy and restore corrupted one. If both copies are corrupted, return 0
 *
 * @param data
 * @param data_s
 * @param hcrc
 * @return	0 - data is corrupted, 1- successful readout (and repair, if necessary)
 * @attention	If there is a need to write the data, it can take about 60ms!
 */
uint8_t v_eep_read_verified(void *data, uint16_t data_s,
		CRC_HandleTypeDef *hcrc);
#endif /* INC_V_EEP_H_ */
