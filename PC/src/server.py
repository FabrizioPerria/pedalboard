
#This file is part of Jack_pedalboard.

#    Jack_pedalboard is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    Jack_pedalboard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with Jack_pedalboard.  If not, see <http://www.gnu.org/licenses/>.

*/

import socket
import sys
import rtmidi_python as rtmidi
import numpy as np
import time
import os

#MIDI MESSAGES
#[a,b,c]
# a = command ---> 0x80 Note Off channel 1   |   0x90 Note On channel 1   |   0xB0 Control message
# b = note   ----> 0 to 127                  |   0 to 127                 |   0x04 Foot pedal
# c = velocity     0 to 100                  |   0 to 100                 |   value

try:
    socket.inet_aton(str(sys.argv[1]))
    
    BOARD_UDP_IP = str(sys.argv[1])
    BOARD_UDP_PORT = int(sys.argv[2])
    MSG = "Hi!"
    BUFLEN = 10

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("",0))
    if(len(sys.argv) > 3):
    	sock.sendto("Bye",(BOARD_UDP_IP,BOARD_UDP_PORT))
    	sys.exit(0)
    else:
        sock.sendto(MSG,(BOARD_UDP_IP,BOARD_UDP_PORT))

    data,addr = sock.recvfrom(BUFLEN)

    if data != MSG:
        print "wrong response"
        sys.exit(1)
    else:
    	print "Connected!"
    midiOut = rtmidi.MidiOut()
    midiOut.open_port(0)

    while True:
		data,addr = sock.recvfrom(BUFLEN);
		#print "received message " ,data ," from " ,addr
		if(data == "ending"):
			break
		if(data == "start"):
			#start a program
			os.system("open /Applications/ReValverMkIIIdotV_Live.app/")
			#os.system("osascript -e \'tell application \"ReValverMkIIIdotV_Live\" to open\'")
		elif(data == "stop"):
			#stop a program
			os.system("osascript -e \'tell application \"ReValverMkIIIdotV_Live\" to quit\'")
		elif(data == "revOn"):
			#Reverber On
			midiOut.send_message([0x90,101,100])
		elif(data == "revOff"):
			#Reverber Off
			midiOut.send_message([0x90,101,100])
		elif(data == "choOn"):
			#chorus On
			midiOut.send_message([0x90,102,100])
		elif(data == "choOff"):
			#chorus Off
			midiOut.send_message([0x90,102,100])
		elif(data == "distOn"):
			#distortion On
			midiOut.send_message([0x90,103,100])
		elif(data == "distOff"):
			#distortion Off
			midiOut.send_message([0x90,103,100])
		elif(data == "pedalOn"):
			#proximity On
			midiOut.send_message([0x90,104,100])
		elif(data == "pedalOff"):
			#proximityOff
			midiOut.send_message([0x90,104,100])
		elif(data.isdigit()):
			#proximity value
			#midiOut.send_message([0x90,int(data)%127,100])
                        midiOut.send_message([0xB0,0x04,int(data)%127])
		else:
			print "Message not Recognized"
except socket.error:
    print "Give me a correct ip address!", sys.exc_info()
except KeyboardInterrupt:
    print "CTRL-C pressed"
    sock.sendto("ending",(BOARD_UDP_IP,BOARD_UDP_PORT))
except SystemExit:
    pass
except:
	print "Error!!!!!!",sys.exc_info()
finally:
	print "Exit"
	sock.close()
	if 'midiOut' in locals():
		midiOut.close_port()
	sys.exit(0)
