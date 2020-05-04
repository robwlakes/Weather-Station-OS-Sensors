# ChiliPeppr and GRBL V0.1

## Overview

Milling PCB patterns into blank copper laminated boards is now a popular and effective alternative to using etch resistant technologies and etch baths. Arguably it is possible to get incredibly good results from photo quality design films and with good etching tanks, with very reliable results. However milling the track's pattern into the copper is now affordable thanks to sensibly priced Chinese engravers, and more importantly, now has the software back up, making the tool chains to produce boards this way very effective. Milling also has the added benefit of also allowing holes to be drilled and boards to be shaped.

## Document

This document has been prepared to assist newcomers to PCB milling, indeed any sort of milling. I would recommend uit as place for beginner to start. The advice applies to the one of the most popular CNC setups, an Arduino Uno and a Protoneer board working with ChiliPeppr. It is a distillation of advice provided driectly from the people at the ChiliPeppr site, so I would like to acknowledge and thank those people for their support. There are numerous other sources across the Internet that I am indebted to for small chunks of insight into what I was tryingto achieve. I was unable to find a publication that was definitive enough to rely completely on it to figure my problems. By the time I got my system working there was actually only a fairly small amounts of information required, so I thought I could contribute a beginners manual in return for the fantastic software I have been able to use.
  
## Supported software and hardware

The following hardware was used:

A Chinese engraving machine (about $250 Australian)
  * 3 Nema 17 style motors, 3 8mm 2 thread lead screws with flexible connectors and end thrust bearings.
  * 12-24V Spindle DC Motor
  * 20-20 Aluminium frame with bolts, motor mounts and right angle connectors
  * 200mmx120mm Aluminium bed
  * An Arduino Uno V3 (GRBL Pre-loaded, but old version)
  * A Protoneer CNC Arduino shield V3.0
  * Plus other bits and pieces to make it work. eg Backlash nuts
  * 1 240V-12V 1A laptop style power supply
  * 1 240V-24V 1A laptop power supply (not used).
  * Plus...
    * 0.1mm 30degree engraving tools
    * 0.9mm drill bits
    * 1 5mm Diam CNC tool chuck with collets
    * 2mm milling bit
    * Any extra hardware is detailed in the document eg wires and switches etc
  
The following software:
  * Eagle PCB software (used within the licence for free, )
  * GRBL 1.1h
  * Serial Port json-server 1.96
  * ChiliPeppr Workspace (www.chilipeppr/workspace/jpadier/)
  * Arduino IDE arduino-1.8.4
  * Chrome Web Browser (Version 81.0.4044.122)
  * Ubuntu Mate 18.04

## Skills

It is assumed readers will be familiar with Eagle software to design their PCBs. This is very popular software and there are many YouTube tutorials on how to use it. If you are debugging you system, I strongly advise to create a very small design eg two resistors joined end to end to begin with. Unless you have a lot of previous experience (unlike me) don't launch into a large double sided board first up. Take time to tune up and learn the strengths and limitations of the machine.

I initially found that this project was really daunting, as there were so many gaps in my knowledge I was not certain I would ever get it to work. This document is to inspire you and empower you. I reckon for someone like me, this was an ideal way to get know how to run a small milling tool, albeit it is really only an engraver at heart.

## Troubleshooting

The engraving machine can be used for light weight milling jobs. However it is essentially an engraving machine with some milling capacity.  The way the Spindle is mounted on the Z-Axis and that is mounted on the X-Axis projects the milling tool quite a wayout in front of the two X-Axis gantry rods.  This gives considerable leaverage on that system and not being able to gain enough force to ensure the milling tools are held firmly against the object to guarantee the cut will be as accurate as possible.
      
Fortunately for PCBs the engraving tools need to be the most accurate, and the pressure required for success here is much smaller. The holes drilling and outline milling are all through the thickness of the board so just reducing the final milling the shape of the board with a 2mm bit can be done in 0.2mm steps rather than say 0.5mm steps. It takes longer but less likely to snap a tool or force an inaccurate cut.

Working out the Auto Level in ChiliPeppr also meant figuring out how to use the A5Pin Probe in GRBL and how to minimise false limit triggering from random RF pulses.

As always take care, and take responsibility for all your own actions in following this advice, it is offered as is and no responsibility will be taken by me for any damages incurred personally or equipment by readers.Build your skills gradually by testing everything thoroughly at each stage. 
       

Rob Ward
ACV (ante Corona Virus)
May 4th 2020
