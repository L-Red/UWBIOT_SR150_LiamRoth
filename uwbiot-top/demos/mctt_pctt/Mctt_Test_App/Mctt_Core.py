# Copyright 2022 NXP
#
# NXP Confidential. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

import os
from threading import Condition, Thread, Lock
from time import sleep
import sys
import queue
import serial
from serial import Serial
import signal
#from enum import IntFlag
import struct
import threading
import logging
import hashlib
# import serial
import time
import math
import matplotlib.pyplot as plt
import numpy as np
from matplotlib import animation
from pprint import pprint
import csv

BAUDRATE = 3000000
# doc:start:comports
COMPORT = os.getenv('UWBIOT_ENV_COM', 'COM122')
# doc:end:comport

HEADER_LEN = 4
UCI_CMD = 0x01
RESET_CMD = 0x04

# sessionInfo to hold the sessionId or sessionHandle
sessionInfo = [0x00, 0x00, 0x00, 0x00]
sessionId = [0x00, 0x00, 0x00, 0x00]


RANGING_STATUS_INDEX = 27
RANGING_AOA_INDEX = 31
RANGING_DISTANCE_INDEX = 29
RANGING_LOS_INDEX = 28

def DEBUG(*args):
    # print(args)
    pass

try:
    from enum import IntFlag
except ImportError:
    class IntFlag(object):
        pass

rangingOffset = 0  # to be updated when the two ranging devices are at "0"cm

class RangingUtil:

    def __init__(self):
        self.distance = [0]
        self.aoa = [0]
        self.fom = [0]
        self.nlos = [0]
        self.ntf_cnt = [0]
        self.idx = 0
        self.isRanging = False

    def __animate(self, i, fig, ax, dx):
        ax.clear()
        ax.set_title('AOA Visualization')
        ax.set_theta_zero_location('N')
        ax.set_theta_direction('clockwise')
        ax.set_xticks(np.array([-90, -45, 0, 45, 90]) / 180 * np.pi)
        ax.set_thetalim(-1 / 2 * np.pi, 1 / 2 * np.pi)
        dx.clear()
        dx.set_xlabel('Notifications')
        dx.set_ylabel('Distance')
        dx.set_title('Distance Visualization')
        d, a = 0, 0
        if self.distance:
            a = self.aoa[len(self.ntf_cnt) - 1]
            d = self.distance[len(self.ntf_cnt) - 1]
            f = self.fom[len(self.ntf_cnt) - 1]
            n = self.nlos[len(self.ntf_cnt) - 1]

            ax.arrow(0,
                     0,
                     a / 180. * np.pi,
                     1,
                     alpha=0.5,
                     width=0.015,
                     edgecolor='blue',
                     facecolor='red',
                     lw=2,
                     zorder=5)

            dx.plot(self.ntf_cnt, self.distance, 'g', label='Distance')

            for txt in fig.texts:
                txt.set_visible(False)

            fig.text(0.1,
                     0.35,
                     'Distance : {}'.format(d),
                     fontsize=14,
                     color='black')  # (0,0) is bottom left
            fig.text(0.1,
                     0.3,
                     'Angle     : {}'.format(a),
                     fontsize=14,
                     color='black')
            fig.text(0.1,
                     0.25,
                     'NLOS     : {}'.format(n),
                     fontsize=14,
                     color='black')
            fig.text(0.1,
                     0.2,
                     'FOM       : {}'.format(f),
                     fontsize=14,
                     color='black')

    def set_rng_data(self, distance, aoa, nlos):
        self.idx += 1

        with threading.Lock():
            self.distance.append(distance)
            self.aoa.append(aoa)
            self.fom.append(0)
            self.nlos.append(nlos)
            self.ntf_cnt.append(self.idx)

    def start_plotter(self):
        #_thread.start_new_thread(self.collect_rng_data, (timeout))
        while not self.isRanging:
            continue
        fig = plt.figure()
        fig.canvas.set_window_title('Rhodes Ranging Demo - Target')
        ax = fig.add_subplot(222, polar=True)
        dx = fig.add_subplot(221)
        self.ani = animation.FuncAnimation(fig,
                                           self.__animate,
                                           fargs=(
                                               fig,
                                               ax,
                                               dx,
                                           ),
                                           interval=500)
        try:
            plt.show()
            plt.close('all')
        except:
            pass


