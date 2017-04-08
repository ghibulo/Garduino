/*
Required library - http://www.rinkydinkelectronics.com/library.php?id=5
Blog Post - http://overskill.alexshu.com/ds1302-real-time-clock-w-arduino/
*/
 

#include "gardperif.h"
#include "lcdobject.h"



//max-min temperatures today
float todayMaxTemp, todayMinTemp;

//instantaneous temperature
float temperNow;

//last recorded date
int lastRecNight, lastRecDay;

//main loop - delay and counting
const int deltime = 200;
int loopcounting = 0;

//is morning/evening watering on?
bool morningWateringON = false;
bool eveningWateringON = false;

//focused object
lcdObject *fObj;
lcdObject presentTemperature(tpresentTemperature,0,0,'f');
lcdObject maxDTemperature(tmaxDTemperature,7,0,'f');
lcdObject maxRTemperature(tmaxRTemperature,14,0,'f');
lcdObject minNTemperature(tminNTemperature,7,1,'f');
lcdObject minRTemperature(tminRTemperature,14,1,'f');
lcdObject presentTime(tpresentTime,21,0,'t');
lcdObject morningTime(tmorningTime,28,0,'t');
lcdObject morningDuration(tmorningDuration,35,0,'d');
lcdObject eveningTime(teveningTime,28,1,'t');
lcdObject eveningDuration(teveningDuration,35,1,'d');

//number of objects on the LCD
const int nlcdobj = 10;
lcdObject* arrayobj[nlcdobj];

void showAllInfo() {
    lcdObject::HideCursor();
    for (int i=0;i<nlcdobj;i++) {
      arrayobj[i]->showData();
    }
    lcdObject::ShowCursor();
}




void setup()
{
    // Setup Serial connection
    Serial.begin(9600);
    
    lcdObject::Init();
    initPerifs();
    
    //prepare Scratchpad for temperature reading
    prepareTemperature();delay(800);

    arrayobj[0]= &presentTemperature;
    arrayobj[1]= &maxDTemperature;
    arrayobj[2]= &maxRTemperature;
    arrayobj[3]= &minNTemperature;
    arrayobj[4]= &minRTemperature;
    arrayobj[5]= fObj = &presentTime; //focused from start
    arrayobj[6]= &morningTime;
    arrayobj[7]= &morningDuration;
    arrayobj[8]= &eveningTime;
    arrayobj[9]= &eveningDuration;
    fObj->setFocus();
    
    //position for focus selecting (left, right, up, down)
    presentTemperature.setNeighb(NULL, NULL, NULL, NULL);
    maxDTemperature.setNeighb(NULL, NULL, NULL, NULL);
    maxRTemperature.setNeighb(NULL, &presentTime, NULL, NULL);
    minNTemperature.setNeighb(NULL, NULL, NULL, NULL);
    minRTemperature.setNeighb(NULL, NULL, &maxRTemperature, NULL);
    presentTime.setNeighb(&maxRTemperature, &morningTime, NULL, NULL);
    morningTime.setNeighb(&presentTime, &morningDuration, NULL, &eveningTime);
    morningDuration.setNeighb(&morningTime, NULL, NULL, &eveningDuration);
    eveningTime.setNeighb(NULL, &eveningDuration, &morningTime, NULL);
    eveningDuration.setNeighb(&eveningTime, NULL, &morningDuration, NULL);
    
    
    temperNow = getTemperature();
    prepareTemperature();
    
    presentTemperature.setFl(temperNow);
    maxRTemperature.setFl(temperNow);
    minRTemperature.setFl(temperNow);
    todayMaxTemp = todayMinTemp = temperNow;

    int h,m,di,ni;
    getTimeRTC(h,m,di,ni);
    lastRecNight=ni;
    lastRecDay=di;
    
    presentTime.setTime(h,m);


    //no yesterday, no tomorow...
    morningTime.setTime(infval,0);
    eveningTime.setTime(infval,0);
    maxDTemperature.setFl(infval);
    minNTemperature.setFl(infval); 

    showAllInfo();


}



 
void loop()
{
    if ( loopcounting++ *deltime>1000) {
      loopcounting = 0;
      temperNow = getTemperature();
      prepareTemperature();
      //Serial.println(morningWateringON);
      //Serial.println(eveningWateringON);
      //Serial.println("---");
      
    }
    
    int h,m,di,ni;
    getTimeRTC(h,m,di, ni);
    
    presentTemperature.setFl(temperNow);

    if ((fObj->me == tmaxRTemperature)&&(fObj->state == 'r')) {
      //reset temperature values 
      maxRTemperature.setFl(temperNow);
      minRTemperature.setFl(temperNow);
      maxRTemperature.setNormal(); //no edit but normal again
    } else {
      //record max/min values
      if (temperNow>maxRTemperature.getFl()) {
        maxRTemperature.setFl(temperNow);
      }
      if (temperNow<minRTemperature.getFl()) {
        minRTemperature.setFl(temperNow);
      }
    }
    
    
    if (ni>lastRecNight) { //night is out
      lastRecNight=ni;
      minNTemperature.setFl(todayMinTemp);
      todayMaxTemp = temperNow; //starting a new day
    } else { // night is underway
      todayMinTemp = (temperNow<todayMinTemp)?temperNow:todayMinTemp;
    }
    if (di>lastRecDay) { //day is out
      lastRecDay=di;
      maxDTemperature.setFl(todayMaxTemp);
      todayMinTemp = temperNow;//starting a new night
    }else { // day is underway
      todayMaxTemp = (temperNow>todayMaxTemp)?temperNow:todayMaxTemp;
    }
          

    presentTime.setTime(h,m);
    


    fObj = fObj->focusProcess();
    showAllInfo();
    //start relay impuls
    if ( presentTime.isEqualTime(morningTime) && (!(morningDuration.getIsEmpty())) && (!morningWateringON) && (!eveningWateringON) ) {
      morningWateringON = true;
      morningTime.setBlink(true);
      morningDuration.setBlink(true);
      impulseRelay();
    }
    if ( presentTime.isEqualTime(eveningTime) && (!(eveningDuration.getIsEmpty())) && (!morningWateringON) && (!eveningWateringON) ) {
      eveningWateringON = true;
      eveningTime.setBlink(true);
      eveningDuration.setBlink(true);
      impulseRelay();
    }

    //stop relay impuls
    unsigned long timest = millis();
    if (morningWateringON && ((timest/1000)-lastRelayImpuls > morningDuration.getInt()*60)) {
      morningWateringON = false;
      morningTime.setBlink(false);
      morningDuration.setBlink(false);
      impulseRelay();
    }
    if (eveningWateringON && ((timest/1000)-lastRelayImpuls > eveningDuration.getInt()*60)) {
      eveningWateringON = false;
      eveningTime.setBlink(false);
      eveningDuration.setBlink(false);
      impulseRelay();
    }

    delay(deltime);


}
