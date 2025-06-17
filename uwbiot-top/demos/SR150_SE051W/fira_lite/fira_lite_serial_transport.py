# Copyright 2021 NXP
#
# NXP Confidential. This software is owned or controlled by NXP and may only
# be used strictly in accordance with the applicable license terms.  By
# expressly accepting such terms or by downloading, installing, activating
# and/or otherwise using the software, you are agreeing that you have read,
# and that you agree to comply with and are bound by, such license terms.  If
# you do not agree to be bound by the applicable license terms, then you may
# not retain, install, activate or otherwise use the software.
#

#
# This file is used with demo_fl_uart_initiator and demo_fl_uart_responder
# Please see corresponding readme / documentation of those demos.
#

import sys
import os
import serial
import threading
from threading import Condition
import queue
import time


class firaLiteSerialTransport:

    def __init__(self):
        self._initiator_alive = False
        self._responder_alive = False
        self.initiator_com_port = "COM50"
        self.responder_com_port = "COM47"
        self.initComObj = ""
        self.rspComObj = ""
        self.initiator_thread = ""
        self.responder_thread = ""
        self.reader_thread = ""
        self.initiator_queue = queue.Queue()
        self.responder_queue = queue.Queue()
        self.initiator_rd_Lock = Condition()
        self.responder_rd_Lock = Condition()
        self.initiator_rx_count = 0

    def open_com(self):
        self.initComObj = serial.Serial(
            self.initiator_com_port, 3000000, timeout=1
        )
        self.rspComObj = serial.Serial(
            self.responder_com_port, 3000000, timeout=1
        )
        if self.initComObj.isOpen():
            self.initComObj.close()
        self.initComObj.open()
        print(
            "initiator com port:", self.initiator_com_port,
            "opened successfully"
        )
        if self.rspComObj.isOpen():
            self.rspComObj.close()
        self.rspComObj.open()
        print(
            "responder com port:", self.responder_com_port,
            "opened successfully"
        )
        self._initiator_alive = True
        self._responder_alive = True

    def close_com(self):
        if self.initComObj.isOpen():
            self.initComObj.close()
        if self.rspComObj.isOpen():
            self.rspComObj.close()

    def parse_termination(self, data):
        termList = [0x7C, 0x00, 0x00]
        if data == termList:
            return True
        else:
            return False

    def get_com_ports(self):
        if len(sys.argv) < 3:
            self.initiator_com_port = os.getenv("UWB_IOT_INITIATOR_COM_PORT")
            self.responder_com_port = os.getenv("UWB_IOT_RESPONDER_COM_PORT")
        else:
            self.initiator_com_port = sys.argv[1]
            self.responder_com_port = sys.argv[2]
        if (self.initiator_com_port or self.responder_com_port) is None:
            print(
                "Wrong com ports given, use command: python fira_lite_serial_transport.py <Initiator Com Port> <Responder Com Port>\n"
                "eg. python fira_lite_serial_transport.py COM4 COM5\n"
                "or set environment variables UWB_IOT_INITIATOR_COM_PORT and UWB_IOT_RESPONDER_COM_PORT\n"
                "and run python fira_lite_serial_transport.py"
            )
            exit(1)

    def readCom(self):
        try:
            while self._initiator_alive and self._responder_alive:
                data_bytes = self.initComObj.read(1024)
                data = "".join(map(chr, data_bytes))
                if len(data_bytes) == 0:
                    pass
                else:
                    print(
                        "\nInitiatorRX: ",
                        ''.join('{:02x} '.format(x) for x in data_bytes)
                    )
                    self.initiator_rx_count += 1
                    initCommand = list(bytes(data_bytes))
                    self.responder_queue.put(initCommand)
                    self.initiator_rd_Lock.acquire()
                    self.initiator_rd_Lock.wait()
                    self.initiator_rd_Lock.release()
                    if self.parse_termination(initCommand):
                        print("Initialisation completed start ranging")
                        break
                data_bytes_rsp = self.rspComObj.read(1024)
                if len(data_bytes_rsp) == 0:
                    pass
                else:
                    print(
                        "\nResponderRX: ",
                        ''.join('{:02x} '.format(x) for x in data_bytes_rsp)
                    )
                    rspCommand = list(bytes(data_bytes_rsp))
                    self.initiator_queue.put(rspCommand)
                    self.responder_rd_Lock.acquire()
                    self.responder_rd_Lock.wait()
                    self.responder_rd_Lock.release()
        except serial.SerialException as e:
            print('SerialException')
            self._initiator_alive = False
            self._responder_alive = False
            raise

    def initiator(self):
        try:
            while self._initiator_alive:
                if self.initiator_queue.empty():
                    pass
                else:
                    command = self.initiator_queue.get()
                    usb_out_packet = bytearray()
                    usb_out_packet.extend(command)
                    data_bytes = list(usb_out_packet)
                    # print("\nInitiatorTX: ", ''.join('{:02x} '.format(x) for x in data_bytes))
                    self.initComObj.write(serial.to_bytes(usb_out_packet))
                    time.sleep(1)
                    self.responder_rd_Lock.acquire()
                    self.responder_rd_Lock.notify()
                    self.responder_rd_Lock.release()
        except serial.SerialException as e:
            print('SerialException')
            self._initiator_alive = False
            raise

    def responder(self):
        try:
            while self._responder_alive:
                if self.responder_queue.empty():
                    continue
                else:
                    command = self.responder_queue.get()
                    usb_out_packet = bytearray()
                    usb_out_packet.extend(command)
                    data_bytes = list(usb_out_packet)
                    # print("\nResponderTX: ", ''.join('{:02x} '.format(x) for x in data_bytes))
                    self.rspComObj.write(serial.to_bytes(usb_out_packet))
                    time.sleep(2)
                    self.initiator_rd_Lock.acquire()
                    self.initiator_rd_Lock.notify()
                    self.initiator_rd_Lock.release()
                    if self.parse_termination(command):
                        self._initiator_alive = False
                        self._responder_alive = False
        except serial.SerialException as e:
            self._responder_alive = False
            raise

    def run(self):
        self.get_com_ports()
        self.open_com()
        # start
        self._initiator_alive = True
        try:
            self.reader_thread = threading.Thread(target=self.readCom)
            self.reader_thread.setDaemon(True)
            self.initiator_thread = threading.Thread(target=self.initiator)
            self.responder_thread = threading.Thread(target=self.responder)
            self.initiator_thread.setDaemon(True)
            self.responder_thread.setDaemon(True)
            self.reader_thread.start()
            self.initiator_thread.start()
            self.responder_thread.start()
            # echo 0x7D
            #command = [0x7D, 0x00, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05]
            # start of frame
            command = [0x0B, 0x00, 0x00]
            out_packet = bytearray()
            out_packet.extend(command)
            print(
                "\nInitiatorTX: ",
                ''.join('{:02x} '.format(x) for x in out_packet)
            )
            self.initComObj.write(serial.to_bytes(out_packet))
            while self._initiator_alive and self._responder_alive:
                time.sleep(2)
                if self.initiator_rx_count == 0:
                    print("Retry start of frame")
                    print(
                        "\nInitiatorTX: ",
                        ''.join('{:02x} '.format(x) for x in out_packet)
                    )
                    self.initComObj.write(serial.to_bytes(out_packet))
            print("FiraLite transport done")
            self.close_com()
        except serial.SerialException as e:
            print('SerialException')
            raise
        except KeyboardInterrupt:
            print("ctrl-c detected, exiting...")
            exit(1)


if __name__ == '__main__':
    try:
        transport = firaLiteSerialTransport()
        transport.run()
    except serial.SerialException as e:
        print('SerialException')
        raise
