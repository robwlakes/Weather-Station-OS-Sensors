// testcolors demo for Adafruit RGBmatrixPanel library.
// Renders 512 colors on our 16x32 RGB LED matrix:
// http://www.adafruit.com/products/420
// Library supports 4096 colors, but there aren't that many pixels!  :)

// Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon
// for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.

#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
#include <Wire.h>           // One wire library
#include "RTClib.h"         //Real time clock DS1307

#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, false);
RTC_DS1307 rtc;

char temp[] = "Temp";
char humi[] = "Humi";

//Interface Definitions
int RxPin = 11; //incoming from 433MHz Rx
int ledPin = 13; //status LED

// Variables for Manchester Receiver Logic:
word    sDelay     = 305;    //Small Delay about 1/4 of bit duration  try like 450 to 550
word    lDelay     = 610;   //Long Delay about 1/2 of bit duration  try like 900 to 1100, 1/4 + 1/2 = 3/4
byte    polarity   = 0;      //0 for lo->hi==1 or 1 for hi->lo==1 for Polarity, sets tempBit at start, 0 for these sensors
byte    tempBit    = 1;      //Reflects the required transition polarity
byte    discards   = 0;      //how many leading "bits" need to be dumped, usually just a zero if anything eg discards=1
byte    discNos    = 0;      //Counter for the Discards
boolean firstZero  = false;  //has it processed the first zero yet?  This a "sync" bit.
boolean noErrors   = true;   //flags if signal does not follow Manchester conventions
//variables for Header detection
byte    headerBits = 15;     //The number of ones expected to make a valid header
byte    headerHits = 0;      //Counts the number of "1"s to determine a header
//Variables for Byte storage
byte    dataByte   = 0;      //Accumulates the bit information
byte    nosBits    = 0;      //Counts to 8 bits within a dataByte
byte    maxBytes   = 12;     //Set the bytes collected after each header. NB if set too high, any end noise will cause an error
byte    nosBytes   = 0;      //Counter stays within 0 -> maxBytes
byte    csIndex    = 0;      //counter for nibbles needed for checksum
//Bank array for packet (at least one will be needed)
byte    manchester[12];      //Stores manchester pattern decoded on the fly
//Oregon bit pattern, indexed causes nibble rotation to the right, ABCDabcd becomes DCBAdcba
byte    oregon[]   = {
  16, 32, 64, 128, 1, 2, 4, 8
};

//Weather Variables
double  temperature   = 2.2;
int     humidity      = 1;
int     humidCorrect  = 0; //correction factor for Hunidity Sensor
//House keeping/debug
int seconds = 0;
int readings = 0;       //just to track readings
boolean intBit = false; //mainloop scans this for flag to process Interrrupt
//Real time clock, days of the week, Sun=0
char daysOfTheWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
boolean happyhour = false;
boolean stateDisp = false; //to make weather only update display when verified
byte once = 1; //toggel so display does not strobe

void setup() {
  Serial.begin(115200);
  //set up status LED
  pinMode(ledPin, OUTPUT);
  //433MHz RX input
  pinMode(RxPin, INPUT);
  //Matrix setup
  matrix.begin();
  matrix.setTextSize(1);    // size 1 == 8 pixels high
  //Realtime clock setup
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //NB These lines must be commented out immediately after use
    //Or the clock will reset to what was at compile every time it is run in the CPU.
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21,  3,  0, 0));  //eg
    //                     year, M,  D,  H,  M, S
    //rtc.adjust(DateTime(2018, 6,  5,  11, 59, 0));
    //AND disable those lines if it is set already and just the program is being updated.
    //Then remove and replace the battery ASAP and the new time will take over.
  }
  clock2matrix();
}

