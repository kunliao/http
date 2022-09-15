#! /bin/sh

#生成CA的私钥
openssl genrsa -out cert/rootCA.key 2048

#生成CA的证书
openssl req -x509 -new -nodes -key cert/rootCA.key -sha256 -days 3650 -out cert/rootCA.pem -subj "/C=CN/ST=Beijing/L=Beijing/O=LK Technology Co. Ltd./OU=LK Technology Co. Ltd./CN=LK Technology Root CA"

#生成服务端用的私钥
openssl genrsa -out cert/server.key 2048

#生成服务端的csr文件，ca根据csr文件为服务端颁发证书
openssl req -new -key cert/server.key -out cert/server.csr -subj "/C=CN/ST=Beijing/L=Beijing/O=LK/OU=LK/CN=test.com"


#ca根据csr文件为服务端颁发证书
openssl x509 -req -in cert/server.csr -CA cert/rootCA.pem -CAkey cert/rootCA.key -CAcreateserial -out cert/server.pem -days 825 -sha256 -extfile cert/v3.ext
#SSL证书最长期限825天