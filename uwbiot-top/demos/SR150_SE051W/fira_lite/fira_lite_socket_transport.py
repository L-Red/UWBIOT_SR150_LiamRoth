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
# This file is used with demo_fl_initiator and demo_fl_responder
# Please see corresponding readme / documentation of those demos.
#

import sys
import os
import socket
import threading
from threading import Condition
import queue
import time
import ctypes
import threading
import contextlib

kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)

CTRL_C_EVENT = 0
THREAD_SET_CONTEXT = 0x0010


@contextlib.contextmanager
def ctrl_cancel_async_io(file_handle):
    apc_sync_event = threading.Event()
    hthread = kernel32.OpenThread(
        THREAD_SET_CONTEXT, False, kernel32.GetCurrentThreadId()
    )
    if not hthread:
        raise ctypes.WinError(ctypes.get_last_error())

    @ctypes.WINFUNCTYPE(None, ctypes.c_void_p)
    def apc_cancel_io(ignored):
        kernel32.CancelIo(file_handle)
        apc_sync_event.set()

    @ctypes.WINFUNCTYPE(ctypes.c_uint, ctypes.c_uint)
    def ctrl_handler(ctrl_event):
        # For a Ctrl+C cancel event, queue an async procedure call
        # to the target thread that cancels pending async I/O for
        # the given file handle.
        if ctrl_event == CTRL_C_EVENT:
            kernel32.QueueUserAPC(apc_cancel_io, hthread, None)
            # Synchronize here in case the APC was queued to the
            # main thread, else apc_cancel_io might get interrupted
            # by a KeyboardInterrupt.
            apc_sync_event.wait()
        return False  # chain to next handler

    try:
        kernel32.SetConsoleCtrlHandler(ctrl_handler, True)
        yield
    finally:
        kernel32.SetConsoleCtrlHandler(ctrl_handler, False)
        kernel32.CloseHandle(hthread)


