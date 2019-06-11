/*
 * SPS30.cpp
 *
 * Created: 6/9/2019 3:15:06 PM
 *  Author: joey
 */ 

#include "SPS30.h"

void SPS30Init()
{
	s16 ret;
	u8 auto_clean_days = 4;
	u32 auto_clean;
	
	while (sps30_probe() != 0) {
		Serial.print("SPS sensor probing failed\n");
		delay(500);
	}

	ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
	if (ret) {
		Serial.print("error setting the auto-clean interval: ");
		Serial.println(ret);
	}

	ret = sps30_start_measurement();
	if (ret < 0) {
		Serial.print("error starting measurement\n");
	}

}

String SPS30Read()
{
	struct sps30_measurement m;
	char serial[SPS_MAX_SERIAL_LEN];
	u16 data_ready;
	s16 ret;
	
	do {
		ret = sps30_read_data_ready(&data_ready);
		if (ret < 0) {
			Serial.print("error reading data-ready flag: ");
			Serial.println(ret);
		} else if (!data_ready)
		Serial.print("data not ready, no new measurement available\n");
		else
		break;
		delay(100); /* retry in 100ms */
	} while (1);

	ret = sps30_read_measurement(&m);
	if (ret < 0) {
		Serial.print("error reading measurement\n");
		return String("ERROR\n");
		} else {
		return String("PM  1.0: " + String(m.mc_1p0) + ", PM  2.5: " + String(m.mc_2p5) + ", PM 4.0: " + String(m.mc_4p0) + ", PM 10.0: " + String(m.mc_10p0) + "\n");
	}
}