# Jack Pedalboard
Jack Pedalboard is a project intended to build a guitar MIDI pedalboard 
through the use of a Raspberry Pi and a computer with an amplifier 
simulator installed which can be controlled through MIDI protocol.

The project uses the following hardware modules:
  - Raspberry Pi B+ model
  - USB 5V battery (for the raspberry pi)
  - VCNL 4000 I2C Proximity Sensor
  - SSD1306 I2C OLED display(128x64)
  - USB wifi dongle
  - switches and buttons
  - resistors
  - breadboard and wires 
  - laptop or desktop computer with wifi connection and amplifier simulator
  
# CIRCUIT
The circuit depicted in the file pedalboard_bb.png shows how the buttons, the screen and the proximity sensor are connected to the 
raspberry pi. Through the GPIOs, the raspberry can detect the pression of a button and react sending the datagram to the python server.
Moreover, through the SDA and SCL lines, the raspberry will sense the proximity of the foot and will send the commands to show the graphics 
on the display.

# RASPBERRY PI
The raspberry pi is responsible to detect the pressure of a button, read the proximity of the foot on the expression pedal, control the 
display and send datagrams to the UDP server. 
The datagrams contain very simple messages which are not part of the MIDI protocol (I used simple messages to simplify the sniffing phase 
during the debug) and this part can be optimized if there are bandwidth problems.

# SERVER 
The server is built in python code in order to keep simple the code which implements the MIDI protocol.
In order to use the python code on my laptop (which is a macbook) I used osascript to launch the signal processing application (Peavey 
Revalver 3.5); obviously you need to change this part if you have another OS and if you want to use another amp modeling software.