class firaLiteSocketTransport:

    def __init__(self):
        self._initiator_alive = False
        self._responder_alive = False
        self.server_ip_addr = ""
        self.server_sock_fd = 0
        self.client_obj = 0
        self.initiator_obj = 0
        self.responder_obj = 0
        self.initiator_thread = ""
        self.responder_thread = ""
        self.reader_thread = ""
        self.initiator_queue = queue.Queue()
        self.responder_queue = queue.Queue()
        self.initiator_rd_Lock = Condition()
        self.responder_rd_Lock = Condition()
        self.initiator_rx_count = 0
        self.test_echo_single_device = 0
        self.test_echo = 0

    def create_server(self):
        self.server_ip_addr = ("", 8080)
        self.server_sock_fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_sock_fd.setsockopt(
            socket.SOL_SOCKET, socket.SO_REUSEADDR, 1
        )
        self.server_sock_fd.bind(self.server_ip_addr)
        print(
            "server started", socket.gethostbyname(socket.gethostname()),
            "Waiting for client on port 8080"
        )

    def close_socket(self):
        self.server_sock_fd.close()
        self._initiator_alive = False
        self._responder_alive = False

    def parse_termination(self, data):
        termList = [0x7C, 0x00, 0x00]
        if data == termList:
            return True
        else:
            return False

    def reader(self):
        try:
            while True:
                if self._initiator_alive:
                    data_bytes = self.initiator_obj.recv(1024)
                    if len(data_bytes) == 0:
                        pass
                    else:
                        print(
                            "\nInitiatorRX: ",
                            ''.join('{:02x} '.format(x) for x in data_bytes)
                        )
                        self.initiator_rx_count += 1
                        initCommand = list(bytes(data_bytes))
                        if self.test_echo_single_device:
                            self.initiator_queue.put(initCommand)
                        else:
                            self.responder_queue.put(initCommand)
                            self.initiator_rd_Lock.acquire()
                            self.initiator_rd_Lock.wait()
                            self.initiator_rd_Lock.release()
                        if self.parse_termination(initCommand):
                            print("Initialisation completed start ranging")
                            break
                if self._responder_alive:
                    data_bytes_rsp = self.responder_obj.recv(1024)
                    if len(data_bytes_rsp) == 0:
                        pass
                    else:
                        print(
                            "\nResponderRX: ", ''.join(
                                '{:02x} '.format(x) for x in data_bytes_rsp
                            )
                        )
                        rspCommand = list(bytes(data_bytes_rsp))
                        if self.test_echo_single_device:
                            self.responder_queue.put(rspCommand)
                        else:
                            self.initiator_queue.put(rspCommand)
                            self.responder_rd_Lock.acquire()
                            self.responder_rd_Lock.wait()
                            self.responder_rd_Lock.release()
        except:
            print('Exception')
            self._initiator_alive = False
            self._responder_alive = False
            self.close_socket()
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
                    self.initiator_obj.sendall(usb_out_packet)
                    time.sleep(1)
                    if not self.test_echo_single_device:
                        self.responder_rd_Lock.acquire()
                        self.responder_rd_Lock.notify()
                        self.responder_rd_Lock.release()
        except:
            print('Socket Exception')
            self._initiator_alive = False
            self._responder_alive = False
            self.close_socket()

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
                    self.responder_obj.sendall(usb_out_packet)
                    time.sleep(2)
                    if not self.test_echo_single_device:
                        self.initiator_rd_Lock.acquire()
                        self.initiator_rd_Lock.notify()
                        self.initiator_rd_Lock.release()
                    if self.parse_termination(command):
                        self._initiator_alive = False
                        self._responder_alive = False
        except:
            print('Exception')
            self._initiator_alive = False
            self._responder_alive = False
            self.close_socket()

    def check_client_role(self, client):
        initiator = [0xB0, 0x01, 0x01]
        responder = [0xB0, 0x00, 0x00]
        while True:
            data_bytes = client.recv(1024)
            if data_bytes:
                data = list(bytes(data_bytes))
                print(data)
                if data == initiator:
                    return True
                elif data == responder:
                    return False
                else:
                    print("Invalid client connected")
                    return -1

    def run(self):
        self.create_server()
        with ctrl_cancel_async_io(self.server_sock_fd.fileno()):
            self.server_sock_fd.listen()
            connections = 0
            waitNoConn = 2
            if self.test_echo_single_device:
                waitNoConn = 1
            while connections < waitNoConn:
                self.client_obj, address = self.server_sock_fd.accept()
                print("connected to: ", address[0], ':', str(address[1]))
                ret = self.check_client_role(self.client_obj)
                if ret == -1:
                    self.server_sock_fd.close()
                    exit(1)
                if ret == True:
                    connections += 1
                    self.initiator_obj = self.client_obj
                    print("Initializer client connected")
                    self._initiator_alive = True
                elif ret == False:
                    connections += 1
                    self.responder_obj = self.client_obj
                    print("responder client connected")
                    self._responder_alive = True
            # start
            self.reader_thread = threading.Thread(target=self.reader)
            self.reader_thread.setDaemon(True)
            self.initiator_thread = threading.Thread(target=self.initiator)
            self.responder_thread = threading.Thread(target=self.responder)
            self.initiator_thread.setDaemon(True)
            self.responder_thread.setDaemon(True)
            self.reader_thread.start()
            self.initiator_thread.start()
            self.responder_thread.start()
            if self.test_echo_single_device:
                # echo 0x7D
                command = [0x7D, 0x00, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05]
                out_packet = bytearray()
                out_packet.extend(command)
                if self._initiator_alive:
                    print(
                        "\nInitiatorTX: ",
                        ''.join('{:02x} '.format(x) for x in out_packet)
                    )
                    self.initiator_obj.sendall(out_packet)
                elif self._responder_alive:
                    print(
                        "\nResponderTX: ",
                        ''.join('{:02x} '.format(x) for x in out_packet)
                    )
                    self.responder_obj.sendall(out_packet)
            elif self.test_echo:
                command = [0x7D, 0x00, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05]
                out_packet = bytearray()
                out_packet.extend(command)
                print(
                    "\nInitiatorTX Echo command: ",
                    ''.join('{:02x} '.format(x) for x in out_packet)
                )
                self.initiator_obj.sendall(out_packet)
            else:
                # start of frame
                command = [0x0B, 0x00, 0x00]
                out_packet = bytearray()
                out_packet.extend(command)
                print(
                    "\nInitiatorTX: ",
                    ''.join('{:02x} '.format(x) for x in out_packet)
                )
                self.initiator_obj.sendall(out_packet)
            while self._initiator_alive or self._responder_alive:
                time.sleep(2)
            print("FiraLite transport done")
            self.close_socket()


if __name__ == '__main__':
    try:
        transport = firaLiteSocketTransport()
        transport.run()
    except:
        transport.close_socket()
        exit(1)
    finally:
        exit(0)
