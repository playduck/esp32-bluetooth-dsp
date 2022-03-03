#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/ringbuf.h"
#include "freertos/task.h"
#include "freertos/xtensa_api.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_bt.h"

#include "driver/i2s.h"
#include "driver/gpio.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_bt_device.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"

#include "bt_app_av.h"
#include "bt_app_core.h"

#include "pipeline.h"
