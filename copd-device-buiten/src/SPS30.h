/*
 * SPS30.h
 *
 * Created: 6/9/2019 3:15:18 PM
 *  Author: joey
 */ 


#ifndef SPS30_H_
#define SPS30_H_

#include <Arduino.h>
#include <sps30.h>

struct fijnstof
{
	double PM1p0;
	double PM2p5;
	double PM4p0;
	double PM10p0;
};

void SPS30Init();
int SPS30Read();


#endif /* SPS30_H_ */