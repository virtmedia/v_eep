/**
  ******************************************************************************
  * @file           : v_eep.c
  * @brief          : Code for v_eep library.
  *                   Library uses last FLASH page to store one(!) data
  *                   structure in non-volatile memory.
  *                   Library allows to use only one data structure in the
  *                   project. It stores 2 copies of the data, and manages
  *                   data corruption.
  ******************************************************************************
  */
#include "v_eep.h"


void v_eep_read(void *data, uint16_t data_s) {
	for (uint16_t i = 0; i < data_s; i = i + 2) {
		*((uint16_t*) (data + i)) = *(uint16_t*) (V_EEP_PAGE + i);
	}
}

uint8_t v_eep_write(void *data, uint16_t data_s, CRC_HandleTypeDef *hcrc) {
	if (data_s > 508)
		return 0;

	//4 because we include 4B CRC
	const uint32_t copy_base_add = V_EEP_PAGE + data_s + 4;

	HAL_FLASH_Unlock();
	//Erase page
	FLASH_EraseInitTypeDef FLASH_EraseInitStruct = { 0 };
	FLASH_EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	FLASH_EraseInitStruct.PageAddress = V_EEP_PAGE;
	FLASH_EraseInitStruct.NbPages = 1;
	uint32_t errorStatus = 0;
	HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &errorStatus);
	//write data to page
	for (uint16_t i = 0; i < data_s; i = i + 2) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (V_EEP_PAGE + i),
				*(uint16_t*) (data + i));
	}
	//after the data, calculate & store CRC
	uint32_t data_crc = 0;
	data_crc = HAL_CRC_Calculate(hcrc, (uint32_t*) data, data_s);

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (V_EEP_PAGE + data_s), data_crc);

	//write copy
	for (uint16_t i = 0; i < data_s; i = i + 2) {
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (copy_base_add + i),
				*(uint16_t*) (data + i));
	}
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (copy_base_add + data_s),
			data_crc);

	HAL_FLASH_Lock();
	return 1;
}

uint8_t v_eep_verify(uint32_t *base_addr, uint_fast16_t data_s,
		CRC_HandleTypeDef *hcrc) {
	uint32_t calculated_crc = 0, stored_crc = 0;
	calculated_crc = HAL_CRC_Calculate(hcrc, base_addr, data_s);
	base_addr += (data_s/4);
	stored_crc = *(__IO uint32_t*) (base_addr);

	return (calculated_crc == stored_crc);
}

v_eep_read_result_t v_eep_read_verified(void *data, uint16_t data_s,
		CRC_HandleTypeDef *hcrc) {
	uint_fast8_t is_1stOK = v_eep_verify((uint32_t*)V_EEP_PAGE, data_s, hcrc);
	uint_fast8_t is_2ndOK = v_eep_verify((uint32_t*) (V_EEP_PAGE + data_s + 4),
			data_s, hcrc);


	//both copies are corrupted - abort.
	if (!is_1stOK && !is_2ndOK)
		return V_EEP_RESULT_FAIL;

	//1st copy corrupted, restore from second one
	if (!is_1stOK) {
		for (uint16_t i = 0; i < data_s; i = i + 2) {
				*((uint16_t*) (data + i)) = *(uint16_t*) (V_EEP_PAGE + data_s + 4 + i);
		}
		v_eep_write(data, data_s, hcrc);
		v_eep_read(data, data_s);
		return V_EEP_RESULT_2NDOK;
	}

	//2nd copy corrupted, restore from first one
	if (!is_2ndOK) {
		v_eep_read(data, data_s);
		v_eep_write(data, data_s, hcrc);
		return V_EEP_RESULT_1STOK;
	}
	//All copies are ok, just readout
	v_eep_read(data, data_s);
	return V_EEP_RESULT_OK;
}

