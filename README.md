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

