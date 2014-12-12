/* 
 Manchester Decoding, reading by delay rather than interrupt
 Rob Ward August 2014
 This example code is in the public domain.
 Use at your own risk, I will take no responsibility for any loss whatsoever its deployment.
 Visit https://github.com/robwlakes/ArduinoWeatherOS for the latest version and
 documentation.
 
 This program dumps out binary numbers for any Manchester encoded signal that fits the parameters you have entered
 
 It should handle:
 Checking waveform conforms to Machester encoding 
 Any length header (currently set to 10)
 Find the Synching zero
 Discard the zero (and any other bits) if required (currently included in bytes)
 Any number of bytes (Currently set to 8)
 
 What is does not do:
 Is any error checking of data
 Any processing of data (eg reversal of significance within nibbles)
 Calculations to change the data into human redable form.
 Compose messages to the outside world to summarise incoming data.
 
 Filename: DebugVersion_12_BestRaw, 1stAugust 2014
 
 Most of the parameters can be changed to investigate other Manchester encoded signals.
 Readers/users are invited to use the core decoding part of this code to interface with
 other products that use Manchester encoding, or if necessary invent your own with a
 Manchester encoding transmitter.
 
 
 */

//Interface Definitions
int RxPin           = 8;   //The number of signal from the Rx
int ledPin          = 13;  //The number of the onboard LED pin

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
byte    headerBits = 10;   //The number of ones expected to make a valid header
byte    headerHits = 0;    //Counts the number of "1"s to determine a header
//Variables for Byte storage
byte    dataByte   = 0;    //Accumulates the bit information
byte    nosBits    = 0;    //Counts to 8 bits within a dataByte
byte    maxBytes   = 8;    //Set the bytes collected after each header. NB if set too high, any end noise will cause an error
byte    nosBytes   = 0;    //Counter stays within 0 -> maxBytes
//Variables for multiple packets
byte    bank       = 0;    //Points to the array of 0 to 3 banks of results from up to 4 last data downloads 
byte    nosRepeats = 0;    //Number of times the header/data is fetched at least once or up to 4 times
//Banks for multiple packets if required (at least one will be needed)
byte  manchester[12];   //Stores 4 banks of manchester pattern decoded on the fly

void setup() {
  Serial.begin(115200);
  pinMode(RxPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.println("Debug Manchester Version 12 Raw");
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
  Serial.println("D 00 00001111 01 22223333 02 44445555 03 66667777 04 88889999 05 AAAABBBB 06 CCCCDDDD 07 EEEEFFFF"); 
  eraseManchester();
}

// Main routines, find header, then sync in with it, get a packet, and decode data in it, plus report any errors.
void loop(){
  tempBit=polarity^1;//toggle flag to track what the next data transition to expect
  noErrors=true;//flag that causes a restart of the scan if false
  firstZero=false;//flag that reflects whether the synch 0 has been detected
  headerHits=0;//counter for the number of 1's found in the header
  nosBits=0;//counter for the number of bits shifted in
  nosBytes=0;//for for the number of bytes required in the payload
  discNos = discards;//how many leading bits are discarded from valid bit stream
  eraseManchester();
  maxBytes = 8; //This is the minimum number of bytes expected, this may need to be reduced or increased for other applications
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

void analyseData(){ 
}

void add(byte bitData){
  if (discNos>0){ //some manchester encodings strip off the first zero or more
    discNos--;//discards some bits, not always used
  }
  else{
    dataByte=(dataByte<<1)|bitData;//OR in the data bit
    nosBits++;//count 'em!!
    if (nosBits==8){
      nosBits=0;
      manchester[nosBytes]=dataByte;
      nosBytes++;
      if(nosBytes==maxBytes){
        hexBinDump();
        nosBytes=0;
        eraseManchester();
      }
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

void eraseManchester(){
  for( int i=0; i < maxBytes; i++){ 
    manchester[i]=0;
  }
}



























