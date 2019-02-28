
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
    power_config.max_freq_mhz = 80;
	power_config.min_freq_mhz = 80;
	power_config.light_sleep_enable = false;
    esp_pm_configure(&power_config);

    gpio_config_t pCFG = {};
    pCFG.pin_bit_mask = 0b10101;
    pCFG.intr_type = GPIO_INTR_DISABLE;
    pCFG.mode = GPIO_MODE_OUTPUT_OD;

    gpio_config(&pCFG);

	XaI2C::MasterAction::init(GPIO_NUM_12, GPIO_NUM_13);

    const char *tString = "Helu!";
	 uint32_t rCommand = 0x593412;

	 uint16_t measVoltage;

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
    		printf("I2C Error: %s\n", esp_err_to_name(ret));

    		gpio_set_level(GPIO_NUM_4, true);

    		if(ret == ESP_ERR_TIMEOUT)
    			gpio_set_level(GPIO_NUM_0, true);
    	}
    	else {
    		printf("Measured voltage is: %d\n", measVoltage);
    	}

    	vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