def extract_angle(bytelist):
    return int(bytelist[RANGING_AOA_INDEX + 3] << (8 * 3) | \
        bytelist[RANGING_AOA_INDEX + 2] << (8 * 2) | \
        bytelist[RANGING_AOA_INDEX + 1] << (8 * 1) | \
        bytelist[RANGING_AOA_INDEX] << (8 * 0))


def extract_distance(bytelist):
    return int((bytelist[RANGING_DISTANCE_INDEX + 1] << 8) +
               bytelist[RANGING_DISTANCE_INDEX])


def twos_comp(val, bits):
    """compute the 2's complement of int value val"""
    if (val & (1 <<
               (bits - 1))) != 0:  # if sign bit is set e.g., 8bit: 128-255
        val = val - (1 << bits)  # compute negative value
    return val


def convertQFormatToFloat(qIn, nInts, nFracs, roundof=2):
    intPart = (qIn >> nFracs)
    fracPart = qIn & ((1 << nFracs) - 1)
    fracPart = math.pow(2, -nFracs) * fracPart
    return round(intPart +
                 fracPart, roundof) if roundof else intPart + fracPart


log = logging.getLogger(__name__)
mutex = threading.Lock()

GPIO_CB_EVENT = threading.Event()

serialPort = serial.Serial()
rng_util = RangingUtil()


def AddCommandToQueue(uci_command):
    global commandQueue
    commandQueue.put(uci_command)
    # print ("Added to Queue :", end =" "), print (''.join('{:02x} '.format(x) for x in uci_command))

def WriteToSerialPort():
    print("Write to Serial Started")
    global serialPort
    global commandQueue
    global stopWriteThread
    global sessionInfo
    global sessionId
    while not commandQueue.empty() and not stopWriteThread:
        DEBUG("commandQueue not Empty")

        # Search in each command in the queue to replce Session ID with Session handle
        for command in commandQueue.queue:
            if (command[0] == 0x21 or command[0] == 0x22 or command[0] == 0x2F):
                # Check if the Command contains SessionId, then replace it with sessionHandle
                if(command[4:8] == sessionId):
                    for x in range(4):
                        command[HEADER_LEN + x]= sessionInfo[x]

        uci_command = commandQueue.get()
        usb_out_packet = bytearray()
        usb_out_packet.extend(uci_command)
        DEBUG(list(usb_out_packet))
        global retryCmd
        global writeLock
        global isChainedPkt
        print("\nNXPUCIX => ",
              ''.join('{:02x} '.format(x) for x in uci_command))
        if serialPort.isOpen():
            serialPort.write(serial.to_bytes(usb_out_packet))
        if usb_out_packet[0] == 0x3D:
            isChainedPkt = True
            continue
        writeLock.acquire()
        writeLock.wait()
        writeLock.release()
        if retryCmd == True:
            print("\nNXPUCIX => Retry ",
                  ''.join('{:02x} '.format(x) for x in uci_command))
            if serialPort.isOpen():
                serialPort.write(serial.to_bytes(usb_out_packet))
            retryCmd = False
            writeLock.acquire()
            writeLock.wait()
            writeLock.release()

    print("Write Exited")


def HardResetSR040():
    if not serialPort.isOpen():
        DEBUG("Serial Port is not open")
        return
    hardResetCmd = [RESET_CMD, 0x00, 0x00]
    serialPort.write(serial.to_bytes(hardResetCmd))
    commandResponse = serialPort.read(
        4)  # RESET_CMD response = [ 0x01, 0x02, 0x03, 0x04 ]
    DEBUG(list(commandResponse))
    hardResetNotification = serialPort.read(5)  # HARD_RESET notification
    DEBUG(list(hardResetNotification))


