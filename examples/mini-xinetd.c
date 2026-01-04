
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <lwnbd/lwnbd.h>
//#include <lwnbd/lwnbd-server.h>

#define MAX_SERVICES 32
#define MAX_LINE     512

typedef struct
{
    char name[64];
    int port;
    char server[256];
    int listen_fd;
} service_t;

// typedef struct service {
//     char *id;              // service identifier (name)
//     char *server;          // path to the executable for this service
//     char *server_args;     // arguments to pass to the server
//     int port;              // port number the service listens on
//     int protocol;          // protocol (TCP = 6, UDP = 17, etc.)
//     int socket_type;       // socket type (SOCK_STREAM, SOCK_DGRAM, etc.)
//     int wait;              // wait flag (1 = service handles one client at a time, 0 = fork per connection)
//     char *user;            // user to run the service as
//     int disable;           // flag to disable the service (1 = disabled)
//     /* Many other fields exist in xinetd for resource limits, logging, access control, etc. */
//     struct rlimit rlimits[MAX_RLIMITS]; // resource limits (CPU, memory, file descriptors, etc.)
//     /* ...additional internal fields... */
// } service_t;

static service_t services[MAX_SERVICES];
static int service_count = 0;

typedef struct
{
    int conn_fd;
    char name[64];
    ;
} thread_arg_t;

void die(const char *msg)
{
    perror(msg);
    exit(1);
}

char *trim(char *s)
{
    while (*s && isspace(*s))
        s++;
    char *end = s + strlen(s);
    while (end > s && isspace(*(end - 1)))
        end--;
    *end = '\0';
    return s;
}

/* ------------------ Parsing façon xinetd ---------------------- */
void parse_config(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        die("fopen config");

    char line[MAX_LINE];
    service_t current;
    int in = 0;

    while (fgets(line, sizeof(line), f)) {
        char *p = trim(line);
        if (*p == '\0' || *p == '#')
            continue;

        if (strncmp(p, "service", 7) == 0) {
            char name[64], brace[8];
            if (sscanf(p, "service %63s %7s", name, brace) != 2) {
                fprintf(stderr, "bad service decl\n");
                exit(1);
            }
            memset(&current, 0, sizeof(current));
            strcpy(current.name, name);
            in = 1;
            continue;
        }

        if (strcmp(p, "}") == 0) {
            services[service_count++] = current;
            in = 0;
            continue;
        }

        if (in) {
            char key[64], eq[4], value[256];
            if (sscanf(p, "%63s %3s %255s", key, eq, value) == 3) {
                if (strcmp(key, "port") == 0)
                    current.port = atoi(value);
                else if (strcmp(key, "server") == 0)
                    strcpy(current.server, value);
            }
        }
    }
    fclose(f);
}

/* ------------------ Listener socket --------------------------- */
int start_listener(int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        die("socket");

    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        die("bind");
    if (listen(fd, 20) < 0)
        die("listen");

    return fd;
}

/* ------------------ Thread routine ---------------------------- */
#define RUN_SERVER_ON_SOCKET(server, fd)            \
    do {                                            \
        if (!(server)) {                            \
            dprintf(fd, "Server not found\n");      \
            close(fd);                              \
            break;                                  \
        }                                           \
        FILE *thread_stdout = fdopen(dup(fd), "w"); \
        FILE *thread_stderr = fdopen(dup(fd), "w"); \
        if (!thread_stdout || !thread_stderr) {     \
            close(fd);                              \
            break;                                  \
        }                                           \
        FILE *old_stdout = stdout;                  \
        FILE *old_stderr = stderr;                  \
        stdout = thread_stdout;                     \
        stderr = thread_stderr;                     \
        (server)->run(NULL, NULL);                  \
        stdout = old_stdout;                        \
        stderr = old_stderr;                        \
        fclose(thread_stdout);                      \
        fclose(thread_stderr);                      \
    } while (0)

void *service_thread(void *arg)
{
    thread_arg_t *t = arg;
    int fd = t->conn_fd;
    char name[64];
    strncpy(name, t->name, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    free(t);

    printf("coucou %s\n", name);
    struct lwnbd_server *s = get_server_by_name(name);
    if (!s) {
        printf("not found\n");
        return NULL;
    }

    RUN_SERVER_ON_SOCKET(s, fd); // <-- toute la logique est ici

    close(fd);
    return NULL;
}

/* ------------------ main loop avec select + threads ----------- */
int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <config>\n", argv[0]);
        exit(1);
    }

    parse_config(argv[1]);
    printf("Loaded %d services\n", service_count);

    int maxfd = -1;

    lwnbd_servers_init();

    for (int i = 0; i < service_count; i++) {
        services[i].listen_fd = start_listener(services[i].port);
        printf("Service '%s' listening on port %d\n",
               services[i].name, services[i].port);
        if (services[i].listen_fd > maxfd)
            maxfd = services[i].listen_fd;
    }

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);

        for (int i = 0; i < service_count; i++)
            FD_SET(services[i].listen_fd, &readfds);

        int ret = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0)
            die("select");

        /* Nouveaux clients */
        for (int i = 0; i < service_count; i++) {
            int lfd = services[i].listen_fd;
            if (FD_ISSET(lfd, &readfds)) {
                int conn = accept(lfd, NULL, NULL);
                if (conn < 0) {
                    perror("accept");
                    continue;
                }

                printf("Connection for service '%s'\n", services[i].name);

                /* Préparer args du thread */
                thread_arg_t *t = malloc(sizeof(thread_arg_t));
                t->conn_fd = conn;
                strcpy(t->name, services[i].name);

                /* Lancer un thread */
                pthread_t th;
                pthread_create(&th, NULL, service_thread, t);
                pthread_detach(th); // On ne récolte pas le join
            }
        }
    }

    return 0;
}
