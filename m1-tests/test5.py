import socket as sk
from time import sleep

def legit1():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('netpg', 80))
        s.sendall(b'GET ')
        sleep(1)
        s.sendall(b'/ HTTP/1.1\r')
        sleep(0.5)
        s.sendall(b'\n')
        s.sendall(b'Host: cs.muic.mahidol.ac.th\r\nConnection: close\r\n')
        sleep(5)
        s.sendall(b'\r\n')
        while data := s.recv(1024):
            print(data)

def legit2():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'GET /index.html HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def legit3():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'HEAD /index.html HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def legit4():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'GET /cat.jpg HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def legit5():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'HEAD /cat.jpg HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def legit6():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'GET /index.html HTTP/1.1\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def legit7():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'HEAD /index.html HTTP/1.1\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def illegal1():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'HEAD /aaa.html HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def illegal2():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'blah /cat.jpg HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def illegal3():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'POST /index.html HTTP/1.1\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def illegal4():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'GET /index.html HTTP/99\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def illegal5():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'POST /index.aaa HTTP/99\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def illegal6():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def illegal7():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        s.connect(('localhost', 2222))
        s.sendall(b'\r\n\r\n\r\n\r\n')
        while data := s.recv(1024):
            print(data)

legit1()
print()
legit2()
print()
legit3()
print()
legit4()
print()
legit5()
print()
legit6()
print()
legit7()
print()
illegal1()
print()
illegal2()
print()
illegal3()
print()
illegal4()
print()
illegal5()
print()
illegal6()
print()
illegal7()
print()