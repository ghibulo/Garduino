#ifndef GARDPERIF_H
#define GARDPERIF_H
#include <Arduino.h>
#include <OneWire.h>
#include <DS1302.h>

#define nDEBUG

#ifdef __cplusplus
extern "C" {
#endif


 






//Control button pad

// left, right, up, down, select, difference
const int butConst[2][6] = { {325, 494, 529, 0, 437, 12}, {'l', 'r', 'u', 'd', 's', '-'} };

const int buttonPin = A0;
const int temperatPin = 7;
const int relayImpulsPin = A1;


//time stamp of the last pressing a button
extern unsigned long lastPress;
// when to stop focus mode
//extern unsigned long mxDurAfterPress;

//time stamp of the last relay impuls
extern unsigned long lastRelayImpuls;

extern OneWire dst;

// Init the DS1302
extern DS1302 rtc;    


void initPerifs();

/*
 * input: number of pin
 * return: {'l', 'r', 'u', 'd', 's', '-'}
 */
char getBut();
boolean noPressBut();

void prepareTemperature();
float getTemperature();

void getTimeRTC(int &h, int &m, int &di, int &ni);
char *form2digit(int num);

void impulseRelay();


#ifdef __cplusplus
}
#endif



#endif /* GARDPERIF_H*/


