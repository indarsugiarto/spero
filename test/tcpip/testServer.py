#!/usr/bin/env python3

import socket
import sys

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 9999        # The port used by the server

if len(sys.argv) == 1:
    print("Berikan argumen untuk dikirim")
    print("Argument stop untuk menghentikan server")
    sys.exit(-1)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    try:
        s.connect((HOST, PORT))
        msg = bytearray(sys.argv[1],encoding='utf-8')
        s.sendall(msg)
    except ConnectionRefusedError:
        print("Server-nya belum dijalankan?")
