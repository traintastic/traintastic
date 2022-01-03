#!/usr/bin/env python3

import socket
import re

ADDRESS = '127.0.0.1'
PORT = 15471

OBJECTID_ECOS = 1


def process(request):
    print('RX:\n' + request)
    m = re.match(r'^(queryObjects|set|get|create|delete|request|release)\(\s*([0-9]+)\s*(.*)\)$', request)
    if m is not None:
        command = m.group(1)
        objectId = int(m.group(2))
        args = [s for s in re.split(r'\s*,\s*', m.group(3)) if s]

        if objectId == OBJECTID_ECOS:
            if command == 'get':
                if args[0] == 'info':
                    return \
                        '<REPLY get(1, info)>\n' + \
                        '1 ECoS\n' + \
                        '1 ProtocolVersion[0.1]\n' + \
                        '1 ApplicationVersion[1.0.1]\n' + \
                        '1 HardwareVersion[1.3]\n' + \
                        '<END 0 (OK)>\n'

        return None


print('Traintastic ECoS simulator')
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((ADDRESS, PORT))
    s.listen()
    print('Listening at {:s}:{:d}'.format(ADDRESS, PORT))
    conn, addr = s.accept()
    with conn:
        print('Connection from {:s}:{:d}'.format(addr[0], addr[1]))
        while True:
            try:
                for request in conn.recv(1024).decode('utf-8').split('\n'):
                    if request != '':
                        response = process(request)
                        if response is not None:
                            print('TX:\n' + response)
                            conn.sendall(response.encode('utf-8'))
            except ConnectionError:
                print('Connection lost')
                break
