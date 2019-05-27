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
#include "client.h"

#define PORT 5000

void close_connection(int socket)
{
  write(socket, "exit", strlen("exit"));
  close(socket);
}

//Download file from remote directory
void receive_file(char *file, int socket)
{
  int n;
  char buffer[256];
  if (getFileFromStream(file, socket) > 0)
  {
    strcpy(buffer, "File received successfully!\n");
    n = write(socket, buffer, sizeof(buffer));
    printf("Download of the file %s finished!\n", file);
  }
  else
  {
    printf("File doesn't exist. Tip: Use list to see all the files in your directory ;) \n");
  }
}

//Upload file to remote directory
void send_file(char *file, int socket)
{
  FILE *fp = fopen(file, "r");
  int n, count = 0;
  long int size = file_size(fp);
  char buffer, newBuffer[256], last_modified[MAXNAME];
  ;
  // Auxiliar to help casting the file size to buffer
  unsigned char *bufferSize;

  if (!fp)
  {
    printf("File doesn't exist. Please specify a valid file name\n");
    bufferSize = (unsigned char *)&size;
    n = write(socket, (void *)bufferSize, 4);
    return;
  }
  // Valid file, starts the stream to the server
  else
  {
    TIMEINFO *newTime = (TIMEINFO *)malloc(sizeof(TIMEINFO));
    newTime->t0 = atoi(getClientTime());
    //envia a requisição ao servidor pedindo o seu horario
    newTime->ts = getTimeServer(socket);
    newTime->t1 = atoi(getClientTime());
    strcpy(last_modified, getCorrectTime(newTime));
    // After passes the size of the file to the server
    bufferSize = (unsigned char *)&size;
    n = write(socket, (void *)bufferSize, 4);
    if (n > 0)
    {
      while (count < size)
      {
        buffer = fgetc(fp);
        count++;
        n = write(socket, (void *)&buffer, 1);
        if (n < 0)
          break;
      }
    }
    //Finished passing the file, closes the file and
    // wait for servers answer
    printf("File %s upload is finished.\n", file);
    fclose(fp);
    n = read(socket, newBuffer, 256);
    printf("Server answered: %s\n", newBuffer);
  }
}

void client_loop(int socket)
{
  int n;
  char *fileName;
  char buffer[256];
  while (1)
  {
    bzero(buffer, 256);
    printf("client > ");
    fgets(buffer, 256, stdin);

    if (strstr(buffer, "exit"))
    {
      close_connection(socket);
      break;
    }
    if (strstr(buffer, "upload"))
    {
      // To get the filename
      n = write(socket, buffer, strlen(buffer));
      if (n < 0)
        printf("Error sending upload command. \n");
      else
      {
        // Parsing the file name
        fileName = parseFilename(buffer);
        send_file(fileName, socket);
      }
    }
    else if (strstr(buffer, "list"))
    {
      n = write(socket, buffer, strlen(buffer));
      print_file_list(socket);
    }
    else if (strstr(buffer, "download"))
    {
      n = write(socket, buffer, strlen(buffer));
      if (n < 0)
        printf("Error sending download command.\n");
      else
      {
        fileName = parseFilename(buffer);
        receive_file(fileName, socket);
      }
    }
  }
}
// Basicamente o código de TCP do pôfessô
int connect_server(char *host, int port)
{
  int socketfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256];

  server = gethostbyname(host);
  if (server == NULL)
  {
    fprintf(stderr, "ERROR, no such host\n");
    return -1;
  }
  if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    printf("ERROR opening socket\n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
  bzero(&(serv_addr.sin_zero), 8);
  if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    printf("ERROR connecting\n");
  return socketfd;
}

void print_file_list(int socket)
{
  int n;
  char buffer[1000];
  n = read(socket, buffer, sizeof(buffer));
  printf("%s", buffer);
}

int main(int argc, char *argv[])
{
  char buffer[MAXCHARS];
  if (argc < 4)
  {
    printf("Usage: ./client username hostname port");
    exit(0);
  }
  int port = atoi(argv[3]);
  int socket = connect_server(argv[2], port);

  // Cria as duas listas, a local e a do servidor

  // Faz a conexão do usuário com o servidor
  char username[40];
  strcpy(username, "login ");
  strcat(username, argv[1]);
  int n = write(socket, username, strlen(username));

  n = read(socket, buffer, sizeof(buffer));
  if ((n > 0) && (strstr(buffer, "OK")))
    client_loop(socket);
  else
  {
    printf("You already have two devices connected. Please, disconnect from one and try again!\n");
  }
  printf("Connection closed. Terminating the program\n");
  return 0;
}
