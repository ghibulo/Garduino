#include "lcdobject.h"
#include <Arduino.h>

LiquidCrystal lcd = LiquidCrystal(12, 11, 5, 4, 3, 13);
int lcdObject::ax = 0;
int lcdObject::ay = 0;
char lcdObject::visCursor = 'n';



int lcdObject::lcdSetDigit (int x, int y, int maxd1, int maxd2) {
    int selNum[2] = {0,0};
    int maxd[2] = {maxd1, maxd2};
    int cx=x, cy=y;
    char b;
    do {
      lcd.setCursor(x,y);
      lcd.print(form2digit(selNum[0]*10+selNum[1]));
      lcd.setCursor(cx,cy);
      b = getBut();
      int ind = cx-x;
      switch (b) {
        case 'u': selNum[ind] = (selNum[ind]<maxd[ind])?selNum[ind]+1:0;break;
        case 'd': selNum[ind] = (selNum[ind]>0)?selNum[ind]-1:maxd[ind];break;
        case 'r': //r,l -> switch to another digit
        case 'l': cx = (cx == x)?x+1:x;break;
      }
      delay(200);
    } while (b!='s');
    return selNum[0]*10+selNum[1];
}

void lcdObject::lcdSetTime (int x, int y, int &h, int &m) {
    int selTime[4] = {0,0,0,0};
    int maxd[4] = {2,9,5,9};
    int cx=x, cy=y;
    int ind; 
    char b;
    do {
      lcd.setCursor(x,y);
      lcd.print(form2digit(selTime[0]*10+selTime[1]));
      lcd.print(":");
      lcd.print(form2digit(selTime[2]*10+selTime[3]));
      ind=cx-x;
      if (ind>1) lcd.setCursor(cx+1,cy); else lcd.setCursor(cx,cy);
      b = getBut();
      switch (b) {
        case 'u': selTime[ind] = (selTime[ind]<maxd[ind])?selTime[ind]+1:0;break;
        case 'd': selTime[ind] = (selTime[ind]>0)?selTime[ind]-1:maxd[ind];break;
        case 'l': cx = (ind > 0)?cx-1:x+3;break;
        case 'r': cx = (ind < 3)?cx+1:x;break;
      }
      delay(200);
    } while (b!='s');
    h=selTime[0]*10+selTime[1];
    m=selTime[2]*10+selTime[3];
}


lcdObject::lcdObject(GarduinoObjects tme, int x, int y, char format){
	lcdObject::x = x;
	lcdObject::y = y;
	lcdObject::format = format;
  state = 'v';
  isEmpty = true;
  isBlinking = false;
  me = tme;
}

void lcdObject::Init() {
  lcd.begin(40, 2);
}

void lcdObject::HideCursor() {
  lcd.noCursor();
}

void lcdObject::ShowCursor() {
  lcd.setCursor(ax,ay);
  switch (visCursor) {
    case 'n': lcd.noCursor();lcd.noBlink();break;
    case 'c': lcd.cursor();lcd.noBlink();break;
    case 'b': lcd.cursor();lcd.blink();break;
  }
}

bool lcdObject::setTime(int h, int m) {
  if ((h>=0)&&(h<=24)&&(m>=0)&&(m<=59)) {
      hour = h; min = m;isEmpty = false;
      return true;
  } else {
    if (h==infval) {
      isEmpty = true;
      return true;
    }
  }
  return false;
}

 bool lcdObject::isEqualTime(lcdObject &anotherTime) {
    return ((format=='t')&&((hour==anotherTime.getHour())&&(min==anotherTime.getMin())));
 }

void lcdObject::setInt(int dn) {
  isEmpty =  (dn==infval); 
	dnum = dn;

}
void lcdObject::setFl(float fn) {
  isEmpty =  (fn==infval);
	fnum = fn;
}

void lcdObject::setBlink(bool b) {
  isBlinking=b;
}

float lcdObject::getFl() {
  return fnum;
}

int lcdObject::getHour() {
  if ((format=='t')&&(!isEmpty)) {
    return hour;
  } else return infval;
}

int lcdObject::getMin() {
  if ((format=='t')&&(!isEmpty)) {
    return min;
  } else return infval;
}

