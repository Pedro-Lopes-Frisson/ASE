#ifndef __WIFIV2__2024_ASE
#define __WIFIV2__2024_ASE
/*
 * SPDX-FileCopyrightText: 2006-2016 ARM Limited
 * SPDX-FileCopyrightText: 2015-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_event.h"
void event_handler(void *arg, esp_event_base_t event_base,
                   int32_t event_id, void *event_data);

void initialise_wifi(void);
int isWifiConnected(void);
void wifi_enterprise_example_task(void *pvParameters) ;
#endif