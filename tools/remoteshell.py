#!/usr/bin/python3

# Sample to demonstrate usage of URI with query on a NBD server
#
# usage:
# export TARGET_IP=192.168.1.5
# ./remoteshell.py dumpmem offset=0x200 size=0x200
#
# On Alpine :
# echo "@testing https://dl-cdn.alpinelinux.org/alpine/edge/testing/" >> /etc/apk/repositories && apk update
# apk add libnbd@testing

import argparse
import nbd
import os
import sys
from cmd import Cmd
from urllib.parse import urlunsplit, urlencode, quote, parse_qs


class MyPrompt(Cmd):
    prompt = 'lwnbd> '
    intro = "Welcome! Type ? to list commands"

    def do_exit(self, inp):
        print("Bye")
        return True

    def help_exit(self):
        print('exit the application. Shorthand: x q Ctrl-D.')

    def do_add(self, inp):
        print("adding '{}'".format(inp))

    def help_add(self):
        print("Add a new entry to the system.")

    def default(self, inp):
        if inp == 'x' or inp == 'q':
            return self.do_exit(inp)

        args.query = [inp]
        send_request(args)

    do_EOF = do_exit
    help_EOF = help_exit


def send_request(args):
    encoded_url = create_uri(args)
    h = nbd.NBD()
    h.connect_uri(encoded_url)
    size = h.get_size()
    if args.verbose:
        print('Response:', size, 'bytes')
    ret = h.pread(size, 0)
    s = str(ret, 'utf-8')
    print(s)
    h.shutdown()


def create_uri(args):
    # ## not very happy with that parsing
    # if args.verbose: print(args.query)
    query = parse_qs('&'.join(args.query), keep_blank_values=True)
    # if args.verbose: print(query)
    args.query = urlencode(query, doseq=True, quote_via=quote)
    # if args.verbose: print(args.query)
    # query = args.command + '=' + ' '.join(args.arguments)
    uri = urlunsplit(('nbd', args.target, args.exportname, args.query, ''))
    if args.verbose:
        print('Clear URI:', uri)
    encoded_url = quote(uri, safe=':/%=')
    if args.verbose:
        print('Request:', encoded_url)
    return encoded_url


p = argparse.ArgumentParser(description='remote command for lwNBD',
                            epilog='This software is part of lwNBD project https://github.com/bignaux/lwNBD')
p.add_argument('query', default='list', nargs='*')
p.add_argument('-e', '--exportname', default='shell', help='name of export')
p.add_argument('-i', '--interactive', action='store_true',
               help='interactive shell')
p.add_argument('-t', '--target',
               default=os.environ['TARGET_IP'], help='hostname or IP of target')
p.add_argument('-v', '--verbose', action='store_true',
               help='increase output verbosity')
args = p.parse_args()

if not args.target:
    sys.exit("You should provide a target via export TARGET_IP or --target argument")

if args.interactive:
    MyPrompt().cmdloop()
else:
    send_request(args)
