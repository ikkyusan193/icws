import socket as sk
from time import sleep

with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
    # (host: str, port: int)
    s.connect(('cs.muic.mahidol.ac.th', 80))
    s.sendall(b'GET ')
    # case 1 wrong method 
    # - s.sendall(b'HELLO ')
    #sleep(1)
    s.sendall(b'/ HTTP/1.1\r')
    # case 2 wrong version
    # - s.sendall(b'/ HTTP/2.1\r')
    # case 3 carriage return error
    # - s.sendall(b'\r\n')
    s.sendall(b'\n')
    # case 4 gramatically wrong header
    # case 5 duplicate header
    # case 6 wrong uri
    # case 7 wrong mimetype
    s.sendall(b'Host: cs.muic.mahidol.ac.th\r\nConnection: close\r\n')
    #sleep(5)
    s.sendall(b'\r\n')

    while data := s.recv(1024):
        print(data) 
