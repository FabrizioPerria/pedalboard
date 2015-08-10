
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

import socket
import sys
import rtmidi_python as rtmidi
import numpy as np
import time
import os
import re
import sys
import threading

try:
    from Tkinter import *
except ImportError:
    from tkinter import *

try:
    import ttk
    py3 = 0
except ImportError:
    import tkinter.ttk as ttk
    py3 = 1

def vp_start_gui():
    '''Starting point when module is the main routine.'''
    global val, w, root
    root = Tk()
    root.title('Jack_Pedalboard')
    geom = "400x240+650+150"
    root.geometry(geom)
    root.protocol('WM_DELETE_WINDOW', destroy_Jack_Pedalboard)
    w = Jack_Pedalboard (root)
    root.mainloop()

def destroy_Jack_Pedalboard():
    global w, midiOut
    w.closeThread()
    if 'midiOut' in globals():
        midiOut.close_port()
    root.destroy()
    w = None

class ComThread(threading.Thread):
     def __init__(self):
         super(ComThread, self).__init__()

     def close(self):
         if self.keepRunning:
             global sock, BOARD_UDP_IP, BOARD_UDP_PORT, BUFLEN
             sock.sendto("ending",(BOARD_UDP_IP,BOARD_UDP_PORT))
             self.keepRunning = False   

     def run(self):
         global sock, midiOut
         self.keepRunning = True
         sock.settimeout(None)
         while self.keepRunning == True:
            try: 
                data,addr = sock.recvfrom(BUFLEN);
                #print "received message " ,data ," from " ,addr
                if(data == "ending"):
                    break
                if(data == "start"):
                    #start a program
                    if sys.platform == "win32":
                        os.startfile("C:\Program Files (x86)\Peavey Electronics\ReValver Mk IIIdotV\ReValverMkIIIdotV.exe");
                elif(data == "stop"):
                    #stop a program
                    if sys.platform == "win32":
                        os.system("TASKKILL /F /IM ReValverMkIIIdotV.exe")
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
                    midiOut.send_message([0xB0,0x04,int(data)%127])
                else:
                    print ("Message not Recognized")
            except:
                print ("Thread Error!!!!!!",sys.exc_info())

