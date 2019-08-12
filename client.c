#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MAX_BUFFER_SIZE 7000
char* tmp_port;

void _mkdir(const char *path)
{
    char path_tmp[200];
    char temp[200];
    memset(path_tmp, 0, sizeof(path_tmp));
    strcat(path_tmp, "./output");
    strcat(path_tmp, path);
    //strcat(path_tmp, "/");
    //printf("path_out = %s\n", path_tmp);
    memset(temp, 0, sizeof(temp));
    char* ptr = NULL;

    strncpy(temp, path_tmp, sizeof(temp));
    for(ptr = temp+2; *ptr; ptr++) {
        if(*ptr == '/') {
            *ptr = '\0';
            mkdir(temp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            *ptr = '/';
        }
    }
    mkdir(temp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    //printf("create a dir !\n");
}

void* DFS_find(void* args)
{
    //printf("*in DFS_B\n");
    char *tmp = (char *)args;
    //printf("tmp = %s\n", tmp);
    //socket的建立
    int sockfd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("*Fail to create a socket.");
    }
    //printf("*socket thread = %d\n", sockfd);

    //socket的連線
    struct sockaddr_in info;
    memset(&info, 0, sizeof(info));
    info.sin_family = PF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(atoi(tmp_port));


    int err = connect(sockfd, (struct sockaddr *)&info, sizeof(info));
    if(err==-1) {
        printf("*Connection error\n");
    }
    //printf("give me connect!!!!!!!!!!\n");

    //create inputmsg
    char msg[100];
    memset(msg, 0, sizeof(msg));
    //strcat(msg, "/"); //////////////////////////////////////////////////////////////////////
    strcat(msg, tmp);
    strcat(msg, " HTTP/1.x\r\nHOST: 127.0.0.1:1234\r\n\r\n");
    //printf("*send msg in thread = %s\n", msg);

    send(sockfd, msg, sizeof(msg), 0);

    char receiveMessage[MAX_BUFFER_SIZE] = {};
    memset(receiveMessage, 0, sizeof(receiveMessage));
    recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
    printf("%s",receiveMessage);/////////////////////////////////////////
    printf("\n");

    close(sockfd);

    //copy recvmsg
    char recvmsg_tmp[MAX_BUFFER_SIZE];
    memset(recvmsg_tmp, 0, sizeof(recvmsg_tmp));
    strcat(recvmsg_tmp, receiveMessage);

    //cut recvmsg
    char* delim_n = "\n";
    char* context_str = NULL;
    char* file_str = NULL;
    char* file_state = NULL;
    file_state = strtok(recvmsg_tmp, delim_n);
    context_str = strtok(NULL, delim_n);
    strtok(NULL, delim_n);
    file_str = strtok(NULL, delim_n);
    // printf("context_str = %s\n", context_str);
    // printf("file_str = %s\n", file_str);

    ////state_type
    char* delim_space = " ";
    char* state_type = NULL;
    strtok(file_state, delim_space);
    state_type = strtok(NULL, delim_space);

    ////context_type
    char* delim_m = ": ";
    char* context_type = NULL;
    strtok(context_str, delim_m);
    context_type = strtok(NULL, delim_m);

    if(strcmp(state_type, "200") != 0) {
        //end
        //printf("error inputtttt\n");
    } else if(strcmp("directory", context_type) == 0) {
        //is a directory, find more files
        //printf("file_str = %s\n", file_str);
        char* delim_space = " ";
        char* file_DFS = NULL;
        file_DFS = strtok(file_str, delim_space);

        //make path to mkdir
        char copy_tmp[200];  //copy tmp
        memset(copy_tmp, 0, sizeof(copy_tmp));
        strcat(copy_tmp, tmp);

        char* mkdir_path_tmp = NULL;
        strtok(copy_tmp, delim_space);
        mkdir_path_tmp = strtok(NULL, delim_space);

        char mkdir_path[200];
        memset(mkdir_path, 0, sizeof(mkdir_path));
        strcat(mkdir_path, mkdir_path_tmp);
        _mkdir(mkdir_path);

        char *ptr[100];
        int i = 0;
        while(file_DFS != NULL) {

            ptr[i] = file_DFS;
            //printf("*in while ptr = %s\n", ptr[i]);
            file_DFS = strtok(NULL, delim_space);
            i++;

        }
        //printf("i in thread = %d\n", i);
        int k = i;
        pthread_t DFS_thread[100];
        memset(DFS_thread, 0, sizeof(DFS_thread));
        while(i != 0) {
            //GET ...
            char DFS_tmp[200];
            memset(DFS_tmp, 0, sizeof(DFS_tmp));
            strcat(DFS_tmp, tmp);
            strcat(DFS_tmp, "/");
            strcat(DFS_tmp, ptr[i-1]);
            //printf("in while\n");
            //printf("tmp = %s\n", tmp);
            //printf("DFS_tmp = %s\n", DFS_tmp);

            //create thread !!!!!!

            pthread_create(&DFS_thread[i-1], NULL, DFS_find, DFS_tmp);
            sleep(1);
            // printf("iiiii = %d\n", i);
            i--;
        }
        while(k != 0) {
            pthread_join(DFS_thread[k-1], NULL);
            k--;
        }

    } else {
        //is a file
        char* delim_space = " ";
        char* mkfile_path_tmp = NULL;
        strtok(tmp, delim_space);
        mkfile_path_tmp = strtok(NULL, delim_space);

        char mkfile_path[200];
        memset(mkfile_path, 0, sizeof(mkfile_path));
        strcat(mkfile_path, "./output");
        strcat(mkfile_path, mkfile_path_tmp);

        //copy recvmsg
        char recvmsg_file[MAX_BUFFER_SIZE];
        memset(recvmsg_file, 0, sizeof(recvmsg_file));
        strcat(recvmsg_file, receiveMessage);
        //printf("recvmsg_file = %s\n", recvmsg_file);

        //buffer create
        char buffer[MAX_BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        //do the text
        char* file_txt = NULL;
        char* delim_n_file = "\n";
        file_txt = strtok(recvmsg_file, delim_n_file);
        int n = 0;
        while(file_txt != NULL) {
            n++;
            if(n < 4) {
                file_txt = strtok(NULL, delim_n_file);
            } else {
                //printf("file_txt = %s\n", file_txt);
                strcat(buffer, file_txt);
                strcat(buffer, "\n");
                file_txt = strtok(NULL,delim_n_file);
            }

        }

        //printf("ffffff mkfile_path = %s\n", mkfile_path);
        FILE *fp = fopen(mkfile_path, "w+");
        fprintf(fp,"%s",buffer);
        fclose(fp);
        //printf("*end!!!!!\n");
    }
    pthread_exit(NULL);
    //return 0;
}

int main(int argc, char *argv[])
{
    char* tmp_argv = argv[2];
    tmp_port = argv[6];
    //socket的建立
    int sockfd = 0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("Fail to create a socket.");
    }

    //socket的連線
    struct sockaddr_in info;
    memset(&info, 0, sizeof(info));
    info.sin_family = PF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(atoi(tmp_port));


    int err = connect(sockfd, (struct sockaddr *)&info, sizeof(info));
    if(err==-1) {
        printf("Connection error\n");
    }
    //printf("give me connect!!!!!!!!!!\n");

    //cut inputmsg
    char message[100];
    memset(message, 0, sizeof(message));
    strcat(message, "GET ");
    //har tmp_argv[] = {"/example"};
    strcat(message, tmp_argv);  //argv[2]
    strcat(message, " HTTP/1.x\r\nHOST: 127.0.0.1:1234\r\n\r\n");
    //printf("msg = %s\n", message);

    //Send a message to server
    //char message[] = {"GET /secfolder/trifolder HTTP/1.x\r\nHost: localhost\r\n\r\n"};
    //printf("after msh1111\n");

    send(sockfd, message, sizeof(message), 0);

    //printf("after msh\n");
    //recv msg from server
    char receiveMessage[MAX_BUFFER_SIZE] = {};
    memset(receiveMessage, 0, sizeof(receiveMessage));
    recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
    printf("%s",receiveMessage);//////////////////////////////////////////////
    printf("\n");

    //close socket
    //printf("close Socket\n");
    close(sockfd);

    //cut recvmsg
    char recvmsg_tmp[MAX_BUFFER_SIZE];
    memset(recvmsg_tmp, 0, sizeof(recvmsg_tmp));
    strcat(recvmsg_tmp, receiveMessage);

    char* delim_n = "\n";
    char* context_str = NULL;
    char* file_str = NULL;
    char* file_state = NULL;
    file_state = strtok(recvmsg_tmp, delim_n);
    context_str = strtok(NULL, delim_n);
    strtok(NULL, delim_n);
    file_str = strtok(NULL, delim_n);

    //printf("file_state = %s\n", file_state);
    // printf("context_str = %s\n", context_str);
    // printf("file_str = %s\n", file_str);

    ////state_type
    char* delim_space = " ";
    char* state_type = NULL;
    strtok(file_state, delim_space);
    state_type = strtok(NULL, delim_space);
    //printf("state_type = %s\n", state_type);

    ////context_type
    char* delim_m = ": ";
    char* context_type = NULL;
    strtok(context_str, delim_m);
    context_type = strtok(NULL, delim_m);
    //printf("type = %s\n", context_type);

    //if directory
    if(strcmp(state_type, "200") != 0) {
        //end
        //printf("error inputtttt\n");
    } else if (strcmp("directory", context_type) == 0) {
        //is a directory, find more files
        //printf("file_str = %s\n", file_str);
        char* delim_space = " ";
        char* file_DFS = NULL;
        file_DFS = strtok(file_str, delim_space);

        //make path to mkdir
        char mkdir_path[200];
        memset(mkdir_path, 0, sizeof(mkdir_path));
        strcat(mkdir_path, tmp_argv);
        _mkdir(mkdir_path);

        char *ptr[100];
        int i = 0;
        while(file_DFS != NULL) {

            ptr[i] = file_DFS;
            //printf("%s\n", ptr[i]);
            file_DFS = strtok(NULL, delim_space);
            i++;

        }
        int k = i;
        pthread_t DFS_thread_m[100];
        memset(DFS_thread_m, 0, sizeof(DFS_thread_m));
        while(i != 0) {
            char *tmp = ptr[i-1];
            //GET ...
            char DFS_tmp[100];
            memset(DFS_tmp, 0, sizeof(DFS_tmp));
            strcat(DFS_tmp, "GET ");
            strcat(DFS_tmp, tmp_argv);
            strcat(DFS_tmp, "/");
            strcat(DFS_tmp, tmp);

            //printf("ans = %s\n",DFS_tmp);

            //create thread !!!!!!
            pthread_create(&(DFS_thread_m[i-1]), NULL, DFS_find, DFS_tmp);

            sleep(1);
            // printf("iiiii = %d\n", i);
            i--;
        }
        while(k != 0) {
            pthread_join(DFS_thread_m[k-1], NULL);
            k--;
        }

    } else {
        //is a file
        //make path to mkdir
        char mkfile_path[200];
        memset(mkfile_path, 0, sizeof(mkfile_path));
        _mkdir(mkfile_path);
        //printf("mkfile_path = %s\n", mkfile_path);

        //copy recvmsg
        char recvmsg_file[MAX_BUFFER_SIZE];
        memset(recvmsg_file, 0, sizeof(recvmsg_file));
        strcat(recvmsg_file, receiveMessage);
        //printf("recvmsg_file = %s\n", recvmsg_file);

        //buffer create
        char buffer[2000];
        memset(buffer, 0, sizeof(buffer));

        //do the text
        char* file_txt = NULL;
        char* delim_n_file = "\n";
        file_txt = strtok(recvmsg_file, delim_n_file);
        int n = 0;
        while(file_txt != NULL) {
            n++;
            if(n < 4) {
                file_txt = strtok(NULL, delim_n_file);
            } else {
                //printf("file_txt = %s\n", file_txt);
                strcat(buffer, file_txt);
                strcat(buffer, "\n");
                file_txt = strtok(NULL,delim_n_file);
            }

        }

        strcat(mkfile_path, "./output");
        strcat(mkfile_path, tmp_argv);
        FILE *fp = fopen(mkfile_path, "w+");
        fprintf(fp,"%s",buffer);
        fclose(fp);

        //printf("end!!!!!\n");
    }
    return 0;
}