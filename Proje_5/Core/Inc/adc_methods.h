#ifndef INC_ADC_METHODS_H_
#define INC_ADC_METHODS_H_

#include "main.h"
#include <stdint.h>
#include <stdio.h>

// =====================================================
// 📌 ADC ÇALIŞMA YÖNTEMİ SEÇİMİ
// Sadece bir tanesinin başındaki "//" kaldırılmalıdır
// =====================================================

//#define USE_DMA
//#define USE_INTERRUPT
#define USE_POLLING   // 🔹 Şu anda aktif yöntem bu

// =====================================================
// 📌 Değişken tanımları (senin orijinal değişkenlerin)
// =====================================================
extern volatile uint32_t adc_value;
extern volatile float temperature;

// =====================================================
// 📌 Fonksiyon prototipleri
// =====================================================

// Ortak okuma fonksiyonu (hangi yöntem aktifse o çalışır)
void Read_Temperature(ADC_HandleTypeDef *hadc);

// Ham ADC değerini sıcaklığa dönüştürür
float Convert_ADC_to_Temperature(uint32_t adc_raw);

// =====================================================
// 📌 Yöntem bazlı ek prototipler
// =====================================================
#ifdef USE_POLLING
void Read_Temperature_Polling(ADC_HandleTypeDef *hadc);
#endif

#ifdef USE_DMA
void Read_Temperature_DMA(ADC_HandleTypeDef *hadc);
#endif

#ifdef USE_INTERRUPT
void Read_Temperature_IT(ADC_HandleTypeDef *hadc);
#endif

#endif /* INC_ADC_METHODS_H_ */
