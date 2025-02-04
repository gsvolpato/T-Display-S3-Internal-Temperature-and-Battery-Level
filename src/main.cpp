#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <esp_adc_cal.h>
#include "pin_config.h"

#define BATT_MIN_VOLTAGE 3300
#define BATT_MAX_VOLTAGE 4200
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 170
#define BUFFER_SIZE (SCREEN_WIDTH * 10)  // Reduced buffer size

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[BUFFER_SIZE] __attribute__((aligned(4)));
static lv_color_t buf2[BUFFER_SIZE] __attribute__((aligned(4)));
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

TFT_eSPI tft = TFT_eSPI();

// LVGL objects
static lv_obj_t *temp_label;
static lv_obj_t *voltage_label;
static lv_obj_t *charge_label;
static lv_obj_t *uptime_label;
static lv_obj_t *battery_bar;

// Timer for updates
static lv_timer_t *update_timer;

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

int calculateBatteryPercentage(uint32_t voltage) {
    if (voltage >= BATT_MAX_VOLTAGE) return 100;
    if (voltage <= BATT_MIN_VOLTAGE) return 0;
    return (voltage - BATT_MIN_VOLTAGE) * 100 / (BATT_MAX_VOLTAGE - BATT_MIN_VOLTAGE);
}

void update_values(lv_timer_t *timer) {
    // Get battery voltage
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    uint32_t raw = analogRead(PIN_BAT_VOLT);
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;
    
    // Calculate battery percentage
    int batteryPercentage = calculateBatteryPercentage(voltage);

    // Get temperature
    float temp = temperatureRead();

    // Format strings
    char temp_str[32], voltage_str[32], charge_str[32], uptime_str[32];
    snprintf(temp_str, sizeof(temp_str), "%.1f °C", temp);
    snprintf(voltage_str, sizeof(voltage_str), "%d mV", voltage);
    snprintf(charge_str, sizeof(charge_str), "%d%%", batteryPercentage);
    
    // Calculate uptime
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = (seconds / 60) % 60;
    unsigned long hours = (seconds / 3600);
    seconds = seconds % 60;
    snprintf(uptime_str, sizeof(uptime_str), "%02lu:%02lu:%02lu", hours, minutes, seconds);

    // Update LVGL labels
    lv_label_set_text(temp_label, temp_str);
    lv_label_set_text(voltage_label, voltage_str);
    lv_label_set_text(charge_label, charge_str);
    lv_label_set_text(uptime_label, uptime_str);
    
    // Update battery bar
    lv_bar_set_value(battery_bar, batteryPercentage, LV_ANIM_ON);
}

void setup() {
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    Serial.begin(115200);

    // Initialize display
    tft.begin();
    tft.setRotation(1);
    
    // Initialize LVGL
    lv_init();
    
    // Initialize buffer with both buffers for double buffering
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, BUFFER_SIZE);
    
    // Initialize display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Create UI elements
    lv_obj_t *main_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_container, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(main_container, lv_color_black(), 0);
    lv_obj_set_style_border_color(main_container, lv_color_make(255, 0, 0), 0);
    lv_obj_set_style_border_width(main_container, 2, 0);
    lv_obj_set_style_radius(main_container, 10, 0);
    lv_obj_clear_flag(main_container, LV_OBJ_FLAG_SCROLLABLE);

    // Create labels with titles
    lv_obj_t *title_temp = lv_label_create(main_container);
    lv_label_set_text(title_temp, "Temperature:");
    lv_obj_align(title_temp, LV_ALIGN_TOP_LEFT, 10, 20);
    
    temp_label = lv_label_create(main_container);
    lv_obj_align_to(temp_label, title_temp, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *title_voltage = lv_label_create(main_container);
    lv_label_set_text(title_voltage, "Voltage:");
    lv_obj_align(title_voltage, LV_ALIGN_TOP_LEFT, 10, 50);
    
    voltage_label = lv_label_create(main_container);
    lv_obj_align_to(voltage_label, title_voltage, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *title_charge = lv_label_create(main_container);
    lv_label_set_text(title_charge, "Charge:");
    lv_obj_align(title_charge, LV_ALIGN_TOP_LEFT, 10, 80);
    
    charge_label = lv_label_create(main_container);
    lv_obj_align_to(charge_label, title_charge, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t *title_uptime = lv_label_create(main_container);
    lv_label_set_text(title_uptime, "Uptime:");
    lv_obj_align(title_uptime, LV_ALIGN_TOP_LEFT, 10, 110);
    
    uptime_label = lv_label_create(main_container);
    lv_obj_align_to(uptime_label, title_uptime, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // Create battery bar
    battery_bar = lv_bar_create(main_container);
    lv_obj_set_size(battery_bar, 200, 15);
    lv_obj_align(battery_bar, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_bar_set_range(battery_bar, 0, 100);
    
    // Set all text colors to red
    lv_obj_set_style_text_color(main_container, lv_color_make(255, 0, 0), 0);
    
    // Create update timer
    update_timer = lv_timer_create(update_values, 1000, NULL);

    // Turn on backlight
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);
}

void loop() {
    lv_timer_handler();
    delay(5);
}
