/* 
 Manchester Decoding, reading by delay rather than interrupt
 Rob Ward August 2014
 This example code is in the public domain.
 Use at your own risk, I will take no responsibility for any loss whatsoever its deployment.
 Visit https://github.com/robwlakes/ArduinoWeatherOS for the latest version and
 documentation. Filename: DebugManchester.ino
 
 DebugVersion_16, 1stAugust 2014
 
 */

//Interface Definitions
int     RxPin      = 8;    //The number of signal from the Rx
int     ledPin     = 13;   //The number of the onboard LED pin

// Variables for Manchester Receiver Logic:
word    sDelay     = 250;  //Small Delay about 1/4 of bit duration  try like 250 to 500
word    lDelay     = 500;  //Long Delay about 1/2 of bit duration  try like 500 to 1000, 1/4 + 1/2 = 3/4
byte    polarity   = 1;    //0 for lo->hi==1 or 1 for hi->lo==1 for Polarity, sets tempBit at start
byte    tempBit    = 1;    //Reflects the required transition polarity
byte    discards   = 0;    //how many leading "bits" need to be dumped, usually just a zero if anything eg discards=1
byte    discNos    = 0;    //Counter for the Discards 
boolean firstZero  = false;//has it processed the first zero yet?  This a "sync" bit.
boolean noErrors   = true; //flags if signal does not follow Manchester conventions
//variables for Header detection
byte    headerBits = 15;   //The number of ones expected to make a valid header
byte    headerHits = 0;    //Counts the number of "1"s to determine a header
//Variables for Byte storage
byte    dataByte   = 0;    //Accumulates the bit information
byte    nosBits    = 0;    //Counts to 8 bits within a dataByte
byte    maxBytes   = 9;    //Set the bytes collected after each header. NB if set too high, any end noise will cause an error
byte    nosBytes   = 0;    //Counter stays within 0 -> maxBytes
//Array for packet data, if required (at least one will be needed)
byte    manchester[12];    //Stores 4 banks of manchester pattern decoded on the fly
//Oregon bit pattern, causes a 4 bit rotation to the right, otherwise in Oregon data the LSB precedes MSB
byte  oregon[] = {
  16,32,64,128,1,2,4,8
};
byte    csIndex = 0;       //how many nibbles needed for checksum (2 less than nos of nibbles, as last two is CS
//Weather Variables
byte    quadrant  = 0;     //used to look up 16 positions around the compass rose
double  avWindspeed = 0.0;
double  gustWindspeed = 0.0; //now used for general anemometer readings rather than avWinspeed
float   rainTotal = 0.0;
float   rainRate  = 0.0;
double  temperature = 0.0;
int     humidity  = 0;
const char windDir[16][4] = {  
  "N  ", "NNE", "NE ", "ENE",  "E  ", "ESE", "SE ", "SSE",  "S  ", "SSW", "SW ", "WSW",  "W  ", "WNW", "NW ", "NNW"};

