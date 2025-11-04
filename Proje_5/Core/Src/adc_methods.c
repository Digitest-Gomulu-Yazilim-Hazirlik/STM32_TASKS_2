	#include "adc_methods.h"
	#include <stdio.h>
	#include <string.h>
	extern UART_HandleTypeDef huart2;

	#define VREF_CAL_ADDR ((uint16_t*)0x1FFF7A2A) // VREF kalibrasyonu
	#define TS_CAL1_ADDR ((uint16_t*)0x1FFF7A2C)  // 30°C kalibrasyon
	#define TS_CAL2_ADDR ((uint16_t*)0x1FFF7A2E)  // 110°C kalibrasyon

	volatile uint32_t adc_value = 0;
	volatile float temperature = 0.0f;

	// =====================================================
	// 🧩 POLLING YÖNTEMİ
	// =====================================================
	#ifdef USE_POLLING
	void Read_Temperature_Polling(ADC_HandleTypeDef *hadc)
	{
		HAL_StatusTypeDef status;

		//printf("ADC Start... "); son..
		status = HAL_ADC_Start(hadc);
		//printf("Start Status: %d | ", status); sonn.

		if (HAL_ADC_PollForConversion(hadc, 100) == HAL_OK)
		{
			adc_value = HAL_ADC_GetValue(hadc);
			//printf("ADC Raw: %lu | ", adc_value);

			temperature = Convert_ADC_to_Temperature(adc_value);
			char buffer[20];
			sprintf(buffer, "%.2f\n", temperature);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
			//printf("Temp: %.2f C\r\n", temperature); sonn.
		}
		//else
		//{
			//printf("Conversion Timeout!\r\n"); sonn..
		//}

		HAL_ADC_Stop(hadc);
	}
	#endif

	// =====================================================
	// 🧩 DMA YÖNTEMİ
	// =====================================================
	#ifdef USE_DMA
	void Read_Temperature_DMA(ADC_HandleTypeDef *hadc)
	{
		if (HAL_ADC_Start_DMA(hadc, (uint32_t*)&adc_value, 1) == HAL_OK)
		{
			//printf("DMA started...\r\n");
		}
		else
		{
			//printf("DMA start failed!\r\n");
		}
	}

	// DMA Callback (main.c içinde de olabilir, ama burada da olabilir)
	void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
	{
		if (hadc->Instance == ADC1)
		{
			temperature = Convert_ADC_to_Temperature(adc_value);
			//printf("DMA Callback -> ADC: %lu | Temp: %.2f C\r\n", adc_value, temperature);
		}
	}
	#endif

	// =====================================================
	// 🧩 INTERRUPT YÖNTEMİ
	// =====================================================
	#ifdef USE_INTERRUPT
	void Read_Temperature_IT(ADC_HandleTypeDef *hadc)
	{
		if (HAL_ADC_Start_IT(hadc) == HAL_OK)
		{
			//printf("Interrupt mode started...\r\n");
		}
		else
		{
			//printf("Interrupt start failed!\r\n");
		}
	}

	// Interrupt Callback
	void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
	{
		if (hadc->Instance == ADC1)
		{
			adc_value = HAL_ADC_GetValue(hadc);
			temperature = Convert_ADC_to_Temperature(adc_value);
			//printf("IT Callback -> ADC: %lu | Temp: %.2f C\r\n", adc_value, temperature);
		}
	}
	#endif

	// =====================================================
	// 🧩 ORTAK FONKSİYON (SEÇİLEN YÖNTEME GÖRE ÇALIŞIR)
	// =====================================================
	void Read_Temperature(ADC_HandleTypeDef *hadc)
	{
	#ifdef USE_POLLING
		Read_Temperature_Polling(hadc);
	#elif defined(USE_DMA)
		Read_Temperature_DMA(hadc);
	#elif defined(USE_INTERRUPT)
		Read_Temperature_IT(hadc);
	#endif
	}

	// =====================================================
	// 📐 Ham ADC değerinden sıcaklığı hesaplar
	// =====================================================
	float Convert_ADC_to_Temperature(uint32_t adc_raw)
	{
		if (adc_raw == 0) {
			//printf("ADC RAW = 0! | "); sonn.
			return -999.0f;
		}

		// Kalibrasyon değerlerini oku
		uint16_t ts_cal1 = *TS_CAL1_ADDR;
		uint16_t ts_cal2 = *TS_CAL2_ADDR;

		//printf("CAL1: %u, CAL2: %u | ", ts_cal1, ts_cal2); sonn.

		// Eğer kalibrasyon değerleri okunamadıysa
		if (ts_cal1 == 0 || ts_cal2 == 0) {
			//printf("No CAL Data | "); sonn..

			// Basit voltaj hesaplama
			float voltage = ((float)adc_raw * 3.3f) / 4095.0f;
			//printf("Voltage: %.3fV | ", voltage);  sonn..

			// Alternatif sıcaklık formülü
			float temp = ((voltage - 0.76f) / 0.0025f) + 25.0f;
			return temp;
		}

		// Kalibrasyonlu sıcaklık hesaplama
		float temp = ((110.0f - 30.0f) * ((float)adc_raw - ts_cal1)) / (ts_cal2 - ts_cal1) + 30.0f;
		return temp;
	}