bool lcdObject::getIsEmpty() {
  return isEmpty;
}

int lcdObject::getInt() {
  if ((format == 'd') && (!isEmpty)) {
    return dnum;
  } else return infval;
}

void lcdObject::showData() {
  int oddSec = (millis()/1000)%2;
	lcd.setCursor(x,y);
  if ((isBlinking)&&(oddSec)) {
    if (format=='d') lcd.print("  "); else lcd.print("     ");
    return;
  }
  if (isEmpty) {
    if (format=='t') {
      lcd.print("--:--"); 
    } else {
      lcd.print("--");
    }
  } else {
  	if (format=='d') {
  		lcd.print(form2digit(dnum));
  	}
  	if (format=='f') {
  		lcd.print(fnum);
  	}
  	if (format=='t') {
  		lcd.print(form2digit(hour));
  		if ((me==tpresentTime)&&(oddSec)) {
        lcd.print(' ');}
  		else {
  		  lcd.print(':');
  		}
  		lcd.print(form2digit(min));
  	}
  }
}


void lcdObject::setFocus() {
  state = 'f';//Serial.print(x);Serial.print(" - ");Serial.print(y);
  ax=x;ay=y;
  visCursor = 'c';lcdObject::ShowCursor();
}

void lcdObject::setEdit() {
  state = 'r';
  ax=x;ay=y;
  visCursor = 'b';lcdObject::ShowCursor();
  while (!noPressBut());// Serial.println("Nepustil!"); // until releas 's'
  //Serial.println("pustil!");
}

void lcdObject::setNormal() {
  state = 'v';
  ax=x;ay=y;
  visCursor = 'n';lcdObject::ShowCursor();
  while (!noPressBut()); // until all releas
}

void lcdObject::setNeighb(lcdObject *tl,lcdObject *tr,lcdObject *tu,lcdObject *td) {
  l=tl;r=tr;u=tu;d=td;
}

lcdObject *lcdObject::focusProcess() {
  if (state=='v') {
    if (!noPressBut()) {
      setFocus();
    } 

    return this;
  }
  if (state=='f') {
    #ifdef DEBUG
    Serial.print("lastPress:");Serial.println(lastPress);
    Serial.print("durationCursor:");Serial.println(durationCursor);
    Serial.print("millis:");Serial.println(millis());
    #endif
    
    if (lastPress + durationCursor < (millis()/1000)) {// end of focus?
      setNormal();
      return this;
    } 
    char key = getBut();
    if (key==butConst[1][5]) { //no press
      return this;
    } else { //up, down, left, right, select
            //Serial.println("testuji");
      state='v';
      switch (key)  { 
        case 's': //select-key pressed
          setEdit();
          return this;
        case 'l': //left-key pressed
          if (l==NULL) 
            return this; 
          else {         
            l->setFocus();
            return l;
          }
          break;
        case 'r':  //right-key pressed
          if (r==NULL) 
            return this; 
          else {
            //Serial.println("prave");
            r->setFocus();
            return r;
          }
          break;
         case 'u':  //up-key pressed
          if (u==NULL) 
            return this; 
          else {
            u->setFocus();
            return u;
          }
          break; 
        case 'd':  //down-key pressed
          if (d==NULL) 
            return this; 
          else {
            d->setFocus();
            return d;
          }
          break; 
      } //switch     
        
      } //else
    } //if (state=='f')

    if (state == 'r') {
      if (format == 'd') {
        setInt(lcdSetDigit (x,y,9,9));
      }
      if (format == 't') {
        int h,m;
        //Serial.println("Vstupuju do nastavovani, isEmpty...");
        //Serial.println(isEmpty);
        lcdSetTime (x,y,h,m);
        //Serial.println("Vystupuju z nastavovani, isEmpty...");
        //Serial.println(isEmpty);
        //Serial.print(h);Serial.print(":");Serial.println(m);
        bool isok = setTime(h,m);
        //Serial.print("isok=");Serial.println(isok);
        //if present-time -> setting rtc
        if ((me==tpresentTime)&&(isok)) rtc.setTime(h, m, 0);
      }
      
      setNormal();
      showData();
    }
    
  }// focusProcess()
  
  
  

