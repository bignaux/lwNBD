#include <sys/socket.h>
#include <netinet/in.h>
#include <endian.h>
#include <unistd.h>
#include <assert.h> //todo: move in .c
typedef signed char err_t;
//TODO : manage endianess
#define htonll(x) htobe64(x)
#define ntohll(x) be64toh(x)
