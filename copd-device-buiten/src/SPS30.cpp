/*
 * SPS30.cpp
 *
 * Created: 6/9/2019 3:15:06 PM
 *  Author: joey
 */ 

#include "SPS30.h"

extern struct fijnstof f1;

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

int SPS30Read()
{
	struct sps30_measurement m;
	char serial[SPS_MAX_SERIAL_LEN];
	u16 data_ready;
	s16 ret;
	
	ret = sps30_read_data_ready(&data_ready);
	if (ret < 0) {
		Serial.print("error reading data-ready flag: ");
		Serial.println(ret);
	} else if (!data_ready)	return -1;

	ret = sps30_read_measurement(&m);
	if (ret < 0) {
		Serial.print("error reading measurement\n");
		return -1;
		} else {
		f1.PM1p0 = m.mc_1p0;
		f1.PM2p5 = m.mc_2p5;
		f1.PM4p0 = m.mc_4p0;
		f1.PM10p0 = m.mc_10p0;
		return 0;
	}
}