void loop() {
  DateTime now = rtc.now(); //read clock
  /*
       Serial.print(now.hour(), DEC);
       Serial.print(':');
       Serial.print(now.minute(), DEC);
       Serial.print(':');
       Serial.print(now.second(), DEC);
       Serial.print("  =  ");
  */
  // Serial.print(F(__DATE__));
  //Serial.print("  *  ");
  //Serial.print(F(__TIME__));
  //Serial.println();

  int secs = now.second() / 6;
  if ((secs % 2 == 1) && stateDisp && (once == 1)) {
    weather2matrix();
    once = 2;
  }
  if ((secs % 2 == 0) && (once == 2)) {
    clock2matrix();
    once = 1;
  }
  tempBit = polarity ^ 1;
  noErrors = true;
  firstZero = false;
  headerHits = 0;
  nosBits = 0;
  maxBytes = 15; //too big for any known OS signal
  nosBytes = 0;
  discNos = discards;
  manchester[0] = 0;
  digitalWrite(ledPin, 0);//Reset status LED
  while (noErrors && (nosBytes < maxBytes)) {
    while (digitalRead(RxPin) != tempBit) {
    }
    delayMicroseconds(sDelay);
    if (digitalRead(RxPin) != tempBit) {
      noErrors = false;
    }
    else {
      byte bitState = tempBit ^ polarity;
      delayMicroseconds(lDelay);
      if (digitalRead(RxPin) == tempBit) {
        tempBit = tempBit ^ 1;
      }
      if (bitState == 1) {
        if (!firstZero) {
          headerHits++;
          if (headerHits == headerBits) {
            digitalWrite(ledPin, 1); //possible header detected?
          }
        }
        else {
          add(bitState);
        }
      }
      else {
        if (headerHits < headerBits) {
          noErrors = false;
        }
        else {
          if ((!firstZero) && (headerHits >= headerBits)) {
            firstZero = true;
            digitalWrite(ledPin, 1); //first data 0 detected
            //Serial.println("Data?");//remove if working
          }
          add(bitState);
        }
      }
    }
  }
  digitalWrite(ledPin, 0);
}

void add(byte bitData) {
  boolean identity = false;
  if (discNos > 0) {
    discNos--;//discard bits before real data
  }
  else {
    //the incoming bitstream has bytes placed in reversed nibble order on the fly, then the CS is done.
    if (bitData) {
      //if it is a '1' OR it in, others leave at a '0'
      manchester[nosBytes] |= oregon[nosBits];//places the reversed low nibble, with hi nibble, on the fly!!!
    }
    //Oregon Scientific sensors have specific packet lengths
    //Maximum bytes for each sensor must set once the sensor has been detected.
    if (manchester[0] == 0xA1) {
      maxBytes = 9; //temp
      csIndex = 16;
      identity = true;
    }
    nosBits++;
    //Pack the bits into 8bit bytes
    if (nosBits == 8) {
      nosBits = 0;
      nosBytes++;
      manchester[nosBytes] = 0; //next byte to 0 to accumulate data
    }
    //Check the bytes for a valid packet once maxBytes received
    if (nosBytes == maxBytes) {
      //hexBinDump();
      digitalWrite(ledPin, 1);
      //Check Checksum first
      if (ValidCS(csIndex)) {
        //Process the byte array into Human readable numbers
        analyseData();
      }
      noErrors = false; //make it begin again from the start
      if ((identity == false) && (nosBytes < 12)) {
        hexBinDump();
      }
    }
  }
}

//Useful to invoke to debug the byte Array
void hexBinDump() {
  //Serial.println("T A3 10100011 07 00000111 02 00000010 AA 10101010 F0 11110000 06 00000110 FF 11111111 07 00000111 33 00110011 60 01100000");
  Serial.print("D ");
  for ( int i = 0; i < maxBytes; i++) {
    byte mask = B10000000;
    if (manchester[i] < 10) { //Formats Hex<16 to 2 digits, adds the leading 0
      Serial.print("0");
    }
    Serial.print(manchester[i], HEX);
    Serial.print(" ");
    for (int k = 0; k < 8; k++) {
      if (manchester[i] & mask) {
        Serial.print("1");
      }
      else {
        Serial.print("0");
      }
      mask = mask >> 1;
    }
    Serial.print(" ");
  }
  Serial.println();
}

