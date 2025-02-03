#include "Arduino.h"
#include "TFT_eSPI.h"
#include <esp_adc_cal.h>
#include "pin_config.h"

#define LCD_MODULE_CMD_1
#define BATT_MIN_VOLTAGE 3300
#define BATT_MAX_VOLTAGE 4200

TFT_eSPI tft = TFT_eSPI();
unsigned long targetTime = 0;

#if defined(LCD_MODULE_CMD_1)
typedef struct {
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;

lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},
};
#endif

int calculateBatteryPercentage(uint32_t voltage) {
    if (voltage >= BATT_MAX_VOLTAGE) return 100;
    if (voltage <= BATT_MIN_VOLTAGE) return 0;
    
    return (voltage - BATT_MIN_VOLTAGE) * 100 / (BATT_MAX_VOLTAGE - BATT_MIN_VOLTAGE);
}

void setup() {
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    Serial.begin(115200);

    tft.begin();
    tft.setRotation(1);
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED, TFT_BLACK);

#if defined(LCD_MODULE_CMD_1)
    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++) {
        tft.writecommand(lcd_st7789v[i].cmd);
        for (int j = 0; j < lcd_st7789v[i].len & 0x7f; j++) {
            tft.writedata(lcd_st7789v[i].data[j]);
        }
        if (lcd_st7789v[i].len & 0x80) {
            delay(120);
        }
    }
#endif

    // Draw a red rounded edge rectangular outline
    tft.drawRoundRect(0, 0, TFT_HEIGHT, TFT_WIDTH, 10, TFT_RED);

    // Turn on backlight
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);
}

void loop() {
    if (millis() > targetTime) {
        esp_adc_cal_characteristics_t adc_chars;
        
        // Get battery voltage
        esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
        uint32_t raw = analogRead(PIN_BAT_VOLT);
        uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;
        
        // Calculate battery percentage
        int batteryPercentage = calculateBatteryPercentage(voltage);

        // Get temperature
        float temp = temperatureRead();

        // Clear previous readings only
        tft.fillRect(170, 20, TFT_HEIGHT-180, TFT_WIDTH-40, TFT_BLACK);

        // Display temperature
        tft.setCursor(20, 30);
        tft.print("Temperature:");
        tft.setCursor(175, 30);
        tft.print(String(temp, 1) + " C");

        // Display battery voltage
        tft.setCursor(20, 55);
        tft.print("Voltage:");
        tft.setCursor(175, 55);
        tft.print(String(voltage) + " mV");

        // Display battery percentage
        tft.setCursor(20, 75);
        tft.print("Charge:");
        tft.setCursor(175, 75);
        tft.print(String(batteryPercentage) + "%");

        targetTime = millis() + 1000;
    }
    delay(20);
}
