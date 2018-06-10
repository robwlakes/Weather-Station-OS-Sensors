Weather Sensors
===============

### Arduino Uno, 433MhzRx and Oregon Scientific
### Temperature and Humidity Sensor THGR122NX
### Adafruit Medium 16x32 RGB LED matrix panel

This project allows the 433MHz signals from an Oregon Scientific Temperature and Humidity Sensor (THGR122NX) to be intercepted, decoded and displayed on an Adafruit Medium 16x32 RGB LED matrix panel
(PRODUCT ID: 420) using an Arduino Uno. The matrix display sold by Adafruit comes with the cables and adapter shield to connect the Matrix Display board up with minimum effort (ie no Soldering!). The other real bonuis of this shield was the provision of a battery backed up real time clock.

A separate shield was added to connect the 433Mhz receiver, but if a smaller 433MHz Rx was used there is a prototyping area on the Matrix interface that accomodate the receiver as well, saving adding the extra shield. It is highly recommended to use a Super Superheterodyne Receiver to get better noise rejection and sensitivity.

The design was to show the external Temperature and Humidty for 10 seconds and then to show the Time and Date for 10 seconds, and just keep alternating at this rate.  the whole thing has been placed in the bar of our local Tavern as a talking point for our locals and tourists alike. The project does not have any manual time setting options and drift with the real time clock will become a gradual problem.  A more accurate and stable RTC could have been chosen, but the one provided on the board will have to do for the moment.  Obviously an internet or GPS derived clock would have been a better long term solution, but not one we were prepared to go to at this stage.

###The hardware

![alt text](images/TempHum.jpg?raw=true "OS Sensor and Adafruit Matrix Display")
![alt text](images/TimeDate.jpg?raw=true "OS Sensor and Adafruit Matrix Display")

The other major challenge was the multi-plexing and update of the matrix of 32x16 LEDs was done by interrupt on the Arduino Uno.  This could have complicated the use of the Oregon Scientific 433MHz reception. However as the technique used here for receiving the OS signals is based on delays, rather than interrupts, and the program sole purpose was to listen for 433MHz bursts meant all that was needed for the reception algorithm to have the delays adjusted for the time added by the display interrupts and it works just fine. Stable displays and no flickering at all.

The Arduino listens continually for the 433MHz signals on Pin11 and decodes the Manchester Protocol.  The protocol for this sensor differs from the OS WMR86 Weather Station that the polarity is reversed and the bit-rate is about half the WMR86 V3.0 frequency.  Fortunately the Oregon Scientific data packet structure is identical to the WMR86.  This initially caused considerable confusion as it was thought the THGR122NX would have the same V3 protocol as the WMR86, but with no results forthcoming, it was obvious something was different.  Changing the polarity of the Manchester decoding was tried first but no avail, and then the bit rate was adjusted and, and messages began to appear.  A little bit of fine tuning around the bit rate and processing the packets gave a steady stream of Temperature and Humidity readings. It would be great to be able elaborate on a difficult and clever analysis that revealed this required adjustment, but it was just another guess (pretty good after the initial one to get the sensor right), and only took a couple of tries and it was working!

The Temperature and Humidity Sensor THGR122NX is a nice bit of Oregon Scientific kit. They are relatively expensive, but a robust design that runs a long time on two AAA Cells.  Designing and making the low power sensor that powers a 433MHz wireless Tx for many months (years?) is normally a very hard part of this sort of project to do from scratch.  However the addition of a professional sensor to an Arduino project still has all the goodies for experimenters on the Arduino side of things. This particular sensor also has a direct LCD readout of the Temperature and Humidity on the outside and is a handy way to double check the Arduino is working properly and reporting the Temp and Hum correctly.

The other model that has been tested as well is the THGN132N Temperature and Humidity Sensor from Oregon Scientific.  It does not have the luxury of the LCD display, but only requires a single AA Battery.  A third "temperature only" model is the Oregon THN132N Thermo Sensor, which is also water proof, so probably very rugged.  However This latter model has not been tested with this software.  The OS website does mention that it is compatible with the Oregon Scientific weather stations using 2.2 Protocol sensors (Compatible With RAR813, RAR213HG, BAR218HG).  This is mentioned for all the above sensors so this last sensor is most likely compatible too.

Some variation was discovered between the two Temp/Hum sensors tested when displaying the Humidity, and up to 6-9% difference was observed. Just remember the Humidity sensors in these systems are not extremely accurate and may need some sort of conversion factor applied to correct for a particular sensor.

All the sensors discussed above have a switch in the battery compartment that allocates a station number of 1,2 or 3.  So it would be quite easy to identify up to three sensors in the program.  This has been tested, but is commented out in its present state. A mixture of the sensors could be used if desired.  

These robust and well designed units from Oregon Scientific are highly recommended, but there is no financial benefit to me if you purchase one.

For a detailed explanation of the Manchester Protocol and structure of OS data packets please read https://github.com/robwlakes/ArduinoWeatherOS 


Adafruit Medium 16x32 RGB LED matrix panel (and Arduino interface shield)
PRODUCT ID: 420