class Jack_Pedalboard:
    def __init__(self, master=None):
        _bgcolor = '#d9d9d9'  # X11 color: 'gray85'
        _fgcolor = '#000000'  # X11 color: 'black'
        _compcolor = '#d9d9d9' # X11 color: 'gray85'
        _ana1color = '#d9d9d9' # X11 color: 'gray85' 
        _ana2color = '#d9d9d9' # X11 color: 'gray85' 
        self.style = ttk.Style()

        self.style.configure('.',background=_bgcolor)
        self.style.configure('.',foreground=_fgcolor)
        self.style.configure('.',font="TkDefaultFont")
        self.style.map('.',background=
            [('selected', _compcolor), ('active',_ana2color)])
        master.configure(background="#d9d9d9")


        self.Frame1 = Frame(master)
        self.Frame1.place(relx=0.01, rely=0.01, relheight=0.79, relwidth=0.98)
        self.Frame1.configure(relief=RAISED)
        self.Frame1.configure(borderwidth="4")
        self.Frame1.configure(relief=RAISED)
        self.Frame1.configure(background=_bgcolor)
        self.Frame1.configure(width=595)

        self.IPEntry = Entry(self.Frame1)
        self.IPEntry.place(relx=0.14, rely=0.16, relheight=0.13, relwidth=0.78)
        self.IPEntry.configure(background="white")
        self.IPEntry.configure(disabledforeground="#a3a3a3")
        self.IPEntry.configure(font="TkFixedFont")
        self.IPEntry.configure(foreground="#000000")
        self.IPEntry.configure(insertbackground="black")
        self.IPEntry.configure(width=304)
        self.IPEntry.configure(text="192.168.42.1")

        self.Label1 = Label(self.Frame1)
        self.Label1.place(relx=0.08, rely=0.16, height=26, width=21)
        self.Label1.configure(background=_bgcolor)
        self.Label1.configure(disabledforeground="#a3a3a3")
        self.Label1.configure(foreground="#000000")
        self.Label1.configure(text='''IP:''')

        self.Label2 = Label(self.Frame1)
        self.Label2.place(relx=0.05, rely=0.42, height=26, width=36)
        self.Label2.configure(background=_bgcolor)
        self.Label2.configure(disabledforeground="#a3a3a3")
        self.Label2.configure(foreground="#000000")
        self.Label2.configure(text='''Port:''')

        self.PortEntry = Entry(self.Frame1)
        self.PortEntry.place(relx=0.15, rely=0.42, relheight=0.13, relwidth=0.78)
        self.PortEntry.configure(background="white")
        self.PortEntry.configure(disabledforeground="#a3a3a3")
        self.PortEntry.configure(font="TkFixedFont")
        self.PortEntry.configure(foreground="#000000")
        self.PortEntry.configure(insertbackground="black")
        self.PortEntry.configure(width=304)
        self.PortEntry.configure(text="9999")

        self.MidiList = ttk.Combobox(self.Frame1)
        self.MidiList.place(relx=0.15, rely=0.68, relheight=0.14, relwidth=0.79)
        self.MidiList.configure(width=307)
        self.MidiList.configure(takefocus="")
        self.midiOut = rtmidi.MidiOut()
        self.MidiList["value"] = ["None"] + self.midiOut.ports
        self.MidiList.current(0)
        
        self.Label3 = Label(self.Frame1)
        self.Label3.place(relx=0.04, rely=0.68, height=26, width=38)
        self.Label3.configure(background=_bgcolor)
        self.Label3.configure(disabledforeground="#a3a3a3")
        self.Label3.configure(foreground="#000000")
        self.Label3.configure(text='''MIDI''')

        self.connectedLabel = Label(master)
        self.connectedLabel.place(relx=0.03, rely=0.88, height=26, width=106)
        self.connectedLabel.configure(background=_bgcolor)
        self.connectedLabel.configure(disabledforeground="#a3a3a3")
        self.connectedLabel.configure(foreground="red")
        self.connectedLabel.configure(text='''Not Connected''')

        self.ConnectButton = Button(master)
        self.ConnectButton.place(relx=0.73, rely=0.83, height=33, width=106)
        self.ConnectButton.configure(activebackground="#d9d9d9")
        self.ConnectButton.configure(activeforeground="#000000")
        self.ConnectButton.configure(background=_bgcolor)
        self.ConnectButton.configure(disabledforeground="#a3a3a3")
        self.ConnectButton.configure(foreground="#000000")
        self.ConnectButton.configure(highlightbackground="#d9d9d9")
        self.ConnectButton.configure(highlightcolor="black")
        self.ConnectButton.configure(pady="0")
        self.ConnectButton.configure(text='''Connect''')
        self.ConnectButton.configure(width=106)
        self.ConnectButton["command"] = self.connect

    def checkIP(self):
        # Check the IP format
        pattern = re.compile("^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$")
        if pattern.match(self.IPEntry.get()):
            return True
        else:
            return False

    def checkPort(self):
        #Check the format of the UDP port (1 to 5 only-digits string)
        pattern = re.compile("^\d{1,5}$")
        if pattern.match(self.PortEntry.get()):
            return True
        else:
            return False

    def closeThread(self):
        #if a thread exists, close it!
        if hasattr(self, 'thread'):
            self.thread.close()
            

    def connect(self):
        if self.ConnectButton['text'] == "Connect":
            if self.checkIP() == False:
                self.IPEntry['bg'] = "red"
            elif self.checkPort() == False:
                self.IPEntry['bg'] = "white"
                self.PortEntry['bg'] = "red"
            else:
                self.IPEntry['bg'] = "white"
                self.PortEntry['bg'] = "white"
                try:
                    global sock, midiOut, BOARD_UDP_IP, BOARD_UDP_PORT, BUFLEN
                    
                    #Setup the Midi port
                    
                    midiOut = rtmidi.MidiOut()
                    if self.MidiList.get() == "None":
                        if sys.platform != "win32":
                            midiOut.open_virtual_port("JackPort")
                            self.MidiList.set("JackPort")
                        else:
                            self.MidiList.current(0)
                            midiOut.open_port(0)
                    else:
                        midiOut.open_port(self.MidiList.current()-1)

                    # Setup the UDP connection
                    socket.inet_aton(self.IPEntry.get())
                    
                    BOARD_UDP_IP = self.IPEntry.get()
                    BOARD_UDP_PORT = int(self.PortEntry.get())
                    # Message useed for the handshake between PC and device
                    # .......Silly, but it's just a message......
                    MSG = "Hi!"
                    BUFLEN = 10
                    
                    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                    sock.bind(("",0))
                    
                    sock.sendto(MSG,(BOARD_UDP_IP,BOARD_UDP_PORT))
                    sock.settimeout(2)
                    data,addr = sock.recvfrom(BUFLEN)
                    
                    if data != MSG:
                        # Who are you??
                        print ("wrong handshake")
                        sys.exit(1)

                    self.thread = ComThread();
                    self.thread.start()
                    self.ConnectButton['text'] = "Disconnect"

                    self.connectedLabel['fg'] = "green"
                    self.connectedLabel['text'] = "Connected"
                    self.MidiList['state'] = DISABLED
                    self.IPEntry['state'] = DISABLED
                    self.PortEntry['state'] = DISABLED
                except socket.timeout:
                    print "Connection Timeout"
                except:
                    print ("Error!!!!!!",sys.exc_info())

        else:
            #Disconnecting the application from the device
            self.closeThread()
            self.MidiList['state'] = NORMAL
            self.IPEntry['state'] = NORMAL
            self.PortEntry['state'] = NORMAL
            self.ConnectButton['text'] = "Connect"

            self.connectedLabel['fg'] = "red"
            self.connectedLabel['text'] = "Not Connected"

if __name__ == '__main__':
    vp_start_gui()
