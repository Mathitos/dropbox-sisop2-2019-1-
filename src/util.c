#include "util.h"

int file_size(FILE *fp)
{
  int returnSize;
  if (fp != NULL)
  {
    int prev = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    returnSize = ftell(fp);
    fseek(fp, prev, SEEK_SET);
    return returnSize;
  }
  else
  {
    return -1;
  }
}

char *getClientTime()
{
  char *timestamp = (char *)malloc(sizeof(char) * 16);
  int n;
  time_t ltime;
  ltime = time(NULL);
  struct tm *tm;
  tm = localtime(&ltime);
  sprintf(timestamp, "%04d%02d%02d%02d%02d%02d", tm->tm_year + 1900, tm->tm_mon,
          tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
  return timestamp;
}

CLIENT *createClient(char *name)
{
  CLIENT *newClient = (CLIENT *)malloc(sizeof(CLIENT));
  strcpy(newClient->userid, name);
  newClient->files = (PFILA2)malloc(sizeof(FILA2));
  CreateFila2(newClient->files);
  newClient->logged_in = 1;
  newClient->devices[0] = -1;
  newClient->devices[1] = -1;
  return newClient;
}

char *getCorrectTime(TIMEINFO *file_time)
{
  struct tm *timeinfo;
  char *str = (char *)malloc(sizeof(char) * 16);
  if (file_time == NULL)
  {
    printf("ERROR on calculate synchron time\n");
    return NULL;
  }
  else
  {
    file_time->tc = (file_time->ts + (file_time->t1 - file_time->t0) / 2);
    sprintf(str, "%d", file_time->tc);
    return str;
  }
}

int getTimeServer(int socket)
{
  char buffer[256];
  int n;
  n = read(socket, buffer, 256);
  printf("Request return %s", buffer);
  if (n < 0)
  {
    printf("ERROR reading from socket");
    return -1;
  }
  else
    return atoi(buffer);
}

/* Cria o arquivo e seta os valores certinhos  */
FILEINFO *createFile(char *name, int size)
{
  if (strstr(name, "."))
  {
    FILEINFO *newFile = (FILEINFO *)malloc(sizeof(FILEINFO));
    parseNameExt(name, newFile->name, newFile->extension);
    strcpy(newFile->last_modified, getCurrentTime());
    newFile->size = size;
    return newFile;
  }
  else
    return NULL;
}

char *getCurrentTime()
{
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  return asctime(timeinfo);
}

FILEINFO *findFile(CLIENT *user, char *name)
{
  FirstFila2(user->files);
  FILEINFO *it;
  do
  {
    it = GetAtIteratorFila2(user->files);
    if ((it) && (strstr(name, it->name) && strstr(name, it->extension)))
    {
      return it;
    }

  } while (!NextFila2(user->files));
  return NULL;
}

CLIENT *findClient(PFILA2 clientsList, char *name)
{
  FirstFila2(clientsList);
  CLIENT *it;
  do
  {
    it = GetAtIteratorFila2(clientsList);
    if ((it) && (strcmp(it->userid, name) == 0))
      return it;
  } while (!NextFila2(clientsList));
  return NULL;
}

char *parseFilename(char *src)
{
  /* Parte 1: A partir do comando, pega só o path */
  char *token;
  char *path = strtok(src, " ");
  path = strtok(NULL, " ");
  path[strlen(path) - 1] = 0;
  /* Com apenas path/to/file/name.ext, ignora o path
       e fica só com o name.ext */
  /* Caso exista um path/to/file, retira */
  if (strstr(path, "/"))
  {
    token = strtok(path, "/");
    while ((token) && (!strstr(token, ".")))
    {
      token = strtok(NULL, "/");
    }
    return token;
  }
  /* Caso contrário, retorna path pois ele é só
	   composto por nome e extensão */
  else
    return path;
}

int getFileFromStream(char *file, int socket)
{
  long int size = 0;
  int i, n;
  unsigned char *bufferSize;
  char buffer;
  /* pega o tamanho do file a ser recebido */
  bufferSize = (unsigned char *)&size;
  n = read(socket, bufferSize, 4);
  if (size >= SIZE_ERROR)
  {
    return -1;
  }
  else
  {
    FILE *fp = fopen(file, "w+");
    /* começa a ler da stream, byte a byte, o arquivo */
    for (i = 0; i < size; i++)
    {
      n = read(socket, (void *)&buffer, 1);
      fputc(buffer, fp);
    }
    fclose(fp);
    return size;
  }
}
char *getPath(CLIENT *user, char *file)
{
  static char path[MAXCHARS];
  strcpy(path, "sync_dir_");
  strcat(path, user->userid);
  strcat(path, "/");
  strcat(path, file);
  return path;
}

// Divide a string "filename.ext" em "filename" e "ext"
void parseNameExt(char *src, char *name, char *ext)
{
  strcpy(name, strtok(src, "."));
  strcpy(ext, strtok(NULL, "."));
}
