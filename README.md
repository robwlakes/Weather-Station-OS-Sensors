***Oregon Scientific Weather station WMR86 using a 433MHx receiver and an Arduino Uno***

The concept is presented in four stages where the first program (BestRaw) simply captures and displays samples of the sensors' raw packet data from the Oregon Scientific sensors.  The second version (BestDebugger) further decodes the OS protocol and captures the three main types of packets (rainfall data, wind data and temp/humidity data).  The third program (NextStep) recognises the different packet sizes, uses a checksum, and converts the data to human readable format.  The fourth program (Final) develops the receiver further by collecting pressure and humidity data directly off the Ardunio, and every minute sending out a serial string of characters summarising all the external weather sensors' data.  In my case I use this data to plot information for my 24/7 web weather site.

**Manchester Decoding Algorithm**
Anybody who has worked with Manchester encoding/decoding may be interested in studying the algorithm used to decode incoming signals.  My previous attempts had two very similar sections, with the first detecting a header stream of one's and then the second very similar code detecting the payload of data.  I have been able to eliminate the repetition in the code and come up with one unified algorithm.  It may be useful to adapt to other applications.

**Extending the types of Sensors**
One of the drawbacks of not building one's own weather station from scratch is that using a commercial station usually means being stuck with what ever they offer as extra sensors or functions in their consoles.  The power of the Arduino is drawn on here to also be able to design your original OS style sensors.  The code to generate Manchester encoding is much simpler than receiving it.

A program is shown here that sends off an OS protocol compatible signal and allows the Arduino receiver to decode it quite easily, alongside the other OS signals it is getting.  It means that novel sensors can be added to the system and expand it well beyond a simple weather station. The example provided is a temperature sensor.  However this is not meant to be a "cheap" alternative to buying OS sensors. The main advantage is that the Dallas sensor has a TO-3 form factor (ie it looks like a transistor) and can be sealed in glue and stuck in places that a normal sized OS sensor package would never fit (like the inside of the tube that goes into my solar hotwater tank!!).

Note though that any new sensor you design will need to be well planned out as these programs here have not been developed exhaustively to eliminate possible conflicts with other OS sensors.  So a fair degree of hacking and programming skill is required to build any extra sensors that will not conflict with the OS system you have bought. In fact none of the programs use the "rolling code" concept and as a result do not eliminate the possibility of other people's neighboring OS stations being recieved (or for example, a person's setup may want to read more than one wind sensor and also be able to identify each sensor). The code offered here in many cases is the simple bare minimum, and should you want more complete features you will have to add them yourself. Be warned!

**Novel Sensors**
Inventing sensors also open up other possibilities such as a lighting detector, soil moisture detector, pet food status, solar power detector, and so on.  My favourite is to build a Bat Detector.  I have constructed a hand held detector and if I can get it to work locally I will have a go at extending my environmental reporting.  I do intend to be able to do sampling of the temperature in our lake with a portable temperature recorder that I can dunk in the water, then log it in via 433 when I return home and hence report the lake's temperature.

**Sensors plus Arduino versatility**
The weather station sensors, along with the Arduino in control, also open a myriad of other possibilities of intelligent switching of devices in response to weather.  For example low wind speed and low temperatures often combine as a frost situation inland.  These conditions could be detected and the Arduino could turn on sprinklers to save a grape harvest from frost damage.

**How to add weather sensors to your project**
If you have an Arduino project that looks as though weather conditions may be a factor in its success, then I can recommend buying an Oregon Scientific weather station (I used the WMR86 protocol V3.0) as a great start.  These programs are not just compile run and sit back, but are offered to serious experimenters who can program and build on other people's ideas.  If you are inspired by these programs to do something exciting please let me know.
NB I receive no financial, or any other benefits from promoting Oregon Scientific equipment.

Rob

Initial upload of Arduino+433MHz Rx base station routines:

Raw (grabs Manchester encoded data, 8bytes)

Debugger (get Manchester encoded data, variable packet length)

DebugVersion_16_NextStep (Decodes Oregon Scientific sensors to human readable format)

Final (gets data from Oregon Scientific Weather sensors and decodes them)

Transmitter9 (sends temp to Arduino Base using protocol similar to OS (NB Not OS compatible)


