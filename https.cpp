#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#define port 443 //监听端口，可以在范围内自由设定
using namespace std;



void close_ssl(SSL *ssl,int client_fd)
{
    if(ssl)
    {
        /* 关闭 SSL 连接 */
        SSL_shutdown(ssl);
        /* 释放 SSL */
        SSL_free(ssl);

    }

    if(client_fd>0)
    {
        close(client_fd);
    }
}


int main()
{
    std::string message ="";
    message+="HTTP/1.1 200 OK";
    message+="\r\n";
    message+="Content-Type:text/html\r\n";
    message+="server:Tengine \r\n";
    message+="name:LiaoKun \r\n";
    message+="\r\n";
    message+="<html><head>Hello,World!</head></html>\r\n";
    message+="\r\n";
    SSL_CTX *ctx;

    /* SSL 库初始化 */
    SSL_library_init();
    /* 载入所有 SSL 算法 */
    OpenSSL_add_all_algorithms();
    /* 载入所有 SSL 错误消息 */
    SSL_load_error_strings();
    /* 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX ，即 SSL Content Text */
    ctx = SSL_CTX_new(SSLv23_server_method());
    /* 也可以用 SSLv2_server_method() 或 SSLv3_server_method() 单独表示 V2 或 V3标准 */
    if (ctx == NULL) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    // 双向验证
    // SSL_VERIFY_PEER---要求对证书进行认证，没有证书也会放行
    // SSL_VERIFY_FAIL_IF_NO_PEER_CERT---要求客户端需要提供证书，但验证发现单独使用没有证书也会放行
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
//    SSL_CTX_set_security_level(ctx,0);

    /* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
    if ( SSL_CTX_use_certificate_file(ctx, "/Users/liaokun/CLionProjects/http/cert/server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    /* 载入用户私钥 */
    if (SSL_CTX_use_PrivateKey_file(ctx, "/Users/liaokun/CLionProjects/http/cert/server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    /* 检查用户私钥是否正确 */
    if (!SSL_CTX_check_private_key(ctx)) {
        ERR_print_errors_fp(stdout);
        exit(1);
    }

    //1.创建一个socket套接字
    int local_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (local_fd == -1)
    {
        cout << "socket error!" << endl;
        exit(-1);
    }
    cout << "socket ready!" << endl;

    //2.sockaddr_in结构体：可以存储一套网络地址（包括IP与端口）,此处存储本机IP地址与本地的一个端口
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);  //绑定6666端口
    local_addr.sin_addr.s_addr =  INADDR_ANY ; //绑定本机IP地址

    //3.bind()： 将一个网络地址与一个套接字绑定，此处将本地地址绑定到一个套接字上
    int res = bind(local_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (res == -1)
    {
        cout << "bind error!" << endl;
        exit(-1);
    }
    cout << "bind ready!" << endl;

    //4.listen()函数：监听试图连接本机的客户端
    //参数二：监听的进程数
    listen(local_fd, 10);
    cout << "等待来自客户端的连接...." << endl;
    SSL *ssl;
    while (true)//循环接收客户端的请求
    {
        //5.创建一个sockaddr_in结构体，用来存储客户机的地址
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        //6.accept()函数：阻塞运行，直到收到某一客户机的连接请求，并返回客户机的描述符
        int client_fd = accept(local_fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd == -1)
        {
            cout << "accept错误\n"<< endl;

            exit(-1);
        }

        /* 基于 ctx 产生一个新的 SSL */
        ssl = SSL_new(ctx);
        /* 将连接用户的 socket 加入到 SSL */
        SSL_set_fd(ssl, client_fd);
        /* 建立 SSL 连接 */
        if (SSL_accept(ssl) == -1) {
            perror("accept");
            printf("SSL 连接失败!\n");
            close_ssl(ssl,client_fd);
            break;
        }
        //7.输出客户机的信息
        char *ip = inet_ntoa(client_addr.sin_addr);
        cout << "客户机： " << ip << " 连接到本服务器成功!" << endl;

        //8.输出客户机请求的信息
        char buff[1024] = {0};
        int size = SSL_read(ssl, buff, sizeof(buff));
        cout << "Request information:\n"
             << buff << endl;
        cout << size << " bytes" << endl;

        //9.使用第6步accept()返回socket描述符，即客户机的描述符，进行通信。
        SSL_write(ssl, message.c_str(), message.length());//返回message

        close_ssl(ssl,client_fd);
    }
    /* 释放 CTX */
    SSL_CTX_free(ctx);

    close(local_fd);
    return 0;
}

