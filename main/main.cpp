
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


	XaI2C::MasterAction::init(GPIO_NUM_12, GPIO_NUM_13);

    const char *tString = "Helu!";
	std::array<uint8_t, 20> tData;
	memcpy(tData.data(), tString, strlen(tString));

    while (true) {
    	auto tCMD = XaI2C::MasterAction(112);

    	tData[0]++;
    	tData[1]++;

    	tCMD.write(1, tData.data(), 6);
    	tCMD.execute();

    	vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

