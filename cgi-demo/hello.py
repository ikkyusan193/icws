#!/usr/bin/env python3

from os import environ
import cgi, cgitb

CRLF = '\r\n'

cgitb.enable()

query = cgi.FieldStorage()
name = query.getfirst('name', 'Unknown')

print('HTTP/1.1 200 OK', end=CRLF)
print(f'Server: {environ["SERVER_SOFTWARE"]}', end=CRLF)
print(end=CRLF)

print('<html><body>')
print('<h1>Hello!</h1>')
print(f'<h2>Nice to meet you, {name}!</h2>')
print('</body></html>')
