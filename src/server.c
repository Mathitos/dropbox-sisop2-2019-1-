#include "server.h"
#define PORT 5000

void timeServer(int socket) {
    char *timestamp = (char *)malloc(sizeof(char) * 16);
    char buffer[256];
    int n;
    time_t ltime;
    ltime=time(NULL);
    struct tm *tm;
    tm=localtime(&ltime);
    sprintf(timestamp,"%04d%02d%02d%02d%02d%02d", tm->tm_year+1900, tm->tm_mon, 
          tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    strcpy(buffer, timestamp);
    n = write(socket,buffer, sizeof(buffer));
    printf("TimeStamp sent: %s\n", buffer);
}

//Receives file uploaded by client
void receive_file(char* file, int socket, CLIENT* user){
    long int size = 0;
    char buffer[256];
    int n;
    char *path = getPath(user, file);
	FILE* fp = fopen(path, "r+");
    if (fp){
        fclose(fp);
        timeServer(socket);
        if((size = getFileFromStream(path, socket)) > 0) {
            // Avisa o cliente que o file foi atualizado
            // e coloca mensagem na tela
            strcpy(buffer, "File updated successfully!\n");
            n = write(socket, buffer, sizeof(buffer));
            printf("User %s id %d > File %s finished!!!\n", user->userid,socket,file);

            // Como o arquivo existe, a estrutura FILEINFO também já existe
            // logo, a atualiza com o novo time stamp
            FILEINFO* newFile = findFile(user, file);
            if(newFile){
                strcpy(newFile->last_modified, getCurrentTime());
                printf("File structure updated successfully!\n");
            }
            else {
                newFile = createFile(file, size);
                if(AppendFila2(user->files, newFile) ==0){
                    printf("File structured created successfully\n");
                }
            }
        }
        else {
            strcpy(buffer, "Error while transfering file. Please, try again\n");
            n = write(socket, buffer, sizeof(buffer));
        } 
    }
    // File não existe no servidor. Cria o arquivo e a estrutura
    else{
        if((size = getFileFromStream(path, socket)) > 0) {
            strcpy(buffer, "File uploaded successfully!\n");
            FILEINFO* newFile = createFile(file, size);
            if(AppendFila2(user->files, (void*)newFile) == 0){
                n = write(socket, buffer, sizeof(buffer));
                printf("User %s id %d > File %s upload finished and added to filelist\n", user->userid, socket, file);
        }

            // Agora cria uma estrutura file e dá append :)
        }
        else {
            strcpy(buffer, "Error while transfering file. Please, try again\n");
            n = write(socket, buffer, sizeof(buffer));
        }
    }
}

//List file on clients remote directory
void list_files(int socket, CLIENT* user){
    char buffer[MAXCHARS*3] = "";
    int n, i = 0;
    if(!(FirstFila2(user->files))){
        FILEINFO* it;
        do{
            it = GetAtIteratorFila2(user->files);
            strcat(buffer, it->name);
            strcat(buffer, ".");
            strcat(buffer, it->extension);
            strcat(buffer, "\n");
            printf("%d\n", i);
            i++;
        } while(!NextFila2(user->files));

        printf("file %s\n", buffer);
        n = write(socket, buffer, sizeof(buffer));
    } else {
        printf("No files on client directory to list\n");
        strcpy(buffer, "No files on remote directory\n");
        n = write(socket, buffer, sizeof(buffer));
    }
}

int registerSocket(int socket, CLIENT* user){
    int returnValue = 0;
    pthread_mutex_lock(&mutexDevicesRegister);
    if(user->devices[0] < 0){
        user->devices[0] = socket;
        returnValue = 1;
    }
    else if (user->devices[1] < 0){
        user->devices[1] = socket;
        returnValue = 1;
    }
    pthread_mutex_unlock(&mutexDevicesRegister);
    return returnValue;
}

void deregisterSocket(int socket, CLIENT* user){
    if(user->devices[0] == socket)
        user->devices[0] = -1;
    else if (user->devices[1] == socket)
        user->devices[1] = -1;

    if((user->devices[0] < 0)  && (user->devices[1] < 0)){
        CLIENT* try = findClient(clientsList, user->userid);
        DeleteAtIteratorFila2(clientsList);
        free(try);
        FirstFila2(clientsList);
    }
}


// Depois: Cria o diretório e a estrutura cliente e a preenche com os arquivos dentro
// do diretório
int login_user(CLIENT* user, int socket){
    if(registerSocket(socket, user)){
        struct stat st = {0};
        char directory[40];
        strcpy(directory, "./sync_dir_");
        strcat(directory, user->userid);
        DIR *dir;
        int size = -1;
        struct dirent *ent;
        FILEINFO* fileAux;
        char filename[256];
        // Directory doesnt exist, so creates one
        if (stat(directory, &st) == -1) {
            mkdir(directory, 0700);
        }
        // Directory already exists
        else if ((dir = opendir (directory)) != NULL) {
            while ((ent = readdir (dir)) != NULL) {
                strcpy(filename, ent->d_name);
                FILE* newFile = fopen(ent->d_name, "r");
                if((newFile) && (strlen(filename) > 3)){
                   size = file_size(newFile);
                   fileAux = findFile(user,filename);
                   if(fileAux == NULL){
                        fileAux = createFile(filename, size);
                        if(fileAux == NULL)
                            break;
                        else    if(!AppendFila2(user->files, fileAux))
                            printf ("File %s successfully added to client %s\n", filename, user->userid);
                   }
                }
                fclose(newFile);
            }
            closedir (dir);
        }
        return 1;
    }
    else
        return 0;   
}

CLIENT* getClient(char* user){
	pthread_mutex_lock(&mutexClientRegister);
    CLIENT* newClient = findClient(clientsList, user);
    if(!newClient){
        newClient = createClient(user); 
        AppendFila2(clientsList, newClient);
    }

	pthread_mutex_unlock(&mutexClientRegister);
    return newClient;

}
//Sends file for client to download
void send_file(char*file, int socket, CLIENT* user){
    char *path = getPath(user, file);
	FILE *fp = fopen(path, "r");
	int n, i;
	long int size = file_size(fp);
	char buffer, newBuffer[256];
	// Auxiliar to help casting the file size to buffer
	unsigned char* bufferSize;

	if(fp == NULL) {
		printf("File doesn't exist. Please specify a valid file name %ld \n", size);
        bufferSize = (unsigned char*) &size;
        n = write(socket, (void*)bufferSize, 4);
        if (n < 0)
            printf("Connection error");
		return;
	}
	// Valid file, starts the stream to the server
	else {
		//printf("Tamanho do arquivo %ld\n", size);  // CUIDAR LITTLE ENDIAN E BIG ENDIAN PQ O ALBERTO VAI RECLAMAR
		// First passes the size of the file to the server
		bufferSize = (unsigned char*) &size;
		n = write(socket, (void*)bufferSize, 4);
		if (n > 0) {
			for(i = 0; i < size; i++) {
				buffer = fgetc(fp);
				n = write(socket, (void*)&buffer, 1);
				if (n < 0)
					break;
			}
		}
		//Finished passing the file, closes the file and
		// wait for servers answer
		printf("File %s upload is finished.\n", file);
		fclose(fp);
		n = read(socket, newBuffer, 256);
		printf("Client answered: %s\n", newBuffer);
	}

}

void* server_loop(void* oldSocket){
	char buffer[256];
    char* fileName;
	int n;
	int socket = *((int *) oldSocket);
    char* username;
	CLIENT* user;
	while(1){
        /* read from the socket */
        bzero(buffer, 256);
        n = read(socket, (void*)buffer, 256);
        if (n < 0){
            printf("ERROR reading from socket");
            close(socket);
            return (void*)-1;
        }
        if(strstr(buffer, "login")){
            username = strtok(buffer, " ");
            username = strtok(NULL, " ");

            user = getClient(username);
            if(!login_user(user, socket)) {
                // Signals the client that his connection was not permitted
                n = write(socket, "FAIL", sizeof("FAIL"));
                strcpy(buffer, "exit");
            }
            else {
                // Everythings is ok while logging the user!
                n = write(socket, "OK", sizeof("OK"));
            }
        }
        else if(strstr(buffer, "upload")) { 
            fileName = parseFilename(buffer);
            receive_file(fileName, socket, user);
	        }
	    else if(strstr(buffer, "download")){
	    	fileName = parseFilename(buffer);
            send_file(fileName, socket, user);
	    }
	    else if (strstr(buffer, "exit")){
	    	printf("Closing client with id %d\n", socket);
    		close(socket);
            deregisterSocket(socket, user);
    		pthread_exit(0);
    		break;
	    }
	    else if (strstr(buffer, "list")){
	    	list_files(socket, user);
	    }

        else if (strstr(buffer, "timeserver")){
            printf("chega aqui?\n");
            timeServer(socket);
        }

	}
	return (void*) 1;
}


int main(int argc, char *argv[])
{
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_t tid[10];
    int threadCount = 0;
    int port;
	int *arg = (int*)malloc(sizeof(*arg));
    // Criação da lista de clientes geral do servidor
    clientsList = (PFILA2) malloc(sizeof(FILA2));
    CreateFila2(clientsList);

    // Criação do mutex de registro de sockets
    pthread_mutex_init(&mutexDevicesRegister, NULL);
    // Criação do mutex de registro de novos clientes (Sugestao do prof.)
    pthread_mutex_init(&mutexClientRegister, NULL);
    //pid_t pid;

    if(argc < 2){
        printf("Use ./server port-num to start the server\n");
        return 0;
    }

    port = atoi(argv[1]);

    //Opens Socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("ERROR opening socket\n");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);
    
    //Binds Socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("ERROR on binding\n");
        return -1;
    }
    //Server in listen state
    while(1){
	    listen(sockfd, 5);
	    clilen = sizeof(struct sockaddr_in);
	    if ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1){
	        printf("ERROR on accept\n");
	    }
	    else {
    		*arg = newsockfd;
            printf("SOCKET %d\n", newsockfd);
    		pthread_create(&tid[threadCount], NULL, server_loop, arg);
            threadCount++;
            printf("Criou a thread\n");
	    }
	}
    close(sockfd);
    return 0;
}
