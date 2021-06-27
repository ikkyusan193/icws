import http.client
import random
import socket as sk
from time import sleep

# Shamelessly copied from stackoverflow for stylez
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

# Have user input host and port
inputHost = input("Send a request to (host:port): ")
inputHost, inputPort = inputHost.split(":")

# Initiate a connection
connection = http.client.HTTPConnection(host=inputHost, port=int(inputPort))
connection.set_debuglevel(1)

# Our valid headers
headers = [
        {
            "Host" : inputHost, 
            "Accept" : "text/html, image/png, text/css", 
            "Accept_Language" : "en-us", 
            "Connection" : "close"
        }, 
        {
            "Host" : inputHost, 
            "Accept" : "text/html, image/png, text/css, application/xhtml+xml, image/jpg", 
            "Accept_Language" : "en-us", 
            "Connection" : "close"
        },
        {
            "Host" : inputHost, 
            "Accept" : "image/jpg", 
            "Accept_Language" : "en-us, en-uk", 
            "Connection" : "close"
        },
        {
            "Host" : inputHost, 
            "Accept" : "text/css", 
            "Accept_Language" : "en-us, en-uk", 
            "Connection" : "close"
        },
        {
            "Host" : inputHost, 
            "Accept" : "text/html, image/png, text/css, application/xhtml+xml, image/jpg", 
            "Accept_Language" : "en-us", 
            "Connection" : "keep-alive",
            "Accept-Encoding" : "gzip, deflate",

        },
        {
            "Host" : inputHost, 
            "Accept" : "image/jpg", 
            "Accept_Language" : "en-us, en-uk",
            "Content-Length" : "2",
            "Connection" : "close"
        },
        {
            "Host" : inputHost, 
            "From": "kanat.tan@mahidol.edu",
            "Max-Forwards": "200",
            "Accept" : "text/html, image/png, text/css, application/xhtml+xml, image/jpg", 
            "Accept_Language" : "en-us, en-uk", 
            "Connection" : "close",
            "Warning": "199 Miscellaneous warning"
        }

]

# Our invalid headers
invalidheaders = [
        {
            "Host": "Joe.mama",
            "Accept": "nothing",
            "Accept_Language": "France",
            "Connection": "fermer"
        },
        {
            "Host": "Seymour Butts",
            "Accept": "everything aha!",
            "Accept_Language": "sign language",
            "Hotel" : "Trivago"
        },
        {
            "Host": "",
            "Accept": "nothing",
            "Accept_Language": "void",
            "Connection": "gone forever"
        },
        {
            "Host": "Danny Devito",
            "Accept": "Danny Devito",
            "Accept_Language": "Danny Devito",
            "Connection" : "Danny Devito"
        },
        {
            "Host": "",
            "Accept": "",
            "Accept_Language": "",
            "Connection": ""
        },
        {
            "Host": "aosIJDOIASUDIULAHIDUIU!@Omkl",
            "Accept": "AOSHYDIOASYDO7Y7198273881902199!(*)*@u#)!@MEPLEASEHELPMEMYSANITYISWANINGOWMEOWMEOWoihsaushdyasgdfuiahskdFUCJALKDMA,.D",
            "Accept_Language": "Help",
            "Connection": "the spark between us"
        },
        {
            "Host": "Ajarn Sunsern",
            "Accept": "Ajarn Kanat",
            "Accept_Language": "Ajarn Rachata",
            "Connection" : "CMU Gods"
        }
]

method = ["GET", "HEAD"]

"""
The logic for both loops is basically:
    1) Generate a random number
    2) Check to see if it's even or odd and send a normal/weird (spaced out) request based on its value with our valid/invalid headers
    3) Print out the response
"""

# Here we try invalid requests
print(bcolors.OKGREEN + "\nTRYING VALID REQUESTS" + bcolors.ENDC)