//Support Routines for Nybbles and CheckSum

// http://www.lostbyte.com/Arduino-OSV3/ (9) brian@lostbyte.com
// Directly lifted, then modified from Brian's work. Now nybble's bits are pre-processed into standard order, ie MSNybble + LSNybble
// so recieved "A3" becomes "3A"
// CS = the sum of nybbles, 1 to (CSpos-1), then compared to CSpos nybble (LSNybble) and CSpos+1 nybble (MSNybble);
// This sums the nybbles in the packet and creates a 1 byte number, and this is compared to the two nybbles beginning at CSpos
bool ValidCS(int CSPos) {
  boolean ok = false;
  byte cs = 0;
  for (int x = 1; x < CSPos; x++) {
    byte test = nyb(x);
    cs += test;
  }
  //do it by nybbles as some CS's cross the byte boundaries eg rainfall
  byte check1 = nyb(CSPos);
  byte check2 = nyb(CSPos + 1);
  byte check = (check2 << 4) + check1;
  if (cs == check) {
    ok = true;
  }
  return ok;
}

// Get a nybble from manchester bytes, short name so equations elsewhere are neater :-)
// Enables the byte array to be indexed as an array of nybbles
byte nyb(int nybble) {
  int bite = nybble / 2;       //DIV 2, find the byte
  int nybb  = nybble % 2;      //MOD 2  0=MSB 1=LSB
  byte b = manchester[bite];
  if (nybb == 0) {
    b = (byte)((byte)(b) >> 4);
  }
  else {
    b = (byte)((byte)(b) & (byte)(0xf));
  }
  return b;
}

//enable debug dumps if formatted data required
void analyseData() {
  if (manchester[0] == 0xA1) { //detected the Thermometer and Hygrometer (every 53seconds)
    //Serial.print(readings, DEC);
    //Serial.print("=");
    thermom();//do the actual calculations
    if ((manchester[2] & 2)) {
      stateDisp = true;
      //weather2matrix();// Matrix display of readings
    }
    //dumpThermom();// Send that data to the terminal
    delay(1000);//let things stabilize
    readings++;//useful if debugging
  }
  eraseManchester();
}

//Calculation Routines

// THGN800 Temperature and Humidity Sensor
// 0        1        2        3        4        5        6        7        8        9          Bytes
// 0   1    2   3    4   5    6   7    8   9    A   B    C   D    E   F    0   1    2   3      nybbles
// 01011111 00010100 01000001 01000000 10001100 10000000 00001100 10100000 10110100 01111001   Bits
// -------- -------- bbbbcccc RRRRRRRR 88889999 AAAABBBB SSSSDDDD EEEE---- CCCCcccc --------   Explanation
// byte(0)_byte(1) = Sensor ID?????
// bbbb = Battery indicator??? (7), My investigations on the anemometer would disagree here.  After exhaustive low battery tests these bits did not change
// cccc = Sensor Number ie 0x01=1,0x02=2,0x04=3
// RRRRRRRR = Rolling code byte
// nybble(5) is channel selector c (Switch on the sensor to allocate it a number)
// BBBBAAAA.99998888 Temperature in BCD
// SSSS sign for negative (- is !=0)
// EEEEDDDD Humidity in BCD
// ccccCCCC 1 byte checksum cf. sum of nybbles
// Packet length is 18 nybbles and indeterminate after that
// H 00 01 02 03 04 05 06 07 08 09    Byte Sequence
// D AF 82 41 CB 89 42 00 48 85 55    Real example
// Temperature 24.9799995422 degC Humidity 40.0000000000 % rel

void thermom() {
  temperature = (double)((nyb(11) * 100) + (nyb(10) * 10) + nyb(9)) / 10; //accuracy to 0.1 degree seems unlikely
  //The following line has been corrected by Darko in Italy, thank you, Grazie ragazzi!!!
  if (nyb(12) == 8) { //  Trigger a negative temperature
    temperature = -1.0 * temperature;
  }
  humidity = (nyb(14) * 10) + nyb(13);
}

