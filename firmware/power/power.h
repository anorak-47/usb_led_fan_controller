#pragma once

void powermeter_init(void);
void powermeter_scan_for_devices(void);

uint32_t powermeter_getBusVoltage_mV(uint8_t channel);
uint32_t powermeter_getCurrent_mA(uint8_t channel);
uint32_t powermeter_getPower_mW(uint8_t channel);
