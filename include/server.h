#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <dirent.h>
#include "util.h"

PFILA2 clientsList;
pthread_mutex_t mutexDevicesRegister = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexClientRegister = PTHREAD_MUTEX_INITIALIZER;

void sync_server();
void receive_file(char *file, int socket, CLIENT *user);
void send_file(char *file, int socket, CLIENT *user);
void timeServer(int socket);
