/* 
 Manchester Encoding, reading by delay rather than interrupt
 Rob Ward September 2014
 This example code is in the public domain.
 Use at your own risk, I will take no responsibility for any loss whatsoever its deployment.
 Visit https://github.com/robwlakes/433MHz_Tx_Rx 
 for the latest version and documentation. 

  The following format relates to the Weather stations, mainly because I wanted to incorporate
  extra data from other remote RF sensors into my WWW server's weather pages. eg daily solar power
  I currently use the Oregon Scientific line of Weather Stations, using the V3.0 protocol.
  Hence I wanted to be able to make my Tx use a compatible protocol to simplify the receiver program logic
  that decodes the OS protocol.  As the Tx mimics a sensor, it would also need to alert the receiver to be 
  able to distinguish multiple transmitters/sensors apart.  ie different ID number's etc so they don't 
  clash with the existing OS transmitters.  The set up below has not been tested apart from with my WMR86
  The example here is a temperature sensor based on a Dallas DS18B20.  My aim is to be able to log the
  temperature in my solar hot water system.  The tank has a sensor port let into the side of the tank with
  a diameter of 5mm.  Obviously trying to fit a standard Oregon Scientific temperature sensor into this is
  totally out of the question, however the Dallas TO-3 style case is fine.
  Other sensors, like conductivity in soil, tank water levels, animal feed levels, pet behaviour are just a
  few examples of sensors that could be built and still incorporate the Oregon Scientific weather equipment.
  The huge advantage is that the convenient weather station sensors add quality data to the other measurements.
  Hugely important questions like: Does my cat go outside when the wind is from the east?? Could be solved....
  Just joking on that one.  No doubt there are more worthwhile serious challenges and 
  obviously other users may like to determine their own sensor packet format(s), 
  or adapt it in a major way to their own unique weather station protocols,
  ie build a weather station from scratch.
 */

#include <OneWire.h>
#define DS18S20_ID 0x10
#define DS18B20_ID 0x28

const int TxPin = 8;     // the TxPin applied to the 433MHz modulator pin
const int ledPin = 13;   // the number of the onboard LED pin

byte thisBit;            //reflects the current data bit polarity,

byte manchester[10];     //stores a bank of data to be manchester encoded on the fly, say first 8 bytes
byte bytePointer = 0;    //increments through manchester[data]
byte maxBytes = 10;      //number of bytes transmitted
byte oregon[]={
  16,32,64,128,1,2,4,8}; //Oregon bit mask, protocol is Hi Nibble followed by Lo nibble, but bits in nibble in reverse order
word sDelay = 550;       //1/2 of bit pattern duration, begin with 500uS (a common choice, data bit length = 1ms)
int seconds =0;

byte data[12];
float temp;

// DS18S20 Temperature chip i/o
OneWire ds(10);  // on pin 10
/*
Pins from Left to right
 1  = Ground
 2  = Digital data
 3  = +5V
 Also connect 2&3 with a 4K7 Ohm resistor (pull-up)
 */

