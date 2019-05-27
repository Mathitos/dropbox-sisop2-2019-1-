#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "fila2.h"

#define MAXNAME 30
#define MAXFILES 30
#define MAXCHARS 100
#define ERROR 1
#define SIZE_ERROR 4294967295 /* Que deus perdoe esses programadores ruins */

struct file_info
{
  char name[MAXNAME];
  char extension[MAXNAME];
  char last_modified[MAXNAME];
  int size;
};
struct client
{
  int devices[2];
  char userid[MAXNAME];
  PFILA2 files;
  int logged_in;
};

struct time_info
{
  int t0;
  int t1;
  int ts;
  int tc;
};

typedef struct client CLIENT;
typedef struct file_info FILEINFO;
typedef struct time_info TIMEINFO;

int file_size(FILE *fp);

FILEINFO *createFile(char *name, int size);
CLIENT *createClient(char *name);
char *getCorrectTime(TIMEINFO *file_time);
char *getCurrentTime();
int getTimeServer(int socket);
FILEINFO *findFile(CLIENT *user, char *name);
CLIENT *findClient(PFILA2 clientsList, char *name);
char *getClientTime();
int getFileFromStream(char *file, int socket);
char *parseFilename(char *src);
int getUnusedFILEINFO(CLIENT user);
char *getPath(CLIENT *user, char *file);

void parseNameExt(char *src, char *name, char *ext);
