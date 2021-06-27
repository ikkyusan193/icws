import socket as sk
from time import sleep

def test_1():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        # (host: str, port: int)
        s.connect(('cs.muic.mahidol.ac.th', 80))

        #s.sendall(b'GET / HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
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

def test_2():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        # (host: str, port: int)
        s.connect(('cs.muic.mahidol.ac.th', 80))

        #s.sendall(b'GET / HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        s.sendall(b'asdkjhaskdjawhfkjasfkjasndajndkajsndkajnwdoanskdnakwdjnaksjdnakfnaksjfdnaksjkdankwdujanskdjkaniwkdujanskdjanidukwnaksjndiawund')
        s.sendall(b'\r\n')

        while data := s.recv(1024):
            print(data) 

def test_3():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        # (host: str, port: int)
        s.connect(('cs.muic.mahidol.ac.th', 80))

        #s.sendall(b'GET / HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        s.sendall(b'G')
        sleep(1)
        s.sendall(b'E')
        sleep(1)
        s.sendall(b'T')
        sleep(1)
        s.sendall(b' ')
        sleep(1)
        s.sendall(b'/')
        sleep(1)
        s.sendall(b' ')
        sleep(1)
        s.sendall(b'H')
        sleep(1)
        s.sendall(b'T')
        sleep(1)
        s.sendall(b'T')
        sleep(1)
        s.sendall(b'P')
        sleep(1)
        s.sendall(b'/')
        sleep(1)
        s.sendall(b'1')
        sleep(1)
        s.sendall(b'.')
        sleep(1)
        s.sendall(b'1')
        sleep(1)
        s.sendall(b'\r\n')
        sleep(1)
        s.sendall(b'Host: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        while data := s.recv(1024):
            print(data)

def test_4():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        # (host: str, port: int)
        s.connect(('cs.muic.mahidol.ac.th', 80))

        #s.sendall(b'GET / HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')
        s.sendall(b'adawdasd')
        s.sendall(b'\r\n')

        while data := s.recv(1024):
            print(data)

def test_5():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        # (host: str, port: int)
        s.connect(('cs.muic.mahidol.ac.th', 80))

        s.sendall(b'GET /eng/programs/undergraduate-programs/science/computer-science/ HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')

        while data := s.recv(1024):
            print(data)
def test_6():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        # (host: str, port: int)
        s.connect(('cs.muic.mahidol.ac.th', 80))

        s.sendall(b'PUT /eng/programs/undergraduate-programs/science/computer-science/ HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')

        while data := s.recv(1024):
            print(data)
def test_7():
    with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
        # (host: str, port: int)
        s.connect(('cs.muic.mahidol.ac.th', 80))

        s.sendall(b'DELETE /eng/programs/undergraduate-programs/science/computer-science/ HTTP/1.1\r\nHost: cs.muic.mahidol.ac.th\r\nConnection: close\r\n\r\n')

        while data := s.recv(1024):
            print(data) 
#test_1() # does respond
#test_2() # gets frozen and returns an error
#test_3() # does respond
#test_4() # sends an automatic response
#test_5() # instant redirection
#test_6() # instant redirection
#test_7() # instant redirection