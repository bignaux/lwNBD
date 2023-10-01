# command plugin

## Usage : A RPC handler

TARGETS : all

STATUS : Experimental

```c
struct lwnbd_command
{
    char *name;
    char *desc;
    int (*cmd)(int argc, char **argv, void *result, int64_t *size);
};
```

As traditionnal (int argc, char **argv) scheme, you need to manually do the Command-line argument parsing ...
for now argv contains only the first query of the request, so the all the parameters should go in first value string (space allowed).

see example in [examples/local-shell.c](./examples/local-shell.c)

You can test the mecanism over NBD with [lwnbd-server server](./examples/lwnbd-server.c) and [remoteshell.py](./examples/remoteshell.py)

Start lwnbd-server then remoteshell client, typic session :

```
/app/lwNBD/examples # ./remoteshell.py -iv
Welcome! Type ? to list commands
lwnbd> lc
Clear URI: nbd://192.168.1.5/shell?lc=
Request: nbd://192.168.1.5/shell%3Flc=
Response: 153 bytes
lc                              : list commands
le                              : list export
shutdown                        : turn off the application

lwnbd> shutdown=2
Clear URI: nbd://192.168.1.5/shell?shutdown=2
Request: nbd://192.168.1.5/shell%3Fshutdown=2
Response: 31 bytes
shutdown with signal Interrupt
lwnbd> exit
Bye
```

Then lwnbd-server finished his job and terminate using signal interruption.
Thanks to the verbose switch, you can see we got kinda HTTP GET equivalent on NBD transport.