void setup() {
  Serial.begin(115200);
  pinMode(RxPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.println("Debug Manchester Version 11 - Printout sensors in Human readable format.");
  Serial.print("Using a delay of 1/4 bitWaveform ");
  Serial.print(sDelay,DEC);
  Serial.print(" uSecs 1/2 bitWaveform ");
  Serial.print(lDelay,DEC);
  Serial.println(" uSecs ");
  if (polarity){
    Serial.println("Negative Polarity hi->lo=1"); 
  }
  else{
    Serial.println("Positive Polarity lo->hi=1"); 
  }
  Serial.print(headerBits,DEC); 
  Serial.println(" bits expected for a valid header"); 
  if (discards){
    Serial.print(discards,DEC);
    Serial.println(" leading bits discarded from Packet"); 
  }
  else{
    Serial.println("All bits inside the Packet"); 
  }
  Serial.println("D 00 00001111 01 22223333 02 44445555 03 66667777 04 88889999 05 AAAABBBB 06 CCCCDDDD 07 EEEEFFFF 08 00001111 09 22223333 0A 44445555"); 
}

// Main routines, find header, then sync in with it, get a packet, and decode data in it, plus report any errors.
void loop(){
  tempBit=polarity^1;//toggle flag to track what the next data transition to expect
  noErrors=true;//flag that causes a restart of the scan if false
  firstZero=false;//flag that reflects whether the synch 0 has been detected
  headerHits=0;//counter for the number of 1's found in the header
  nosBits=0;//counter for the number of bits shifted in
  nosBytes=0;//counter for number of bytes stored in the payload.
  discNos = discards;//how many leading bits are discarded from valid bit stream
  maxBytes = 10; //This is the maximum number of bytes expected, this may need to be reduced or increased for other applications
  manchester[0]=0;
  //The following code is a generalised Manchester decoding routine that may be of use to others...
  //Please use and adapt it your your own purposes, just acknowledge the author you got it from
  while (noErrors && (nosBytes<maxBytes)){
    while(digitalRead(RxPin)!=tempBit){//pause here until a transition is found
    }
    delayMicroseconds(sDelay);//skip ahead to 3/4 of the bit pattern
    if (digitalRead(RxPin)!=tempBit){//if RxPin has changed it is definitely an error
      noErrors=false;//something has gone wrong, polarity has changed too early, ie always an error
    }
    //Waveform appears ok
    else{
      byte bitState = tempBit ^ polarity;//sample the bit value for incoming data here 
      //now 1 quarter into the next bit pattern,
      delayMicroseconds(lDelay);
      if(digitalRead(RxPin)==tempBit){//if RxPin has not swapped, then bitWaveform is swapping
        //If the header is done, then it means data bit change is occuring ie 1->0, or 0->1
        //data transition detection must swap, so it loops for the opposite transitions
        tempBit = tempBit^1;//toggle the next bit to look for
      }
      //process a one
      if(bitState==1){//could be a header bit or part of data payload
        if(!firstZero){//if first zero not been detected, then is must be a header bit
          headerHits++;
          if (headerHits==headerBits){//valid header accepted, minimum required found
            //digitalWrite(ledPin,1);//indicate a header detected
          }
        }

        else{
          add(bitState);//shift a one into the data payload
        }
      }
      //process a zero, or possibly an error waveform in header
      else{
        if(headerHits<headerBits){
          noErrors=false;//found a "zero" before we have enough header hits == error
        }
        else{
          //if we have the header processed and this is the first zero
          if (!firstZero){
            firstZero=true;//header has been found and now have synch 0
            //digitalWrite(ledPin,1); 
          }
          add(bitState);//shift the zero into the data payload 
        }
      }
    }
  }
  digitalWrite(ledPin,0);//Turn off LED and start a new search....
}

void add(byte bitData){
  if (discNos>0){ 
    discNos--;
  }
  else{
    if (bitData){
      manchester[nosBytes]|=oregon[nosBits]; //Set the bit 
    }
    //Oregon Scientific specific packet lengths
    nosBits++;
    if (nosBits==8){
      if(manchester[0]==0xA2){
        maxBytes=11;//rain
        csIndex =19;
      }
      if(manchester[0]==0xA1){
        maxBytes=10;//wind
        csIndex=18;
      }
      if(manchester[0]==0xAF){
        maxBytes=9;//temp
        csIndex=16;
      }
      if(manchester[0]==0xA3){
        maxBytes=10;//experimental, 9 data bytes and 1 byte CS
        csIndex =18;//CS byte begins 18 nibble
        digitalWrite(ledPin,1);
      }
      nosBits=0;
      nosBytes++;
      manchester[nosBytes]=0; //next byte to 0 to accumulate data
      //Serial.print("?");
    }
    if(nosBytes==maxBytes){
      //hexBinDump();
      if (ValidCS(csIndex)){
        analyseData();
      }
      noErrors=false;//make it begin from the start
    }
  }
}

void hexBinDump(){
  Serial.print("D ");
  for( int i=0; i < maxBytes; i++){ 
    byte mask = B10000000;
    if (manchester[i]<16){
      Serial.print("0");
    }
    Serial.print(manchester[i],HEX);
    Serial.print(" ");
    for (int k=0; k<8; k++){
      if (manchester[i] & mask){
        Serial.print("1");
      }
      else{
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
// Directly lifted, then modified from Brian's work, as nybbles bits are now in convenient order, ie MSNybble + LSNybble
// CS = the sum of nybbles, 1 to (CSpos-1), compared to CSpos byte (LSNybble) and CSpos+1 byte (MSNybble);
// This sums the nybbles in the packet and creates a 1 byte number, and then compared to the two nybbles beginning at CSpos
// Note that Temp and anemometer uses only 10 bytes but rainfall use 11 bytes per packet. (Rainfall CS spans a byte boundary)
bool ValidCS(int CSPos){
  bool ok = false;
  byte cs = 0;
  for (int x=1; x<CSPos; x++){
    byte test=nyb(x);
    cs +=test;
  }
  //do it by nibbles as some CS's cross the byte boundaries eg rainfall
  byte check1 = nyb(CSPos);
  byte check2 = nyb(CSPos+1);
  byte check = (check2<<4)+check1;
  if (false){  //true for debug
    Serial.print(check1,HEX);  //dump out the LSNybble Checksum
    Serial.print("(LSB), ");
    Serial.print(check2,HEX);  //dump out the MSNybble Checksum
    Serial.print("(MSB), ");
    Serial.print(check,HEX);   //dump out the Rx'ed predicted byte Checksum
    Serial.print("(combined),  calculated = ");
    Serial.println(cs,HEX);      //dump out the calculated byte Checksum
  }
  if (cs == check){
    ok = true;
  }
  return ok;
}
// Get a nybble from Manchester bytes, short name so equations elsewhere are neater :-)
byte nyb(int nybble){
  int bite = nybble / 2;       //DIV 2, find the byte
  int nybb  = nybble % 2;      //MOD 2  0=MSB 1=LSB
  byte b = manchester[bite];
  if (nybb == 0){
    b = (byte)((byte)(b) >> 4);
  }
  else{
    b = (byte)((byte)(b) & (byte)(0xf));
  }       
  return b;
}

void analyseData(){
  if (manchester[0]==0xaf){  //detected the Thermometer and Hygrometer
    thermom();
    dumpThermom();
  }
  if (manchester[0]==0xa1){      //detected the Anemometer and Wind Direction
    anemom();
    dumpAnemom();
  }
  if (manchester[0]==0xa2){      //detected the Rain Gauge
    rain();
    dumpRain();
  }
  if (manchester[0]==0xa3){      //detected an original Sensor designed by Us!!!
    //totally experimental
    totExp();
    //dumpTotExp();
  }

}

//Calculation Routines

/*   PCR800 Rain Gauge  Sample Data:
 //  0        1        2        3        4        5        6        7        8        9        A
 //  A2       91       40       50       93       39       33       31       10       08       02
 //  0   1    2   3    4   5    6   7    8   9    A   B    C   D    E   F    0   1    2   3    4   5  
 //  10100010 10010001 01000000 01010000 10010011 00111001 00110011 00110001 00010000 00001000 00000010
 //  -------- -------  bbbb---  RRRRRRRR 88889999 AAAABBBB CCCCDDDD EEEEFFFF 00001111 2222CCCC cccc
 
 // byte(0)_byte(1) = Sensor ID?????
 // bbbb = Battery indicator??? (7)  My investigations on the anemometer would disagree here.  
 // After exhaustive low battery tests these bbbb bits did not change
 // RRRRRRRR = Rolling Code Byte
 // 222211110000.FFFFEEEEDDDD = Total Rain Fall (inches)
 // CCCCBBBB.AAAA99998888 = Current Rain Rate (inches per hour)
 // ccccCCCC = 1 byte Checksum cf. sum of nybbles
 // Message length is 20 nybbles so working in inches
 Three tips caused the following
 1 tip=0.04 inches or 1.1mm (observed off the LCD)
 My experiment
 Personally I don't like this value. I think mult by 30 (it was 42, then 25) and divide by 1000 IS closer to return mm directly.
 0.127mm per tip??? It looks close the above. Again can't vouch 100% for this, any rigorous assistance would be appreciated.
 */
void rain(){
  rainTotal = float(((nyb(18)*100000)+(nyb(17)*10000)+(nyb(16)*1000)+(nyb(15)*100)+(nyb(14)*10)+nyb(13))*30/1000.0);
  //Serial.println((nyb(18)*100000)+(nyb(17)*10000)+(nyb(16)*1000)+(nyb(15)*100)+(nyb(14)*10)+nyb(13),DEC);
  rainRate = float(((nyb(8)*10000)+(nyb(9)*1000)+(nyb(10)*100)+(nyb(11)*10)+nyb(12))*30/1000.0);
  //Serial.println((nyb(8)*10000)+(nyb(9)*1000)+(nyb(10)*100)+(nyb(11)*10)+nyb(12),DEC);
}
void dumpRain(){
  Serial.print("Total Rain ");
  Serial.print(rainTotal);
  Serial.print(" mm, ");
  Serial.print("Rain Rate ");
  Serial.print(rainRate);
  Serial.println(" mm/hr ");
}

// WGR800 Wind speed sensor
// Sample Data:
// 0        1        2        3        4        5        6        7        8        9
// A1       98       40       8E       00       0C       70       04       00       34
// 0   1    2   3    4   5    6   7    8   9    A   B    C   D    E   F    0   1    2   3
// 10100001 10011000 01000000 10001110 00000000 00001100 01110000 00000100 00000000 00110100
// -------- -------- bbbb---- NRRRRRRR xxxx9999 xxxxxxxx CCCCDDDD xxxxFFFF 0000---- CCCCcccc
// Av Speed 0.4000000000m/s Gusts 0.7000000000m/s  Direction: N  

// byte(0)_byte(1) = Sensor ID?????
// bbbb = Battery indicator??? (7)  My investigations would disagree here.  After exhaustive low battery tests these bits did not change
// NRRRRRRR = Rolling Code Byte, the N bit is set to 1 for 64 cycles to indicate it is reset or new to the Rx box
// 9999 = Direction
// DDDD.CCCC = Gust Speed (m per sec)
// 0000.FFFF = Avg Speed(m per sec)
// multiply by 3600/1000 for km/hr
// ccccCCCC = 1 byte checksum cf. sum of nybbles
// packet length is 20 nybbles

void anemom(){
  //D A1 98 40 8E 08 0C 60 04 00 A4
  avWindspeed = ((nyb(16)*10)+ nyb(15))*3.6/10;
  gustWindspeed =((nyb(13)*10)+nyb(12))*3.6/10;
  quadrant = nyb(9) & 0xF;
}
void dumpAnemom(){
  Serial.print("Av Speed ");
  Serial.print(avWindspeed);
  Serial.print(" km/hr, Gusts ");
  Serial.print(gustWindspeed);
  Serial.print(" km/hr, Direction: ");
  Serial.print(quadrant);
  Serial.print(" -> ");
  Serial.println(windDir[quadrant]);
}

// THGN800 Temperature and Humidity Sensor
// 0        1        2        3        4        5        6        7        8        9          Bytes
// 0   1    2   3    4   5    6   7    8   9    A   B    C   D    E   F    0   1    2   3      nybbles
// 01011111 00010100 01000001 01000000 10001100 10000000 00001100 10100000 10110100 01111001   Bits
// -------- -------- bbbbcccc RRRRRRRR 88889999 AAAABBBB SSSSDDDD EEEE---- CCCCcccc --------   Explanation
// byte(0)_byte(1) = Sensor ID?????
// bbbb = Battery indicator??? (7), My investigations on the anemometer would disagree here.  After exhaustive low battery tests these bits did not change
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
void thermom(){
  temperature = (double)((nyb(11)*100)+(nyb(10)*10)+nyb(9))/10; //accuracy to 0.01 degree seems unlikely
  if(nyb(12)==1){//  Trigger a negative temperature
    temperature = -1.0*temperature;
  }
  humidity = (nyb(14)*10)+nyb(13);
}
void dumpThermom(){
  Serial.print("Temperature ");
  Serial.print(temperature);
  Serial.print(" degC, Humidity ");
  Serial.print(humidity);
  Serial.println("% Rel");
}

void totExp(){
  Serial.print("Total Experiment Delay=");
  int tot = manchester[nyb(3)*256+nyb(4)];
  Serial.print(tot,DEC);
  Serial.print(", Incrementing ");
  Serial.print(manchester[5],DEC);
  Serial.print(", Decrementing ");
  Serial.println(manchester[6],DEC);
}

void eraseManchester(){
  for( int i=0; i < maxBytes; i++){ 
    manchester[i]=0;
  }
}


