def ReadFromSerialPort():
    global serialPort
    global stopReadThread
    global sessionInfo
    averaging_window_size = 5
    measured_angle = 0
    measured_distance = 0
    angle_hist = []
    distance_hist = []

    while not stopReadThread:
        global isChainedPkt
        global writeLock
        if serialPort.isOpen():
            DEBUG("Reading")
            uci_hdr = serialPort.read(4)
            if (len(uci_hdr) != 4):
                continue
            DEBUG("Header read. Continuing")
            DEBUG(list(uci_hdr))
            print("\nNXPUCIR <= ",
                  ''.join('{:02x} '.format(x) for x in uci_hdr),
                  end="")
            DEBUG(len(uci_hdr))
            DEBUG(uci_hdr[3])
            # if ((uci_hdr[0] & 0xF0) != 0x40) and ((uci_hdr[0] & 0xF0) != 0x60):
            #     # Other response, continue
            #     print("other response")
            #     continue
            if len(uci_hdr) > 0:
                if uci_hdr[0] == 0xa0:
                    count = uci_hdr[2]
                else:
                    count = uci_hdr[3]
                uci_payload = serialPort.read(count)
                DEBUG("Payload read. Continuing")
                DEBUG(list(uci_payload))
                if len(uci_payload) == count:
                    print(''.join('{:02x} '.format(x) for x in uci_payload))
                    if (uci_hdr[0] == 0x62 and uci_hdr[1] == 0x00
                            and uci_hdr[2] == 0x00 and uci_hdr[3] == 0x3D):
                        if (uci_payload[RANGING_STATUS_INDEX] != 0x00):
                            print("***** Ranging Error Detected ****")
                        else:
                            print("***** Ranging Detected ****")
                            measured_angle = convertQFormatToFloat(
                                twos_comp(extract_angle(uci_payload), 16), 9,
                                7)
                            measured_distance = (
                                extract_distance(uci_payload) - rangingOffset)

                            if (measured_distance == 0xFFFF):
                                print("*********")
                            else:
                                distance_hist.append(measured_distance)
                                angle_hist.append(measured_angle)

                                if len(distance_hist) > averaging_window_size:
                                    del distance_hist[0]
                                if len(angle_hist) > averaging_window_size:
                                    del angle_hist[0]

                                sum_distance = 0
                                sum_angle = 0

                                for i in range(len(angle_hist)):
                                    sum_distance += distance_hist[i]
                                    sum_angle += angle_hist[i]
                                avg_angle = sum_angle / len(angle_hist)
                                avg_distance = sum_distance / len(
                                    distance_hist)
                                if (uci_payload[RANGING_LOS_INDEX] == 0):
                                    print(
                                        "ToF:%d - AoA:%f ; avg_ToF:%d - avg_AoA:%f LoS"
                                        % (measured_distance, measured_angle,
                                           avg_distance, avg_angle))
                                else:
                                    print(
                                        "ToF:%d - AoA:%f ; avg_ToF:%d - avg_AoA:%f NLoS"
                                        % (measured_distance, measured_angle,
                                           avg_distance, avg_angle))
                                rng_util.set_rng_data(
                                    measured_distance, measured_angle,
                                    uci_payload[RANGING_LOS_INDEX])
                        rng_util.isRanging = True

                    elif (uci_hdr[0] == 0x41 and uci_hdr[1] == 0x00 and uci_hdr[3] == 0x05):
                        # Copy the session handle by skipping the status in the session_init response.
                        sessionInfo = uci_payload[1:]

                    elif (uci_hdr[0] == 0x6E and uci_hdr[1] == 0x00):
                        pass
                    global retryCmd
                    global boardVarientLock
                    if uci_hdr[0] == 0x60 and uci_hdr[
                            1] == 0x07 and uci_payload[0] == 0x0A:
                        retryCmd = True
                        writeLock.acquire()
                        writeLock.notify()
                        writeLock.release()
                    elif (uci_hdr[0] & 0xF0) == 0x40:
                        retryCmd = False
                        sleep(0.02)  #for the case response returns too fast
                        writeLock.acquire()
                        writeLock.notify()
                        writeLock.release()
                    elif uci_hdr[0] == 0x60 and uci_hdr[1] == 0x01:
                        # Reset notification
                        retryCmd = False
                        sleep(0.1)
                        writeLock.acquire()
                        writeLock.notify()
                        writeLock.release()
                    elif uci_hdr[0] == 0x6E and uci_hdr[1] == 0x00:
                        # Logging
                        retryCmd = False
                        sleep(0.1)
                        writeLock.acquire()
                        writeLock.notify()
                        writeLock.release()
                    elif uci_hdr[0] == 0xA0:
                        # Logging
                        retryCmd = False
                        sleep(0.1)
                        writeLock.acquire()
                        writeLock.notify()
                        writeLock.release()
                    elif (uci_hdr[0] == 0xff and uci_hdr[1] == 0xff and uci_hdr[2] == 0xff):
                        print("*****WRAPED RDS IS Failed  ****")
                        retryCmd = False
                        sleep(0.02)  #for the case response returns too fast
                        writeLock.acquire()
                        writeLock.notify()
                        writeLock.release()

                else:
                    print("\nExpected Payload Bytes is " + count +
                          ", Actual Paylod Bytes Recieved is : " +
                          len(uci_payload))
            else:
                if (uci_hdr[0] == 0x6E and uci_hdr[1] == 0x21):
                    # Stop Test notification
                    pnp_uci_logger.printTestStats()
                else:
                    print(
                        "\nUCI Payload Size is Zero or UCI Header Count is zero. Skipping"
                    )

        else:
            print("Port is not opened1")

    writeLock.acquire()
    writeLock.notify()
    writeLock.release()
    if waitForResponse.locked():
        waitForResponse.release()
        print("released write waitForResponse")
    print("Aborted Reader Thread")


