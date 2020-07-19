#include<netinet/in.h>  // sockaddr_in
#include<sys/types.h>   // socket
#include<sys/socket.h>  // socket
#include<stdio.h>       // printf
#include<stdlib.h>      // exit
#include<string.h>      // bzero
 
#define SERVER_PORT 8000
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512
#define CMD_SIZE 10
void send_file(int new_server_socket_fd,char* file_name)
{
    char buffer[BUFFER_SIZE];
    // 打开文件并读取文件数据
    FILE *fp = fopen(file_name, "r");
    if(NULL == fp)
    {
        printf("File:%s Not Found\n", file_name);
        return;
    }
    else
    {
        bzero(buffer, BUFFER_SIZE);
        int length = 0;
        // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止
        while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0)
        {
            if(send(new_server_socket_fd, buffer, length, 0) < 0)
            {
                printf("Send File:%s Failed./n", file_name);
                break;
            }
            bzero(buffer, BUFFER_SIZE);
        }

        // 关闭文件
        fclose(fp);
        printf("File:%s Transfer Successful!\n", file_name);
    }
}

void recv_file(int new_server_socket_fd,char* file_name)
{
    char buffer[BUFFER_SIZE];
    // 打开文件，准备写入
    FILE *fp = fopen(file_name, "w");
    if(NULL == fp)
    {
        printf("File:\t%s Can Not Open To Write\n", file_name);
        return;
    }
 
    // 从服务器接收数据到buffer中
    // 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止
    bzero(buffer, BUFFER_SIZE);
    int length = 0;
    while((length = recv(new_server_socket_fd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        if(fwrite(buffer, sizeof(char), length, fp) < length)
        {
            printf("File:\t%s Write Failed\n", file_name);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
    }
 
    // 接收成功后，关闭文件，关闭socket
    printf("Receive File:\t%s From Client Successful!\n", file_name);
    fclose(fp);
    close(new_server_socket_fd);
}

int main(void)
{
    // 声明并初始化一个服务器端的socket地址结构
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
 
    // 创建socket，若成功，返回socket描述符
    int server_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(server_socket_fd < 0)
    {
        perror("Create Socket Failed:");
        exit(1);
    }
    int opt = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
 
    // 绑定socket和socket地址结构
    if(-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
    {
        perror("Server Bind Failed:");
        exit(1);
    }
    
    // socket监听
    if(-1 == (listen(server_socket_fd, LENGTH_OF_LISTEN_QUEUE)))
    {
        perror("Server Listen Failed:");
        exit(1);
    }
 
    while(1)
    {
        // 定义客户端的socket地址结构
        struct sockaddr_in client_addr;
        socklen_t client_addr_length = sizeof(client_addr);
 
        // 接受连接请求，返回一个新的socket(描述符)，这个新socket用于同连接的客户端通信
        // accept函数会把连接到的客户端信息写到client_addr中
        int new_server_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_length);
        if(new_server_socket_fd < 0)
        {
            perror("Server Accept Failed:");
            break;
        }

        // recv函数接收数据到缓冲区buffer中
        char cmd_buffer[CMD_SIZE];
        bzero(cmd_buffer, CMD_SIZE);
        if(recv(new_server_socket_fd, cmd_buffer, BUFFER_SIZE, 0) < 0)
        {
            perror("Server Recieve cmd Failed:");
            continue;
        }

        // recv函数接收数据到缓冲区buffer中
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        if(recv(new_server_socket_fd, buffer, BUFFER_SIZE, 0) < 0)
        {
            perror("Server Recieve Data Failed:");
            continue;
        }

        // 然后从buffer(缓冲区)拷贝到file_name中
        char file_name[FILE_NAME_MAX_SIZE+1];
        bzero(file_name, FILE_NAME_MAX_SIZE+1);
        strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer));
        printf("%s\n", file_name);


        if(0 == strcmp(cmd_buffer,"download"))
        {
            printf("download\n");
            send_file(new_server_socket_fd,file_name);
        }
        else if(0 == strcmp(cmd_buffer,"upload"))
        {
            printf("upload\n");
            recv_file(new_server_socket_fd,file_name);
        }
        else
        {
            printf("invalid cmd");
            continue;
        }

        // 关闭与客户端的连接
        close(new_server_socket_fd);
    }
    // 关闭监听用的socket
    close(server_socket_fd);
    return 0;
}
