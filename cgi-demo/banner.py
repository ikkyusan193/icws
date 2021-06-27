#!/usr/bin/env python3

from os import environ
import sys
import cgi, cgitb
from urllib import request

CRLF = '\r\n'
TARGET_API='https://artii.herokuapp.com/make'

cgitb.enable()

print('HTTP/1.1 200 OK', end=CRLF)
print(f'Server: {environ["SERVER_SOFTWARE"]}', end=CRLF)
print('Content-Type: plain/text', end=CRLF)
print(end=CRLF)

query = cgi.FieldStorage()
text = query.getfirst('text', 'Meh')

with request.urlopen(TARGET_API+f'?text={text}') as resp:
   data = resp.read()
   sys.stdout.buffer.write(data)
   sys.stdout.flush()
