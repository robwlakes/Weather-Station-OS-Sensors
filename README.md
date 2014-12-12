*Scientific Weather station WMR86 using a 433MHx receiver and an Arduino Uno.*

The concept is presented in three stages where the first program simply captures and displays samples of the sensors' raw
packet data from the sensors.  The second version further decodes the OS protocol and captures the three main types of
packets (rainfall data, wind data and temp/humidity data).  The program recognises the different packet sizes, uses
a checksum, and converts the data to human readable format.  The third program develops the receiver further by
collecting pressure and humidity data directly off the Ardunio and every minute sending out a serial string of characters
summarising all the sensors data.  In my case I use this data to plot information for my 24/7 web weather site.

Anybody who has worked with Manchester encoding/decoding may be interested in studying the algorithm used to
decode incoming signals.  My previous attempts had two very similar sections, with the first detecting a header
stream of one's and then the second very similar code detecting the payload of data.  I have been able to eliminate
the repetition in the code and come up with one unified algorithm.  It may be useful to adapt to other applications.

One of the drawbacks of not building one's own weather station is that using a commercial station usually means
being stuck with what ever they offer as extra sensors or functions in their consoles.  The power of the Arduino
is drawn on here to also be able to build OS style sensors.  The code to generate Manchester encoding is much
simpler than receiving it.  A program is shown here that sends off an OS protocol compatible signal and allows the
Arduino receiver to decode it quite easily, alongside the other OS signals it is getting.  It means that novel sensors
can be added to the system and expand it well beyond a simple weather station. The example provided is of a
temperature sensor.  This is not meant to be a "cheap" alternative to buying OS sensors, but the Dallas sensor
has a TO-3 form factor (ie it looks like a transistor) and can be sealed in glue and stuck in places that a normal
sized OS sensor would never fit (like the inside of the tube that goes into my solar hotwater tank!!). Note though
that any new sensor you add will need to be well planned out as these programs have not been developed exhaustively
to eliminate possible conflicts with other OS sensors.  So a fair degree of hacking and programming skill is
required to build an extra sensor that will not conflict with the OS system you have bought. Be warned!

Inventing sensors also open up other possibilities such as a lighting detector, soil moisture detector, pet food
status, solar power detector, and so on.  My favourite is to build a Bat Detector.  I have a hand held detector 
constructed and if I can get it to work locally I will have a go at extending my environmental reporting.  I do
intend to be able to do sampling of the temperature in our lake with a portable temperature recorder that can log
back in when I return home and report the lake's temperature.

The weather station sensors, along with the Arduino in control, also open a myriad of other possibilities of
intelligent switching of devices in response to weather.  For example low wind speed and low temperatures often
combine as a frost situation inland.  These conditions could be detected and the Arduino could turn on sprinklers
to save a grape harvest from frost damage.

If you have an Arduino project that looks as though weather conditions may be a factor in its success, then I can
recommend buying an Oregon Scientific weather station (I used the WMR86 protocol V3.0) as a great start.  These
programs are not just compile run and sit back, but are offered to serious experimenters who can program and build
on other people's ideas.  If you are inspired by these programs to do something exciting please let me know.
NB I receive no financial, or any other benefits from promoting Oregon Scientific equipment.

Rob

Intial upload of Arduino+433MHz Rx base station routines:
Raw (grabs Manchester encoded data, 8bytes)
Debugger (get Machester encoded data, variable packet length)
DebugVersion_16_NextStep (Decodes Oregon Scientific sensors to human readable format)
Final (gets data from Oregon Scientific Weather sensors and decodes them)
Transmitter9 (sends temp to Arduino Base using protocol similar to OS)

