#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "ssd1306.h"


#define tag "SSD1306"

//OLED Display Config
SSD1306_t dev;
int center, top, bottom;
char lineChar[20];

/*
 You have to set this config value with menuconfig
 CONFIG_INTERFACE

 for i2c
 CONFIG_MODEL
 CONFIG_SDA_GPIO
 CONFIG_SCL_GPIO
 CONFIG_RESET_GPIO

 for SPI
 CONFIG_CS_GPIO
 CONFIG_DC_GPIO
 CONFIG_RESET_GPIO
*/

void OLED_init() {
    // CONFIG_I2C_INTERFACE
    #if CONFIG_I2C_INTERFACE
        ESP_LOGI(tag, "INTERFACE is i2c");
        ESP_LOGI(tag, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
        ESP_LOGI(tag, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
        ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
        i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    #endif 
    // CONFIG_SPI_INTERFACE
    #if CONFIG_SPI_INTERFACE
        ESP_LOGI(tag, "INTERFACE is SPI");
        ESP_LOGI(tag, "CONFIG_MOSI_GPIO=%d",CONFIG_MOSI_GPIO);
        ESP_LOGI(tag, "CONFIG_SCLK_GPIO=%d",CONFIG_SCLK_GPIO);
        ESP_LOGI(tag, "CONFIG_CS_GPIO=%d",CONFIG_CS_GPIO);
        ESP_LOGI(tag, "CONFIG_DC_GPIO=%d",CONFIG_DC_GPIO);
        ESP_LOGI(tag, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
        spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
    #endif 

    #if CONFIG_FLIP
	dev._flip = true;
	ESP_LOGW(tag, "Flip upside down");
    #endif
    // CONFIG_SSD1306_128x64
    #if CONFIG_SSD1306_128x64
        ESP_LOGI(tag, "Panel is 128x64");
        ssd1306_init(&dev, 128, 64);
    #endif 
    // CONFIG_SSD1306_128x32
    #if CONFIG_SSD1306_128x32
        ESP_LOGI(tag, "Panel is 128x32");
        ssd1306_init(&dev, 128, 32);
    #endif 
}

//Clear the screen
void OLED_clear(){
    ssd1306_clear_screen(&dev, false);
}

void OLED_display(char time_display1[], char time_display2[]) {
    // Display the formatted text
    ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text_x3(&dev, 0, time_display1, 5, false);  // Display Player 1's time
    ssd1306_display_text_x3(&dev, 4, time_display2, 5, false);  // Display Player 2's time  (The second display start at line 4 because at x3 size each character take 4 line for a display with 8 line)
}