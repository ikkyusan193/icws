import http.client

conn = http.client.HTTPConnection(host='cs.muic.mahidol.ac.th', port=80)

conn.set_debuglevel(1)  # this will print the connection-level info (verbose)

headers = {
    'Accept': 'text/plain',
    'Blah': 'Bleh',
    'Connection': 'close',
}

headers1 = {
    'Accept': 'image/jpg',
    'Connection': 'close',
}

headers2 = {
    'adwadasdawdadw',
}

headers3 = {
    'Accept': 'text/plain',
    'Connection': 'open',
}

headers4 = {
    'Accept': '*/*',
    'Connection': 'close',
}

headers5 = {
    'Accept': 'images/*',
    'Connection': 'close',
}

headers6 = {
    'Accept': 'text/*',
    'Connection': 'close',
}

def request_test_1(hd):
    conn.request('GET', '/', headers=hd)  # send the request + headers
    resp = conn.getresponse()
    print('-------------------------------')
    print(resp.status, resp.reason) # Status Code, "Explanation"
    print(resp.headers)  # Read all the headers
    print(resp.read())   # Print the body

#request_test_1(headers)
#request_test_1(headers1) # instant redirection
#request_test_1(headers2) # instant error
#request_test_1(headers3) # instant redirection
#request_test_1(headers3) # instant redirection
#request_test_1(headers4) # instant redirection
#request_test_1(headers6) # instant redirection