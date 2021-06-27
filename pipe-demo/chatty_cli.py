#!/usr/bin/env python3
from typing import List
from time import sleep
import os, sys

def display_envvars(show: List[str], fp = sys.stdout):
    print(f'####### Environment Variables #######', file=fp)
    for name, value in os.environ.items():
        if name not in show:
            continue
        print(f'{name}={repr(value)}', file=fp)

def echo_line(fin, fout):
    data = fin.read()
    print(f'Received the following {repr(data)} (len={len(data)}).', file=fout)

def delayed_print(duration: float):
    prev, now = 0, 1
    for index in range(1, 10+1):
        print(f'fib[{index}]={now}', flush=True)
        prev, now = now, prev + now
        sleep(duration)
if __name__ == '__main__':
    display_envvars(show=['HOME', 'USER', 'PWD'])
    echo_line(sys.stdin, sys.stdout)
    delayed_print(duration=0.1) # sleep for .1 seconds between prints