void setup(){
  pinMode(TxPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  //Dummy data in the Manchester Byte Array (can be used just to test it out).
  manchester[0] = B10100011; //0xA3; //eg Brand/Sensor ID, can be from 0 to 255
  manchester[1] = B00000111; //eg Packet type (wind, rain, solar, UV etc) 7?
  manchester[2] = 0;         //First byte data eg sunlight or temperature
  manchester[3] = 0;         //Second byte data
  manchester[4] = B11110000; //dummy data, bit patterns
  manchester[5] = B00000000; //dummy data, incremented
  manchester[6] = B11111111; //dummy data, decremented
  manchester[7] = B10101010; //dummy data, bit patterns
  manchester[8] = B00110011; //dummy data, bit patterns
  manchester[9] = 0;         //checksum, calculated from preceding data

  Serial.begin(115200);
  //while the serial stream is not open, do nothing:
  while (!Serial) ;
  //Serial.println("Transmitter On Air....");//Just to check if you like
}

void loop(){
  if (getTemperature()){
    Serial.print("Temperature is (");
    manchester[2] = data[1];
    manchester[3] = data[0];
    temp = float( (data[1] << 8) + data[0] )/16;
    //1 LSB bit change = 0.06 of a degree, nice!!!
    Serial.print(data[1]);//Binary version MSB
    Serial.print("*256 + ");
    Serial.print(data[0]);//Binary version LSB
    Serial.print(")/16 = ");
    Serial.print(temp,2);//Float version
    Serial.println("C");
    transmit();
  } 
  //Wait for 47 Seconds before next transmission
  for (int second=0;second<47;second++){
    delay(1000); //a Second
  }
}
void transmit(){
  digitalWrite(ledPin,HIGH);
  manchester[5]++;         //just tickle over the bits in the number to show it can change
  manchester[6]--;         //as above
  //hexBinDump();
  ValidCS(18);
  digitalWrite(TxPin,0);   //begin on Signal Off
  for (int j=0;j<30;j++){  //header 30 of 1's, probably excessive, 20 could be enough
    Tx1();                 //Send 1's as it stabilises the Rx's AGC the quickest
  }
  //Oregon has the start bit within the Byte boundaries so is in the manchester Array
  //Tx0(); //put in a single 0 as the first "Start Bit", indicates end of Header bit stream of 1's
  bytePointer = 0; //Point to start of manchester array
  //Serial.print("R    ");
  for (bytePointer =0;bytePointer<maxBytes;bytePointer++){
    byte check =128;
    for (int k=0;k<8;k++){
      if (oregon[k] & manchester[bytePointer]){
        Tx1(); //Send a 1
        //Serial.print("1");
      }
      else{
        Tx0(); //Send a 0
        //Serial.print("0");
      }
    }//end of checking a byte, 1 bit a a time bit 4 rotated right to bit 3
    //Serial.print("    ");
  }//end of the manchester[bytePointer]
  //Serial.println();
  digitalWrite(TxPin,0);  //End packet on Signal Off
  digitalWrite(ledPin,LOW); //just for debugging
}

//The definitions that follow are used by numerous authors (e.g., Stallings) 
//See www.wikipedia.com for Manchester encoding strategies.
void Tx0(){
  digitalWrite(TxPin,0);//put in a 'low' here so the next transition makes sense
  delayMicroseconds(sDelay);
  digitalWrite(TxPin,1);//transition from low to a high indicates a 1 sent
  delayMicroseconds(sDelay);
}

void Tx1(){
  digitalWrite(TxPin,1);//put in a 'high' here so the next transition makes sense
  delayMicroseconds(sDelay);
  digitalWrite(TxPin,0);//transition from high to a low indicates a 0 sent
  delayMicroseconds(sDelay);
}

void ValidCS(int CSPos){
  byte cs = 0;
  for (int x=1; x<CSPos; x++){
    cs +=nyb(x);
  }
  //swap the nibbles in the checksum
  manchester[9] = (cs<<4)+(cs/16);
}

// Get a nybble from manchester bytes, short name so equations elsewhere are neater :-)
byte nyb(int nybble){
  int bite  = nybble / 2;       //DIV 2, find the byte
  int nybb  = nybble % 2;       //MOD 2  0=MSB 1=LSB
  byte b    = manchester[bite];
  if (nybb == 0){
    b = (byte)((byte)(b) >> 4);
  }
  else{
    b = (byte)((byte)(b) & (byte)(0xf));
  }       
  return b;
}

//handy for any debugging
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

boolean getTemperature(){
  byte i;
  byte present = 0;
  //byte data[12];
  byte addr[8];
  //find a device
  int junk = ds.search(addr);
  /*
  //Not required if only one sensor to be found
   if (!ds.search(addr)) {
   ds.reset_search();
   Serial.println("Bad Search");
   return false;
   }
   */
  if (OneWire::crc8( addr, 7) != addr[7]) {
    Serial.println("Bad CRC");
    return false;
  }
  if ((addr[0] != DS18S20_ID) && (addr[0] != DS18B20_ID)) {
    Serial.println("Bad ID");
    return false;
  }
  ds.reset();
  ds.select(addr);
  // Start conversion
  ds.write(0x44, 1);
  // Wait some time...
  delay(850);
  present = ds.reset();
  ds.select(addr);
  // Issue Read scratchpad command
  ds.write(0xBE);
  // Receive 9 bytes
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  // Calculate temperature value
  temp = float( (data[1] << 8) + data[0] )/16;
  return true;
}