void dumpThermom() {
  //this reports the sensor number to the console but does not act on it
  if ((manchester[2] & 1)) {
    Serial.print("Sensor 1: ");
  }
  if ((manchester[2] & 2)) {
    Serial.print("Sensor 2: ");
  }
  if ((manchester[2] & 4)) {
    Serial.print("Sensor 3: ");
  }
  Serial.print("Temperature ");
  Serial.print(temperature);
  Serial.print(" degC, Humidity ");
  Serial.print(humidity-humidCorrect);
  Serial.println("% Rel");
}

void weather2matrix() {
  //Show data on the LCD if Sensor# =2
  matrix.setTextColor(matrix.Color333(1, 0, 0));
  matrix.fillScreen(0);
  matrix.setCursor(3, 0);
  matrix.print("T");
  matrix.setCursor(7, 0);
  matrix.print(":");
  matrix.setCursor(11, 0);
  matrix.print(temperature, 0);
  matrix.setCursor(23, 0);
  matrix.print("C");
  matrix.setTextColor(matrix.Color333(0, 0, 1));
  matrix.setCursor(3, 8);
  matrix.print("H");
  matrix.setCursor(7, 8);
  matrix.print(":");
  matrix.setCursor(11, 8);
  matrix.print(humidity-humidCorrect, 1);
  matrix.setCursor(23, 8);
  matrix.print("%");
}

void clock2matrix() {
  int tab = 0;
  delay(100);
  DateTime now = rtc.now(); //read clock
  if (now.second() < 29) {
    //Now check for happy hour
    int myhour = now.hour()%12; //modulo 12 counting of the hours 0-11
    int mydayOfWeek = now.dayOfTheWeek();
    //This was considered for our local tavern but held back
    //Serial.print(myhour);
    //Serial.print(":");
    //Serial.println(mydayOfWeek);
    //check for happy hour, Friday=5, hour=18 (ie 6-7pm)
    if ((myhour == 6) && (mydayOfWeek == 5)) {
      //happyhour = true;
      //Serial.print("Happy hour True");
    }
    else {
      happyhour = false;
    }
    //time in top line
    matrix.fillScreen(0);
    matrix.setTextColor(matrix.Color333(0, 1, 0));
    //wrap the time around 12 hour format
    if (myhour ==0) {
      myhour = 12; // so no '0' hours
    }
    //format for single and double digit hours
    if (myhour < 10) {
      tab = 9; //single digit
    }
    else {
      tab = 3; //double digit
    }
    //display the hour
    matrix.setCursor(tab, 0);
    matrix.print(myhour, DEC);
    //separator
    matrix.setCursor(13, 0);
    matrix.print(':');
    //display the minutes
    int myminute = now.minute();
    if (myminute < 10) {
      tab = 23;
      matrix.setCursor(17, 0);
      matrix.print('0');
    }
    else {
      tab = 17;
    }
    matrix.setCursor(tab, 0);
    matrix.print(myminute, DEC);

    //date in bottom line
    matrix.setTextColor(matrix.Color333(1, 1, 0));
    //format for single and double digit days
    int myday = now.day();
    if (myday < 10) {
      tab = 6;
    }
    else {
      tab = 0;
    }
    matrix.setCursor(tab, 8);
    matrix.print(myday, DEC);
    int mymonth = now.day();
    if (mymonth < 10) {
      tab = 19;
    }
    else {
      tab = 13;
    }
    matrix.setCursor(13, 8);
    matrix.print(months[now.month() - 1]);
    if (happyhour == true) {
      matrix.fillScreen(0);
      matrix.setTextColor(matrix.Color333(1, 0, 1));
      matrix.setCursor(2, 0);
      matrix.print("Happy");
      matrix.setTextColor(matrix.Color333(1, 0, 1));
      matrix.setCursor(5, 8);
      matrix.print("Hour");
      //Serial.println("Happy hour Shown");
    }
  }
}

void eraseManchester() {
  for ( int i = 0; i < 15; i++) {
    manchester[i] = 0;
  }
}

