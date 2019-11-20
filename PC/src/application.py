#!/usr/bin/python2.7

import socket
import sys

# import rtmidi
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

debug = False


def vp_start_gui():
    global val, w, root
    root = Tk()
    root.title('Jack_Pedalboard')
    height = '225'
    if debug:
        height = 500
    geom = "400x{}+650+150".format(height)
    root.geometry(geom)
    root.protocol('WM_DELETE_WINDOW', destroy_jack_pedalboard)
    w = Jack_Pedalboard(root)
    root.mainloop()


class AutoscrollList:
    def __init__(self, frame):
        self.listbox_log = Listbox(frame)
        self.listbox_log.place(relx=0.0, rely=0.0, relheight=1, relwidth=0.94)
        self.scrollbar_log = Scrollbar(frame)
        self.scrollbar_log.place(relx=0.95, rely=0.0, relheight=1, relwidth=0.05)

        self.scrollbar_log.pack(side=RIGHT, fill=Y)
        # self.listbox_log.pack(side=LEFT, fill=Y)

        self.listbox_log.configure(yscrollcommand=self.scrollbar_log.set)
        self.scrollbar_log.configure(command=self.listbox_log.yview)

        self.item_num = 0

    def insert(self, text, index=END):
        self.listbox_log.insert(index, text)
        self.listbox_log.select_clear(self.listbox_log.size() - 2)  # Clear the current selected item
        self.listbox_log.select_set(END)  # Select the new item
        self.listbox_log.yview(END)  # Set the scrollbar to the end of the listbox

        self.item_num += 1


def destroy_jack_pedalboard():
    global w, midiOut
    w.close_thread()
    if 'midiOut' in globals():
        midiOut.close_port()
    root.destroy()
    w = None


def clamp(min_value, x, max_value):
    return max(min_value, min(x, max_value))


class ComThread(threading.Thread):
    def __init__(self):
        super(ComThread, self).__init__()
        self.keepRunning = True

    def close(self):
        if self.keepRunning:
            global sock, BOARD_UDP_IP, BOARD_UDP_PORT, BUFLEN
            sock.sendto("reset", (BOARD_UDP_IP, BOARD_UDP_PORT))
            self.keepRunning = False

    messages_table = {
        'revOn': 101,
        'revOff': 101,
        'choOn': 102,
        'choOff': 102,
        'distOn': 103,
        'distOff': 103,
        'pedalOn': 104,
        'pedalOff': 104
    }

    def send_midi_message(self, message):
        midiOut.send_message([144, self.messages_table[message], 100])
        time.sleep(0.5)
        midiOut.send_message([128, self.messages_table[message], 100])

    def run(self):
        global sock, midiOut
        sock.settimeout(None)
        while self.keepRunning:
            try:
                data, addr = sock.recvfrom(BUFLEN)
                if debug:
                    global w
                    w.messages.insert(data)

                if data == "ending":
                    break
                if data == "start":
                    pass
                elif data == "stop":
                    pass
                elif data.isdigit():
                    a = clamp(0, int(data), 255)
                    midiOut.send_message([176, 7, a])
                elif self.messages_table.has_key(data):
                    self.send_midi_message(data)
                else:
                    print (data, "Message not Recognized")
            except:
                print ("Thread Error!!!!!!", sys.exc_info())


