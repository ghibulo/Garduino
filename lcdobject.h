#ifndef LCDOBJECT_H
#define LCDOBJECT_H

#include <LiquidCrystal.h>
#include "gardperif.h"


//how mamy secunds is cursor visible
const int durationCursor = 10;



// init 40x2 LCD
extern LiquidCrystal lcd;

//crazy value
const int infval = 60000;

enum GarduinoObjects { tpresentTemperature, tmaxDTemperature, tmaxRTemperature, 
                       tminNTemperature, tminRTemperature, tpresentTime, tmorningTime, 
                       tmorningDuration, teveningTime, teveningDuration};
  
class lcdObject {
private:


	int x,y;
  static int ax;
  static int ay;

  // visibility of cursor -> 'n' -> no cursor, 'c' -> cursor, 'b' -> blink 
  static char visCursor;
  
	// examples >> 't' -> 20:00, 'd' -> 7, 'f' -> -2.1
	char format;

  bool isBlinking;


  // true -> no data;
  bool isEmpty;

	//change focus
	lcdObject *l, *r, *u, *d;

	//data
	int hour, min, dnum;
	float fnum;

  //setting two-digits on the position [x,y]
  int lcdSetDigit (int x, int y, int maxd1, int maxd2);

  //setting time on the position [x,y]
  void lcdSetTime (int x, int y, int &h, int &m);

public:
  //identification for main loop
  GarduinoObjects me; 
  
  // state == 'f' => focus; 'v' => only visible; 'r' => edit
  char state;
  
  lcdObject(GarduinoObjects tme, int x, int y, char format);
  static void Init();
  static void HideCursor();
  static void ShowCursor();
	bool setTime(int h, int m);
  bool isEqualTime(lcdObject &anotherTime);
	void setInt(int dn);
	void setFl(float fn);
  void setBlink(bool b);
  float getFl();
  int getHour();
  int getMin();
  int getInt();
  bool getIsEmpty();
	void showData();
  void setFocus();
  void setEdit();
  void setNormal(); 
  void setNeighb(lcdObject *tl,lcdObject *tr,lcdObject *tu,lcdObject *td);
  lcdObject *focusProcess();

};

















#endif
