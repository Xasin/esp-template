
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_pm.h"
#include "esp_timer.h"
#include "esp32/pm.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"

#include "driver/ledc.h"
#include "driver/touch_pad.h"

#include <array>
#include <cstring>

#include "MasterAction.h"

#include "SSD1327.h"
#include "LittleConsole.h"

auto screen = Peripheral::OLED::SSD1327();

auto consoleBox = Peripheral::OLED::DrawBox(128, 64, &screen);
auto sConsole = Peripheral::OLED::LittleConsole(consoleBox);

auto testBox = Peripheral::OLED::DrawBox(64, 12, &screen);

float boxFillPercent = 0;
void redrawTestBox() {
	testBox.draw_line(0, 1, 10, 1, 2);
	testBox.draw_line(63, 1, 10, 1, 2);
	testBox.draw_line(1, 0, 62, 0, 2);
	testBox.draw_line(1, 11, 62, 0, 2);

	testBox.draw_box(53, 0, 10, 12, 2, 2);
	testBox.write_char(55, 3, 'V', 0);

	if(boxFillPercent >= 0.02)
		testBox.draw_box(1, 1, 52*boxFillPercent, 10, 1, 1);

	char voltBuffer[10] = {};
	sprintf(voltBuffer, "%02.2f", boxFillPercent*100);
	testBox.write_string(1 + 52/2 - 3*strlen(voltBuffer), 3, voltBuffer);
}

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

extern "C" void app_main(void)
{
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

    esp_timer_init();

    esp_pm_config_esp32_t power_config = {};
    power_config.max_freq_mhz = 240;
	power_config.min_freq_mhz = 80;
	power_config.light_sleep_enable = false;
    esp_pm_configure(&power_config);

    gpio_config_t pCFG = {};
    pCFG.pin_bit_mask = 0b10101;
    pCFG.intr_type = GPIO_INTR_DISABLE;
    pCFG.mode = GPIO_MODE_OUTPUT_OD;

    gpio_config(&pCFG);

	XaI2C::MasterAction::init(GPIO_NUM_14, GPIO_NUM_13);

	screen.initialize();

	testBox.offsetY = 70;
	testBox.offsetX = 32;
	testBox.onRedraw = redrawTestBox;

    const char *tString = "Helu!";
	 uint32_t rCommand = 0x593412;

	 uint16_t measVoltage = 0;

	 while (true) {
	 	vTaskDelay(50);
	 	boxFillPercent = (measVoltage%330)/330.0;
		sConsole.printf("Test no. %d\n", measVoltage++);
	 }

    while (true) {
    	auto tCMD = XaI2C::MasterAction(112);

    	tCMD.write(0x02, &rCommand, 3);
		uint32_t dBuff;
		tCMD.read(&dBuff, 4);
    	auto ret = tCMD.execute();

		measVoltage = *reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(&dBuff) + 1);

    	gpio_set_level(GPIO_NUM_0, false);
    	gpio_set_level(GPIO_NUM_2, false);
    	gpio_set_level(GPIO_NUM_4, false);

    	if(ret != ESP_OK) {
    		sConsole.printf("I2C Error: %s\n", esp_err_to_name(ret));

    		gpio_set_level(GPIO_NUM_4, true);

    		if(ret == ESP_ERR_TIMEOUT)
    			gpio_set_level(GPIO_NUM_0, true);
    	}
    	else {
    		sConsole.printf("Measured voltage is: %d\n", measVoltage);
    	}

    	vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