class Jack_Pedalboard:
    def __init__(self, master=None):
        global debug
        _bgcolor = '#d9d9d9'  # X11 color: 'gray85'
        _fgcolor = '#000000'  # X11 color: 'black'
        _compcolor = '#d9d9d9'  # X11 color: 'gray85'
        _ana1color = '#d9d9d9'  # X11 color: 'gray85'
        _ana2color = '#d9d9d9'  # X11 color: 'gray85'
        self.style = ttk.Style()

        self.style.configure('.', background=_bgcolor)
        self.style.configure('.', foreground=_fgcolor)
        self.style.configure('.', font="TkDefaultFont")
        self.style.map('.', background=[('selected', _compcolor), ('active', _ana2color)])
        master.configure(background="#d9d9d9")

        self.Frame1 = Frame(master)
        self.Frame1.place(relx=0.0, rely=0.0, relwidth=1.0)
        self.Frame1.configure(relief=RAISED)
        self.Frame1.configure(borderwidth="4")
        self.Frame1.configure(relief=RAISED)
        self.Frame1.configure(background=_bgcolor)
        self.Frame1.configure(height=225)

        self.IPEntry = Entry(self.Frame1)
        self.IPEntry.place(relx=0.15, rely=0.16, relheight=0.13, relwidth=0.78)
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
        self.MidiList.place(relx=0.15, rely=0.68, relheight=0.13, relwidth=0.78)
        self.MidiList.configure(width=307)
        self.MidiList.configure(takefocus="")
        self.midiOut = rtmidi.MidiOut()
        array = ["None"]
        for port_name in self.midiOut.ports:
            array.append(port_name)
        self.MidiList["value"] = array
        self.MidiList.current(0)

        self.Label3 = Label(self.Frame1)
        self.Label3.place(relx=0.04, rely=0.68, height=26, width=38)
        self.Label3.configure(background=_bgcolor)
        self.Label3.configure(disabledforeground="#a3a3a3")
        self.Label3.configure(foreground="#000000")
        self.Label3.configure(text='''MIDI''')

        self.connectedLabel = Label(self.Frame1)
        self.connectedLabel.place(relx=0.03, rely=0.88, height=26, width=106)
        self.connectedLabel.configure(background=_bgcolor)
        self.connectedLabel.configure(disabledforeground="#a3a3a3")
        self.connectedLabel.configure(foreground="red")
        self.connectedLabel.configure(text='''Not Connected''')

        self.ConnectButton = Button(self.Frame1)
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

        self.TurnOffButton = Button(self.Frame1)
        self.TurnOffButton.place(relx=0.37, rely=0.83, height=33, width=106)
        self.TurnOffButton.configure(activebackground="#d9d9d9")
        self.TurnOffButton.configure(activeforeground="#000000")
        self.TurnOffButton.configure(background=_bgcolor)
        self.TurnOffButton.configure(disabledforeground="#a3a3a3")
        self.TurnOffButton.configure(foreground="#000000")
        self.TurnOffButton.configure(highlightbackground="#d9d9d9")
        self.TurnOffButton.configure(highlightcolor="black")
        self.TurnOffButton.configure(pady="0")
        self.TurnOffButton.configure(text='''Turn Off''')
        self.TurnOffButton.configure(width=106)
        self.TurnOffButton["command"] = self.turn_off

        if debug:
            self.Frame2 = Frame(master)
            self.Frame2.place(relx=0.0, y=226, relwidth=1.0, height=500 - 225)
            self.Frame2.configure(relief=RAISED)
            self.Frame2.configure(borderwidth="4")
            self.Frame2.configure(relief=RAISED)
            self.Frame2.configure(background=_bgcolor)

            self.messages = AutoscrollList(self.Frame2)

    def send_message(self, message):
        socket.inet_aton(self.IPEntry.get())

        BOARD_UDP_IP = self.IPEntry.get()
        BOARD_UDP_PORT = int(self.PortEntry.get())

        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.sendto(message, (BOARD_UDP_IP, BOARD_UDP_PORT))

    def disconnect(self):
        self.send_message("reset")

    def turn_off(self):
        self.send_message("shutdown")

    def check_ip(self):
        # Check the IP format
        pattern = re.compile("^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$")
        return pattern.match(self.IPEntry.get())

    def check_port(self):
        # Check the format of the UDP port (1 to 5 only-digits string)
        pattern = re.compile("^\d{1,5}$")
        return pattern.match(self.PortEntry.get())

    def close_thread(self):
        # if a thread exists, close it!
        if hasattr(self, 'thread'):
            self.thread.close()

    def connect(self):
        if self.ConnectButton['text'] == "Connect":
            if not self.check_ip():
                self.IPEntry['bg'] = "red"
            elif not self.check_port():
                self.IPEntry['bg'] = "white"
                self.PortEntry['bg'] = "red"
            else:
                self.IPEntry['bg'] = "white"
                self.PortEntry['bg'] = "white"
                try:
                    self.setup_midi()
                    self.setup_udp_connection()

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
                    print ("Error!!!!!!", sys.exc_info())

        else:
            self.disconnect()
            # Disconnecting the application from the device
            self.close_thread()
            self.MidiList['state'] = NORMAL
            self.IPEntry['state'] = NORMAL
            self.PortEntry['state'] = NORMAL
            self.ConnectButton['text'] = "Connect"

            self.connectedLabel['fg'] = "red"
            self.connectedLabel['text'] = "Not Connected"

    def setup_udp_connection(self):
        global BOARD_UDP_IP, BOARD_UDP_PORT, BUFLEN, sock
        # Setup the UDP connection
        socket.inet_aton(self.IPEntry.get())
        BOARD_UDP_IP = self.IPEntry.get()
        BOARD_UDP_PORT = int(self.PortEntry.get())
        # Message used for the handshake between PC and device
        # .......Silly, but it's just a message......
        MSG = "Hi!"
        BUFLEN = 10
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(("", 0))
        sock.sendto(MSG, (BOARD_UDP_IP, BOARD_UDP_PORT))
        sock.settimeout(2)
        data, addr = sock.recvfrom(BUFLEN)
        if data != MSG:
            # Who are you??
            print ("wrong handshake")
            sys.exit(1)

    def setup_midi(self):
        global midiOut
        midiOut = rtmidi.MidiOut()
        if self.MidiList.get() == "None":
            if sys.platform != "win32":
                midiOut.open_virtual_port("JackPort")
                self.MidiList.set("JackPort")
            else:
                self.MidiList.current(0)
                midiOut.open_port(0)
        else:
            midiOut.open_port(self.MidiList.current() - 1)


if __name__ == '__main__':
    if 'debug' in sys.argv:
        debug = True
    vp_start_gui()
