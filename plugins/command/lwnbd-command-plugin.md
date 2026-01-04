# command plugin

## Usage : A RPC handler

TARGETS : 🎯 all

STATUS : 💣 *very* experimental, in future, we won't rely on a plugin to manage broker.
 

```c
typedef enum {
    METHOD_GET,
    METHOD_POST,
} method_type;

struct lwnbd_command
{
    char *name;
    char *desc;
    method_type type;
    int (*cmd)(int argc, char **argv, void *data, int64_t *size);
};
```

* method of type METHOD_GET, are executed on lwnbd_get_context(), and fill data of size size, that will be read later.
    see example in [examples/local-shell.c](./examples/local-shell.c)
* method of type METHOD_POST, are pwrite() callback function, needs to be reentrant. They received streamed data of size size.
    see example in [examples/local-command-wc.c](./examples/local-command-wc.c)

As traditionnal (int argc, char **argv) scheme, you need to manually do the Command-line argument parsing ...
for now argv contains only the first query of the request, so the all the parameters should go in first value string (space allowed).

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
