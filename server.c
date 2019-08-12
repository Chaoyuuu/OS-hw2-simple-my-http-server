#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "status.h"
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <arpa/inet.h>


char str_head[100];
//task queue
struct node {
    int data;
    struct node* next;
};

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
struct node * Q_head = NULL;
struct node * Q_tail = NULL;
int Q_num = 0;

void Push(int data)
{
    if(Q_head == NULL) {
        Q_head = (struct node*)malloc(sizeof(struct node));
        Q_head->data = data;
        Q_head->next = NULL;
        Q_tail = Q_head;
    } else {
        struct node * ptr = (struct node*)malloc(sizeof(struct node));
        ptr->data = data;
        ptr->next = NULL;
        Q_tail->next = ptr;
        Q_tail = ptr;
    }
    Q_num++;
}

int Pop(void)
{
    struct node * ptr = Q_head;
    int result = ptr->data;
    Q_head = ptr->next;
    free(ptr);
    Q_num--;
    return result;
}

int isEmpty(void)
{
    if(Q_num == 0) return 1;
    else return 0;
}

int file_exist(char *pathname)
{

    struct stat sb;
    // printf("pathname_tmp = %s\n", pathname);
    // char pathname[100];
    // memset(pathname, 0, sizeof(pathname));
    // strcat(pathname, "./");
    // strcat(pathname, pathname_tmp);
    if (stat(pathname, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        //Directory
        return 0;
    } else if(stat(pathname, &sb) == 0 && S_ISREG(sb.st_mode)) {
        //File
        return 0;
    } else if(stat(pathname, &sb)!= 0) {
        //Not Found!
        return 1;
    }
    return 1;
}

int type_exist(char *msg_type)
{
    //printf("1!!!!!!!!!!!!!!\n");
    //printf("%s\n", msg_type);
    //printf("%d\n",strlen(msg_type) );
    if(msg_type == NULL) {
        // printf("!!!!!!!!!!!!!!!!!!\n");
        return 0; //not file
    }
    for(int i=0; i<8; i++) {
        //printf("2!!!!!!!!!!!!!!\n");
        if(strcmp(extensions[i].ext, msg_type) == 0) {
            return 0; //find
        }
    }
    //not found
    //printf("3!!!!!!!!!!!!!!\n");
    return 1;
}
void what_state(char* msg, char* path, char* output_str, int* s)
{
    // printf("inin = %s\n", msg); //GET /exmaple.ppp HTTP...
    // printf("papa = %s\n", path);  //GET /example.pp  testdir/example.ppp
    //int index = 0;
    char* delim_space = " ";
    char* delim_dot = ".";
    char* delim_s = "/";
    char* msg_slash = NULL;
    char* msg_get = NULL;
    char* msg_type = NULL;
    char msg_exist[100];
    char* tmp = NULL;

    memset(msg_exist, 0, sizeof(msg_exist));
    strcat(msg_exist, path);

    msg_get = strtok(msg, delim_space);   //msg_get = GET
    msg_slash = strtok(NULL, delim_space);
    tmp = strrchr(path, '/');
    tmp = strtok(tmp, delim_s);
    strtok(tmp, delim_dot);
    msg_type = strtok(NULL, delim_dot);

    // printf("msg_slash = %s\n", msg_slash);
    // printf("msg_gst = %s\n", msg_get);
    // printf("msg_type = %s\n", msg_type);
    // printf("msg_exist = %s\n", msg_exist);

    if(strncmp(msg_slash, "/", 1) != 0) {
        //index = 400;
        strcat(output_str, "HTTP/1.x 400 BAD_REQUEST\n");
        *s = 0;
    } else if(strcmp(msg_get, "GET") != 0) {
        //index = 405;
        strcat(output_str, "HTTP/1.x 405 METHOD_NOT_ALLOWED\n");
        *s = 0;
    } else if(type_exist(msg_type) != 0) {
        //index = 415;
        strcat(output_str, "HTTP/1.x 415 UNSUPPORT_MEDIA_TYPE\n");
        *s = 0;
    } else if(file_exist(msg_exist) != 0) {
        //index = 404;
        strcat(output_str, "HTTP/1.x 404 NOT_FOUND\n");
        *s = 0;
    } else {
        //index = 200;
        strcat(output_str, "HTTP/1.x 200 OK\n");
        *s = 1;
    }
    //printf("s = %d\n", s);

}

void what_type(char* filename, char* output_str)
{
    //printf("hiiiiiiii\n");
    //cut file_type
    char* delim_dot = ".";
    char* str = NULL;
    str = strtok(filename, delim_dot);
    str = strtok(NULL,delim_dot);
    //printf("file_type = %s\n", str);

    for(int i=0; i<8; i++) {
        if(strcmp(extensions[i].ext, str) == 0) {
            strcat(output_str, extensions[i].mime_type);
        }
        //printf("==%s\n", extensions[i].mime_type);
    }
    strcat(output_str, "\n");
}

void find_more_file(char* path, char* output_str)
{
    DIR *d;
    struct dirent* dir;
    char file_path[200];
    memset(file_path, 0,sizeof(file_path));
    strcat(file_path, "./");
    strcat(file_path, path);
    //printf("file path = %s\n", file_path);

    d = opendir(file_path);//("./testdir");

    if(d) {
        while((dir = readdir(d))!=NULL) {
            if(strcmp(dir->d_name,".") && strcmp(dir->d_name,"..")) {
                printf("dir = %s\n", dir->d_name);
                strcat(output_str, dir->d_name);
                strcat(output_str, " ");
            }
        }
        closedir(d);
    }
    //strcat(output_str, "\n");
}

void read_file(char* path, char* output_str)
{
    //printf("in read_file\n");
    char buffer[5000];
    memset(buffer, 0, sizeof(buffer));
    char ch;
    int i = 0;
    FILE *file;
    file = fopen(path,"r");
    while ((ch=fgetc(file)) != EOF) {
        buffer[i] = ch;
        i++;
    }
    fclose(file);

    strcat(output_str, buffer);
    /*
    if (buffer) {
        //printf("%s",buffer);
    }
    */
}

void* mainthread_func(void* args)
{
    printf("in main thread\n");
    return 0;
}

void* threadpool_thread(void* args)
{
    int num = *(int*)args;
    printf("s* %d\n", num);
    while(1) {
        pthread_mutex_lock(&lock);
        while(isEmpty()) {
            pthread_cond_wait(&cond, &lock);

        }
        //printf("hi thread %d.\n", num);
        int inputSockfd = Pop();
        //printf("S inputsocket = %d\n", inputSockfd);
        char inputBuffer[1000] = {};
        memset(inputBuffer, 0, sizeof(inputBuffer));
        recv(inputSockfd,inputBuffer,sizeof(inputBuffer),0);
        //printf("S inputBuffer22 = %s\n", inputBuffer);

        pthread_mutex_unlock(&lock);
        //do something

        //copy inputbuffer to input_tmp
        char buffer_tmp[256];
        memset(buffer_tmp, 0, sizeof(buffer_tmp));
        strcat(buffer_tmp, inputBuffer);


        //creat output msg
        char output_msg[10000];
        memset(output_msg, 0, sizeof(output_msg));


        //cut inputBuffer (tmp)
        char* delim = " ";
        char* tmp = NULL;
        tmp = strtok(inputBuffer, delim);
        tmp = strtok(NULL, delim);
        //printf("tmp = %s\n", tmp);

        //link str_head
        char pathname[100];
        char path_in_status[100];
        memset(pathname, 0, sizeof(pathname));
        memset(path_in_status, 0, sizeof(path_in_status));
        //strcat(pathname, "~");
        strcat(pathname, str_head);
        strcat(pathname, tmp);
        //printf("S target = %s\n", pathname);
        strcat(path_in_status, pathname);

        //what status
        int status = 1;
        //printf("inin = %s\n", buffer_tmp);
        printf("papa = %s\n", pathname);
        what_state(buffer_tmp,path_in_status, output_msg, &status);
        //printf("outoo = %s\n", pathname);

        if(status == 0) {
            printf("error input bad bad\n");
            strcat(output_msg, "Content-type: \nServer: httpserver/1.x\n\n");
            //printf("oooo = %s\n", output_msg);
            //pthread_exit(NULL);
        } else {
            printf("you alife\n");
            //determine a file or a directory

            struct stat sb;
            if(stat(pathname, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                //Directory
                printf("S this is a directory\n");
                strcat(output_msg, "Content-type: directory\nServer: httpserver/1.x\n\n");
                find_more_file(pathname, output_msg);
                printf("output_msggggg =\n %s\n", output_msg);

            } else if(stat(pathname, &sb) == 0 && S_ISREG(sb.st_mode)) {
                //File
                printf("S this is a file\n");
                //copy pathname
                char path_tmp[100];
                memset(path_tmp, 0, sizeof(path_tmp));
                strcat(path_tmp, pathname);

                //cut filename
                char* delim_slash = "/";
                char* str = NULL;
                char filename[100];
                str = strtok(path_tmp, delim_slash);
                while(str!=NULL) {
                    //printf("cut = %s\n",str);
                    memset(filename, 0, sizeof(filename));
                    strcat(filename, str);
                    str = strtok(NULL, delim_slash);
                }
                printf("filename = %s\n",filename );

                strcat(output_msg, "Content-type: ");
                what_type(filename, output_msg);
                strcat(output_msg, "Server: httpserver/1.x\n\n");


                read_file(pathname, output_msg);
                printf("S out = %s\n", output_msg);

            } else if(stat(pathname, &sb)!= 0) {
                //NOT found
                printf("nothing\n");
            }
        }

        //printf("S output_msg = %s\n", output_msg);
        send(inputSockfd, output_msg, sizeof(output_msg), 0);
        //pthread_exit(NULL);

    }

}



int main(int argc, char *argv[])
{
    //get input argv
    char* tmp_root = argv[2];
    char* tmp_port = argv[4];
    char* tmp_thread_num = argv[6];
    memset(str_head, 0, sizeof(str_head));
    strcat(str_head, tmp_root);

    //socket的建立
    int sockfd = 0,forClientSockfd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }

    //socket的連線
    struct sockaddr_in serverInfo,clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
    memset(&serverInfo, 0, sizeof(serverInfo));

    serverInfo.sin_family = PF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");//INADDR_ANY;
    serverInfo.sin_port = htons(atoi(tmp_port));

    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd,100);

    //create threadpool
    int thread_count = atoi(tmp_thread_num);
    pthread_t *thread_pool = (pthread_t *)malloc(sizeof(pthread_t)* thread_count);
    for(int i=0; i<thread_count; i++) {
        pthread_create(&(thread_pool[i]), NULL, threadpool_thread, (void *)&i);

        usleep(2000);
    }

    //create mainthread
    pthread_t mainthread;
    pthread_create(&mainthread, NULL, mainthread_func, NULL);

    while(1) {
        forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
        //send(forClientSockfd,message,sizeof(message),0);
        //printf("socket  = %d\n", forClientSockfd);

        pthread_mutex_lock(&lock);
        Push(forClientSockfd);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
        printf("in server\n");

    }
    printf("out of while\n");
    pthread_join(*thread_pool, NULL);


    return 0;
}