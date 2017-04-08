#include "gardperif.h"
#include "Arduino.h" 
#include <SPI.h>


OneWire dst =  OneWire(temperatPin);
unsigned long lastPress = 0;
unsigned long lastRelayImpuls = 0;
//unsigned long mxDurAfterPress = 7;
//address for thermometer
byte addr[8] = {0,0,0,0,0,0,0,0};

DS1302 rtc = DS1302(8, 9, 10); // Change the pins here if you want

void initPerifs() {
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);
  //fill addr for thermometer
  while (!dst.search(addr)) delay(250);
  
  pinMode(buttonPin,INPUT);
  pinMode(relayImpulsPin,OUTPUT);
}

char getBut() {
  int buttonValue = analogRead(buttonPin);
  //Serial.print("stisknuto:");Serial.println(buttonValue);
  for (int i=0;i<5;i++) {
     if ((buttonValue>(butConst[0][i]-butConst[0][5]))&&(buttonValue<(butConst[0][i]+butConst[0][5]))) {
        lastPress = millis()/1000;
        #ifdef DEBUG
        Serial.print("stisknuto:");Serial.println((char)butConst[1][i]);
        #endif
        return butConst[1][i];   
     }
  }
  //Serial.print("stisknuto:");Serial.println((char)butConst[1][5]);
  return butConst[1][5];
}

boolean noPressBut() {  
  char pok = getBut();
  //Serial.println(pok);
  return pok == butConst[1][5];
  
}

void prepareTemperature() {
  dst.reset();
  dst.select(addr);
  dst.write(0x44);        // start conversion, use ds.write(0x44,1) with parasite power on at the end
  //this take about 1s
}

float getTemperature() {
  byte i;
  byte data[12];


  dst.reset();
  dst.select(addr);    
  dst.write(0xBE);         // Read Scratchpad
 
  
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = dst.read();
  }


  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];

    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  return (float)raw / 16.0;
}

float getTemperatureOld() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  float celsius;

  
  //fill addr for thermometer
  while (!dst.search(addr)) delay(250);

#ifdef DEBUG


  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }


  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return 0.0;
  }
  Serial.println();

#endif

  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      #ifdef DEBUG
      Serial.println("  Chip = DS18S20");  // or old DS1820
      #endif
      type_s = 1;
      break;
    case 0x28:
      #ifdef DEBUG
      Serial.println("  Chip = DS18B20");
      #endif
      type_s = 0;
      break;
    case 0x22:
      #ifdef DEBUG
      Serial.println("  Chip = DS1822");
      #endif
      type_s = 0;
      break;
    default:
      #ifdef DEBUG
      Serial.println("Device is not a DS18x20 family device.");
      #endif
      return 0.0;
  } 

  dst.reset();
  dst.select(addr);
  dst.write(0x44);        // start conversion, use ds.write(0x44,1) with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = dst.reset();
  dst.select(addr);    
  dst.write(0xBE);         // Read Scratchpad
  
  #ifdef DEBUG
  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  #endif
  
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = dst.read();
    #ifdef DEBUG
    Serial.print(data[i], HEX);
    Serial.print(" ");
    #endif
  }
  #ifdef DEBUG
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();
  #endif

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  return (float)raw / 16.0;
}

long getDateIndex(int y, int m, int d)
{
    m = (m + 9) % 12;
    y = y - m/10;
    return 365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + ( d - 1 );
}



void getTimeRTC(int &h, int &m, int &di, int &ni) {
  Time t = rtc.getTime();
  h=t.hour;
  m=t.min; 
  long indexDay = getDateIndex(t.year, t.mon,t.date);
  // (8:00-20:00 day, 20:00 - 8:00 night)
  di=(h>=20)?indexDay+1:indexDay;
  ni=(h<8)?indexDay-1:indexDay;
}

char *form2digit(int num) {
  char *outp="xx";
  if (num<10) {
    outp[0]=48;
  } else {
    outp[0]=char((num / 10)+48);
  }
  outp[1]=char((num % 10)+48);
  return outp;
}


void impulseRelay() {
  digitalWrite(relayImpulsPin, HIGH);   // impulse for relay
  delay(1000);                  // waits for a second
  digitalWrite(relayImpulsPin, LOW);
  lastRelayImpuls = millis()/1000;
}

