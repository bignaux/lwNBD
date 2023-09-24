# Sample to demonstrate usage of URI with query as a way 
# to send command remotely
# 
# usage:
# export TARGET_IP=192.168.1.5
# python remoteshell.py motd%3fmemset=p
#

#!/usr/bin/python3
import nbd
import os
import sys

# TODO read from cli first [-h hostname]
host = os.environ['TARGET_IP']
if not host:
    print("no host")

n = len(sys.argv)
if n > 1:
    query = sys.argv[1]

# would need encoding URI

h = nbd.NBD()
uri = "nbd://" + host + '/' + query

h.connect_uri(uri)
ret = h.pread(512, 0)
s = str(ret, 'utf-8')
print(s)
