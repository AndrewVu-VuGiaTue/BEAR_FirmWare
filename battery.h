#ifndef BATTERY_H
#define BATTERY_H

#include "driver/adc.h"
#include "esp_adc_cal.h"

#define BATTERY_ADC_PIN         ADC1_CHANNEL_6  // GPIO34
#define BATTERY_NO_OF_SAMPLES   64
#define BATTERY_DEFAULT_VREF    1100  // mV

static esp_adc_cal_characteristics_t adc_chars;

// Khởi tạo ADC
void battery_begin() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(BATTERY_ADC_PIN, ADC_ATTEN_DB_11);

    esp_adc_cal_characterize(
        ADC_UNIT_1,
        ADC_ATTEN_DB_11,
        ADC_WIDTH_BIT_12,
        BATTERY_DEFAULT_VREF,
        &adc_chars
    );
}

// Trả về điện áp đo được (V)
float battery_get_voltage() {
    uint32_t adc_reading = 0;
    for (int i = 0; i < BATTERY_NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw(BATTERY_ADC_PIN);
    }
    adc_reading /= BATTERY_NO_OF_SAMPLES;

    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
    return voltage / 1000.0;
}

// Trả về phần trăm pin (0–100%)
uint8_t battery_get_level() {
    float voltage = 2 * battery_get_voltage();  // nhân đôi nếu dùng chia áp

    if (voltage > 4.2) voltage = 4.2;
    if (voltage < 3.2) voltage = 3.2;

    return (uint8_t)(((voltage - 3.2) / (4.2 - 3.2)) * 100.0);
}

#endif
