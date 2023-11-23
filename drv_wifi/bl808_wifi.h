
#ifndef __BL808_WIFI_H__
#define __BL808_WIFI_H__

#include <wifi_mgmr.h>
#include <wifi_mgmr_api.h>
#include <wifi_mgmr_ext.h>
#include <bl_defs.h>
#include "bl_main.h"

#include <bl_wifi.h>
#include <bl60x_fw_api.h>
#include <bl_os_private.h>

// yloop
#include <aos/kernel.h>
#include <aos/yloop.h>

#include <event_device.h>

void bl808_wifi_init(void *event_callback);

#endif /* __BL808_WIFI_H__ */