class SIGINT_handler():

    def __init__(self):
        self.SIGINT = False

    def signal_handler(self, signal, frame):
        print('You pressed Ctrl+C!')
        global stopReadThread
        global stopWriteThread
        global serialPort
        try:
            stopReadThread = True
            stopWriteThread = True
            plt.close('all')
            rng_util.kill()
            if serialPort.isOpen(): serialPort.close()
            self.SIGINT = True
        except:
            sys.exit(0)


def Configure():
    global serialPort
    serialPort.baudrate = BAUDRATE
    serialPort.timeout = 1
    serialPort.port = COMPORT
    serialPort.stopbits = 1
    serialPort.parity = serialPort.PARITIES[0]
    serialPort.bytesize = 8
    if serialPort.isOpen(): serialPort.close()
    serialPort.open()
    global stopReadThread
    stopReadThread = False
    global stopWriteThread
    stopWriteThread = False


def StartCommandProcessing():
    global serialPort
    try:
        handler = SIGINT_handler()
        signal.signal(signal.SIGINT, handler.signal_handler)
        signal.signal(signal.SIGTERM, handler.signal_handler)

        #HardResetSR040()

        ReaderThread = Thread(target=ReadFromSerialPort, args=())
        ReaderThread.start()
        WriterThread = Thread(target=WriteToSerialPort, args=())
        WriterThread.start()
        rng_util.start_plotter()
    except KeyboardInterrupt:
        print("Caught KeyboardInterrupt, terminating workers")
        pass


serialPort = serial.Serial()
commandQueue = queue.Queue(maxsize=100)
stopReadThread = False
stopWriteThread = False
waitForResponse = Lock()
boardVarientLock = Condition()
writeLock = Condition()
retryCmd = False
isChainedPkt = False
ntfCount = 0


def run(SR040_Command_List, session_id):
    global sessionId
    global sessionInfo
    sessionId = sessionInfo = session_id
    print("Configure Started")
    Configure()
    print("Configure Completed")
    UwbMode = SR040_Command_List

    # Add the UCI Commands to be sent for Evaluating Helios
    print("AddCommandToQueue started")
    for x in range(len(UwbMode)):
        AddCommandToQueue(UwbMode[x])

    # AddCommandToQueue(UCI_StopRanging)
    print("AddCommandToQueue Completed")

    # Do not Edit Below Section
    print("CommandProcessing Started")
    StartCommandProcessing()
    print("CommandProcessing Completed")
