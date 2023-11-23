/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022/12/25     flyingcys    first version
 */

#undef ARCH_RISCV
#include <rtthread.h>
#include <stdio.h>

#include "bl808_wifi.h"

// static wifi_interface_t wifi_interface;

#define WIFI_AP_PSM_INFO_SSID "conf_ap_ssid"
#define WIFI_AP_PSM_INFO_PASSWORD "conf_ap_psk"
#define WIFI_AP_PSM_INFO_PMK "conf_ap_pmk"
#define WIFI_AP_PSM_INFO_BSSID "conf_ap_bssid"
#define WIFI_AP_PSM_INFO_CHANNEL "conf_ap_channel"
#define WIFI_AP_PSM_INFO_IP "conf_ap_ip"
#define WIFI_AP_PSM_INFO_MASK "conf_ap_mask"
#define WIFI_AP_PSM_INFO_GW "conf_ap_gw"
#define WIFI_AP_PSM_INFO_DNS1 "conf_ap_dns1"
#define WIFI_AP_PSM_INFO_DNS2 "conf_ap_dns2"
#define WIFI_AP_PSM_INFO_IP_LEASE_TIME "conf_ap_ip_lease_time"
#define WIFI_AP_PSM_INFO_GW_MAC "conf_ap_gw_mac"

void start_aos_loop(void);

#define TASK_AOS_TASKNAME ((char *)"aos_loop")
#define TASK_AOS_STACKSIZE (2048)
#define TASK_AOS_PRIORITY (20)

#define TASK_WIFIMAIN_TASKNAME ((char *)"wifi_main")
#define TASK_WIFIMAIN_STACKSIZE (4096)
#define TASK_WIFIMAIN_PRIORITY (15)

void bl808_wifi_init(void *event_callback)
{
    static uint8_t stack_wifi_init = 0;

    if (1 == stack_wifi_init) {
        return;
    }
    stack_wifi_init = 1;

    start_aos_loop();

    bl_sec_init();
    phy_powroffset_set((int8_t[4]){0x0, 0x0, 0x0, 0x0});
    bl_tpc_update_power_rate_11b((int8_t[4]){0x14, 0x14, 0x14, 0x12});
    bl_tpc_update_power_rate_11g((int8_t[8]){0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0xe, 0xe});
    bl_tpc_update_power_rate_11n((int8_t[8]){0x12, 0x12, 0x12, 0x12, 0x12, 0x10, 0xe, 0xe});

    bl_pm_init();

    bl_os_task_create(TASK_WIFIMAIN_TASKNAME, wifi_main, TASK_WIFIMAIN_STACKSIZE, NULL, TASK_WIFIMAIN_PRIORITY, NULL);
    bl_os_event_register(EV_WIFI, event_callback, NULL);

    rt_thread_delay(1000);

    // bl_os_event_notify(CODE_WIFI_ON_INIT_DONE, 0);
    static wifi_conf_t conf = {
        .country_code = "CN",
    };
    wifi_mgmr_start_background(&conf);
}

static void aos_loop_proc(void *pvParameters)
{
    // int fd_console;

    vfs_init();
    vfs_device_init();

    aos_loop_init();

    rt_kprintf("start aos loop... \r\n");

    aos_loop_run();

    bl_os_puts("------------------------------------------\r\n");
    bl_os_puts("+++++++++Critical Exit From Loop++++++++++\r\n");
    bl_os_puts("******************************************\r\n");
}

void start_aos_loop(void)
{
    rt_thread_t tid = rt_thread_create(TASK_AOS_TASKNAME, aos_loop_proc, RT_NULL,
                           TASK_AOS_STACKSIZE, TASK_AOS_PRIORITY, 20);
    if(RT_NULL != tid)  
        rt_thread_startup(tid);
    rt_kprintf("aos_loop_tid:%p\r\n", tid);
}