for i in range(len(headers)):
    print(bcolors.HEADER + bcolors.BOLD + "\nREQUEST " + str(i + 1) + bcolors.ENDC)
    pause = random.randint(0, 20)
    if (pause % 2 == 0):
        print(bcolors.OKCYAN + "WE WILL BE MAKING A NORMAL REQUEST" + bcolors.ENDC)
        connection.request(method[random.randint(0,1)], "/", headers=headers[i])
        resp = connection.getresponse()
    
        print('---------------------------------')
        print(resp.status, resp.reason)
        print(resp.headers)
        print(resp.read())

    else:
        print(bcolors.WARNING + "PREPARE TO RECEIVE A MESSED UP REQUEST FROM SOMEONE IN JAMAICA" + bcolors.ENDC)
        with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
            s.connect((inputHost, int(inputPort)))

            s.sendall(b"GET ")
            slep = random.randint(0, 10)
            print(bcolors.WARNING + "I SHALL NAP NAP FOR: " + str(slep) + bcolors.ENDC) 
            sleep(slep)
            
            s.sendall(b"/ HTTP/1." + bytes(str(random.randint(0, 10)), "utf-8") + b"\r\n")
            slep = random.randint(0, 10)
            print(bcolors.WARNING + "I SHALL NAP NAP FOR: " + str(slep) + bcolors.ENDC) 
            sleep(slep)
            
            s.sendall(b"Host: " + bytes(inputHost, "utf-8")  + b"\r\nConnection: close\r\n")
            slep = random.randint(0, 10)
            print(bcolors.WARNING + "I SHALL NAP NAP FOR: " + str(slep) + bcolors.ENDC) 
            sleep(slep)
            
            s.sendall(b"\r\n")
            
            while data := s.recv(1024):
                print(data)


print(bcolors.FAIL + "\nTRYING INVALID REQUESTS" + bcolors.ENDC)

for i in range(len(invalidheaders)):
    print(bcolors.HEADER + bcolors.BOLD + "\nREQUEST " + str(i + 1) + bcolors.ENDC)
    pause = random.randint(0, 20)
    if (pause % 2 == 0):
        print(bcolors.OKCYAN + "WE WILL BE MAKING A NORMAL REQUEST" + bcolors.ENDC)
        connection.request(method[random.randint(0,1)], "/", headers=invalidheaders[i])
        resp = connection.getresponse()
    
        print('---------------------------------')
        print(resp.status, resp.reason)
        print(resp.headers)
        print(resp.read())

    else:
        print(bcolors.WARNING + "PREPARE TO RECEIVE A MESSED UP REQUEST FROM SOMEONE IN JAMAICA" + bcolors.ENDC)
        with sk.socket(sk.AF_INET, sk.SOCK_STREAM) as s:
            s.connect((inputHost, int(inputPort)))

            s.sendall(b"GET ")
            slep = random.randint(0, 10)
            print(bcolors.WARNING + "I SHALL NAP NAP FOR: " + str(slep) + bcolors.ENDC) 
            sleep(slep)
            
            s.sendall(b"/ HTTP/1." + bytes(str(random.randint(0, 10)), "utf-8") + b"\r\n")
            slep = random.randint(0, 10)
            print(bcolors.WARNING + "I SHALL NAP NAP FOR: " + str(slep) + bcolors.ENDC) 
            sleep(slep)
            
            s.sendall(b"Host: " + bytes(inputHost, "utf-8")  + b"\r\nConnection: close\r\n")
            slep = random.randint(0, 10)
            print(bcolors.WARNING + "I SHALL NAP NAP FOR: " + str(slep) + bcolors.ENDC) 
            sleep(slep)
            s.sendall(b"\r\n")
            
            while data := s.recv(1024):
                print(data)


print(bcolors.BOLD + bcolors.OKGREEN + "\nCONGRATULATIONS, YOUR SERVER IS WORKING. GOOD FOR YOU. MINE ISN'T. NOT THAT ANYONE CARES." + bcolors.ENDC)
