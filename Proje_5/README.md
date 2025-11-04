# STM32F407  Dahili Sıcaklık Sensörü Projesi
Bu proje , **STM32F407 discovery kartı** üzerindeki **dahili sıcaklık sensöründen** ADC verilerini okuyup **SWV (Serial wire Viewer)** üzerinden gerçek zamanlı olarak  sıcaklık değerlerini görüntüler.

Proje, **HAL kütüphanesi** kullanılarak geliştirilmiş ve üç farklı ADC okuma yöntemi destekler:

- Polling
- DMA
- İnterrupt

![C](https://img.shields.io/badge/Language-C-blue)
![STM32CubeIDE](https://img.shields.io/badge/IDE-STM32CubeIDE-green)
![Board](https://img.shields.io/badge/Board-STM32F407-orange)

<img src="./demo.png" alt="STM32 SWV ITM Data Console" width="500">




---
## İçindekiler
- [Hakkında](#hakkında)
- [Kullanılan Teknolojiler](#kullanılan-teknolojiler)
- [Kurulum](#kurulum)
- [Kullanım](#kullanım)
- [Geliştirici](#geliştirici)
- [Kodlar Ve Açıklamaları](#kodlar-ve-açıklamaları)
- [SWV ITM Data Console Çıktısı](#swv-ıtm-data-console-çıktısı)


---
## Hakkında
Proje, STM32'nin dahili sensörünü kullanarak sıcaklık verilerini toplar ve bu verileri **SWV ITM Data Console** aracılığıyla görüntüler.

Kullanıcı, kodda tanımlanan **makrolar** sayesinde hangi ADC yönteminin (Polling, DMA veya Interrupt) kullanılacağını seçebilir.

---
## Kullanılan Teknolojiler
- STM32CubeMX
- STM32CubeIDE
- HAL
- ARM Cortex-M4 (STM32F407VG)
- SWV
- C programlama dili
---
## Kurulum
1. STM32CubeMX ile aç ve gerekli pin, ADC,DMA ve Timer ayarlarını yap.
2. Projeyi CubeIDE ile aç.   
3. `Project -> Properties -> C/C++ Build -> Settings` menüsünden gerekli `USE_...` makrosunu aktif et:  
   - `USE_POLLING`
   - `USE_DMA`
   - `USE_INTERRUPT`
4. Debug modda derle ve çalıştır.  
5. **SWV ITM Data Console** penceresini açarak canlı sıcaklık değerlerini gözlemle.

---
## Kullanım

Proje çalıştırıldığında:
1. Seçilen yönteme göre ADC ölçümleri başlar.  
2. Her saniye SWV konsoluna sıcaklık değeri yazdırılır.  
---
## ✨ Geliştirici

👩‍💻 **Nursel Aktürk**  
📧 [nurselakturk52@gmail.com]  
🌐 [GitHub Profilim](https://github.com/nurselakturk)

---
## Kodlar Ve Açıklamaları
### 1️⃣ **adc_methods.h**

```c
//#define USE_DMA
#define USE_Interrupt
//#define USE_POLLING
```

**Açıklama:**

Bu üç satırdan sadece biri aktif olur.
Kullanmak istediğin ADC yöntemine göre (DMA,Interrupt veya Polling) biri seçilir ve proje derlenirken o yöntemin fonksiyonları kullanılır.

```c
extern volatile uint32_t adc_value;
extern volatile float temperature;
```
**Açıklama:**

Tüm proje boyunca erişilebilir değerler.
- `adc_value`: ADC'den okunan ham sayısal değer.
- `temperature`: Hesaplanan sıcaklık değeri


### 2️⃣ **adc_methods.c**
🔹 Polling Yöntemi

```c
void Read_Temperature_Polling(ADC_HandleTypeDef *hadc)
{
    HAL_StatusTypeDef status;
    printf("ADC Start... ");
    status = HAL_ADC_Start(hadc);
    printf("Start Status: %d |", status);

    if(HAL_ADC_PollForConversion(hadc, 1000) == HAL_OK)
    {
        adc_value = HAL_ADC_GetValue(hadc);
        temperature = Convert_ADC_to_Temperature(adc_value);
        printf("Temp: %.2f C\r\n",  temperature);

    }
    else 
    {
        printf("Conversion Timeout!\r\n");
    }

    HAL_ADC_Stop(hadc);

}
```
**Açıklama:**

polling ile ADC başlatılır.ve dönüş tamamlanana kadar CPU bekler.Sonra ADC değeri alınır ve sıcaklığa çevrilir.

---

🔹 DMA Yöntemi

```c
void Read_Temperature_DMA(ADC_HandleTypeDef *hadc)
{
    if (HAL_ADC_Start_DMA(hadc, (uint32_t*)&adc_value, 1) == HAL_OK)
    {
        printf("DMA started...\r\n");
    }
    else
    {
        printf("DMA start failed!\r\n");
    }
}
```
**Açıklama:**

DMA,ADC'nin veriyi RAM'e doğrudan aktarmasını sağlar.
CPU beklemez, transfer tamamlanınca `HAL_ADC_ConvCpltCallback` fonksiyonu tetiklenir.

🔹 DMA Callback
```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        temperature = Convert_ADC_to_Temperature(adc_value);
        printf("DMA Callback ->  ADC: %lu | Temp: %.2f C\r\n", adc_value, temperature);
    }
}
```
**Açıklama:**

DMA transferi bittiğinde çağrılır. `adc_value`'dan sıcaklık hesaplanır ve SWV'ye yazdırılır.


---

🔹 Interrupt Yöntemi
```c
void Read_Temperature_IT(ADC_HandleTypeDef *hadc)
{
    if(HAL_ADC_Start_IT(hadc) ==  HAL_OK)
    {
        printf("Interrupt mode started...\r\n");

    }
    else
    {
        printf("Interrupt start failed!\r\n");
    }
}
```
**Açıklama:**

Kesme modunda ADC başlatılır.Dönüş tamamlamdığında otomatik olarak kesme (interrupt) oluşur. 

🔹 Interrupt Callback
```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        adc_value = HAL_ADC_GetValue(hadc);
        temperature = Convert_ADC_to_Temperature(adc_value);
        printf("IT Callback -> ADC: %lu | Temp: %.2f C\r\n", adc_value, temperature);

    }
}
```
**Açıklama:**

ADC dönüşü bittiğinde kesme tetiklenir ve bu fonksiyon çalışır.
veri alınır, sıcaklığaçevrilir ve SWV konsoluna yazdırılır

---
🔹 Ortak Fonksiyon

```c
void Read_Temperature(ADC_HandleTypeDef *hadc)
{
#ifdef USE_POLLING
    Read_Temperature_Polling(hadc);
#elif defined(USE_DMA)
    Read_Temperature_DMA(hadc);
#elif defined (USE_INTERRUPT)
    Read_Temperature_IT(hadc);
#endif
}
```
**Açıklama:**

Seçilen yönteme göre ilgili fonksiyonu çağırır.


---
🔹 Sıcaklık Dönüşümü

```c
 float Convert_ADC_to_Temperature(uint32_t adc_raw)
 {
    uint16_t ts_cal1 = *TS_CAL1_ADDR;
    uint16_t ts_cal2 = *TS_CAL2_ADDR;
    float temp = ((110.0f - 30.0f) * ((float)adc_raw - ts_cal1)) / (ts_cal2 - ts_cal1) + 30.0f;
    return temp;
 }
```
**Açıklama:**

STM32'nin üretim sırasında belleğe yazılmış kalibrasyon değerleri kullanılarak ADC ham değeri sıcklığa çevrilir.

---

### 3️⃣ **main.c**

🔹 Başlatma Mesajları
```c
printf("🚀 STM32F4 Sıcaklık Sensörü Başlatılıyor...\r\n");
printf("🔧 System Clock: %lu Hz\r\n", HAL_RCC_GetSysClockFreq());
printf("🔧 ADC Clock: %lu Hz\r\n", HAL_RCC_GetPCLK2Freq() / 8);
```
**Açıklama**

SWV konsoluna sistem ve ADC saat frekansalrını yazar.


---
🔹 İlk Ölçüm ve Döngü

```c
printf("\r\n--- İLK TEST ÖLÇÜMÜ ---\r\n");
Read_Temperature(&hadc1);
printf("\r\n--- NORMAL ÖLÇÜMLER BAŞLIYOR ---\r\n");
```
**Açıklama**

proje başlarken bir test ölçümü yapılır.
sonrasında döngüde 2 saniyede bir yeni ölçüm alınır

---
🔹 SWV Konsoluna Yazdırma Fonksiyonu

```c
int _write(int file, char *ptr, int len)
{
    for (int i = 0 ; i< len ; i++ )
    {
        ITM_SendChar(*ptr++);//swv ITM kanalınna gönder

    }
    return len;
}
```
**Açıklama**

Bu özel `_write` fomksiyonu `printf()` çıktısını **SWV ITM data console**'a yönlendirir. Yani debug pin üzerinden PC'ye aktarılır.

----
## 📊 **SWV ITM Data Console Çıktısı**



Aşağıda, STM32F407VG’nin dahili sıcaklık sensöründen alınan verilerin  
**SWV ITM Data Console** üzerinden canlı olarak izlendiği bir örnek görülmektedir.

 ![SWV ITM Data Console Çıktısı](./swv_console_output.png)
