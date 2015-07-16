/*
* config.c
*
* Copyright (c) 2014-2015, Tuan PM <tuanpm at live dot com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "mqtt.h"
#include "config.h"
#include "user_config.h"
#include "debug.h"

SYSCFG sysCfg;
SAVE_FLAG saveFlag;

void ICACHE_FLASH_ATTR
CFG_Save()
{
	 spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
	                   (uint32 *)&saveFlag, sizeof(SAVE_FLAG));

	if (saveFlag.flag == 0) {
		spi_flash_erase_sector(CFG_LOCATION + 1);
		spi_flash_write((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&sysCfg, sizeof(SYSCFG));
		saveFlag.flag = 1;
		spi_flash_erase_sector(CFG_LOCATION + 3);
		spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	} else {
		spi_flash_erase_sector(CFG_LOCATION + 0);
		spi_flash_write((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&sysCfg, sizeof(SYSCFG));
		saveFlag.flag = 0;
		spi_flash_erase_sector(CFG_LOCATION + 3);
		spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	}
}

void ICACHE_FLASH_ATTR
CFG_Load()
{

	INFO("\r\nload ...\r\n");
	spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
				   (uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	if (saveFlag.flag == 0) {
		spi_flash_read((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
					   (uint32 *)&sysCfg, sizeof(SYSCFG));
	} else {
		spi_flash_read((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
					   (uint32 *)&sysCfg, sizeof(SYSCFG));
	}
	if(sysCfg.cfg_holder != CFG_HOLDER){
		os_memset(&sysCfg, 0x00, sizeof sysCfg);


		sysCfg.cfg_holder = CFG_HOLDER;

		os_sprintf((char *)sysCfg.sta_ssid, "%s", STA_SSID);
		os_sprintf((char *)sysCfg.sta_pwd, "%s", STA_PASS);
		sysCfg.sta_type = STA_TYPE;

		os_sprintf((char *)sysCfg.device_id, MQTT_CLIENT_ID, system_get_chip_id());
		os_sprintf((char *)sysCfg.mqtt_host, "%s", MQTT_HOST);
		sysCfg.mqtt_port = MQTT_PORT;
		os_sprintf((char *)sysCfg.mqtt_user, "%s", MQTT_USER);
		os_sprintf((char *)sysCfg.mqtt_pass, "%s", MQTT_PASS);

		sysCfg.security = DEFAULT_SECURITY;	/* default non ssl */

		sysCfg.mqtt_keepalive = MQTT_KEEPALIVE;

		INFO(" default configuration\r\n");

		CFG_Save();
	}

}

void ICACHE_FLASH_ATTR CFG_Update(char essid[128], char passwd[128],
																	char host[64], uint32_t port,
																	char mqtt_user[32], char mqtt_pass[32], char mqtt_topic[32])
{
	//Save configuration to the memory
	saveFlag.flag  = 1;
	os_sprintf((char *)sysCfg.sta_ssid, "%s", essid);
	os_sprintf((char *)sysCfg.sta_pwd, "%s", passwd);
	os_sprintf((char *)sysCfg.mqtt_host, "%s", host);

	sysCfg.mqtt_port = port;

	//save MQTT user & password
	os_sprintf((char *)sysCfg.mqtt_user, "%s", mqtt_user);
	os_sprintf((char *)sysCfg.mqtt_pass, "%s", mqtt_pass);

	//Save MQTT topic
	os_sprintf((char *)sysCfg.mqtt_topic, "%s", mqtt_topic);

	CFG_Save();
}
