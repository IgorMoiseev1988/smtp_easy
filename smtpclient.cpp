#include "smtpclient.h"

#include <cassert>
#include <cctype>       //std::isdigit
#include <cstdlib>      //std::atoi
#include <iostream>     //std::cerr

#include <arpa/inet.h>  //socket, htons

#include <openssl/err.h>
#include <netdb.h>      //hostent, gethostbyname
#include <netinet/in.h> //sockaddr_in, htons

#include <sys/types.h>  //socket, send, recv, sockaddr
#include <sys/socket.h> //socket, send, recv, sockaddr

#include <string.h>     //memcpy
#include <unistd.h>     //read

constexpr int BUFF_SIZE = 512;

SMTPClient::SMTPClient(const std::string& hostname, int port
                       , const std::string& login, const std::string& pass)
    : hostname_(hostname)
    , port_(port)
    , login_(login)
    , pass_(pass)
    , ssl_connect(false)
{}

bool SMTPClient::SendMail(const Email mail) {
    if (!StartConnect()) {
        std::cerr << "Connect to server fail\n";
        return false;
    }
    std::cout << "Connect to server OK\n";
    if (!StartHandshake(mail.getDomain())) {
        std::cerr << "Handshake with server fail\n";
        return false;
    }
    std::cout << "HandShake with server OK\n";
    if (SendAndReadCode("MAIL FROM: " + mail.getFrom() + "\r\n") != 250) {
        std::cerr << "Bad answer for MAIL FROM\n";
        return false;
    }

    for (size_t rcpts = mail.getToCount(); rcpts > 0; --rcpts) {
        if (SendAndReadCode("RCPT TO: " + mail.getTo(rcpts - 1) + "\r\n") != 250) {
            std::cerr << "Bad answer for RCPT TO\n";
            return false;
        }
    }

    if (SendAndReadCode("DATA\r\n") != 354) {
        std::cerr << "Server not read to get data\n";
        return false;
    }

    for (const auto& line : mail.getDATA()) {
        if(!SockSend(line)) return false;
    }
    if (ReadCode(SockRead()) != 250) {
        std::cerr << "Server not send msg\n";
        return false;
    }

    if(SendAndReadCode("quit\r\n") != 221) {
        std::cerr << "Server not bye\n";
        return false;
    }
    std::cout << "Send msg success\n";
    return true;
}

bool SMTPClient::StartSSL() {
    if(!InitCtx()) return false;
    ssl_socket_ = SSL_new(ctx_);
    SSL_set_fd(ssl_socket_, server_socket_);
    int state = SSL_connect(ssl_socket_);
    if (state < 1) {
        std::cout << "SSL connect return error\n";
        return false;
    }
    return true;
}

bool SMTPClient::InitCtx() {
    const SSL_METHOD* method;
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = TLSv1_2_method();
    ctx_ = SSL_CTX_new(method);
    if (ctx_ == NULL) {
        std::cerr << "SSL_STX_new() faild\n";
        return false;
    }
    return true;
}

bool SMTPClient::StartConnect() {

    if ((server_socket_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Call socket() fail\n";
        return false;
    }

    struct hostent* host = gethostbyname(hostname_.c_str());
    std::cout << "hostname:" << hostname_ << std::endl;
    if (host == NULL) {
        std::cerr << "Call gethostbyname faild\n";
        return false;
    }

    struct sockaddr_in addr;
    memcpy(&addr.sin_addr, host->h_addr_list[0], host->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);

    int res = connect(server_socket_, (struct sockaddr*)&addr, sizeof (addr));
    if (res < 0) {
        std::cerr << "Call connect fail\n";
        return false;
    }
    return true;
}

bool SMTPClient::StartHandshake(const std::string domain) {
    const std::string EHLO = "EHLO <" + domain + ">\r\n";
    const std::string LOGIN = login_ + "\r\n";
    const std::string PASS = pass_ + "\r\n";
    if (ReadCode(SockRead()) != 220) {
        std::cerr << "Server not ready\n";
        return false;
    }
    //send EHLO
    if (!SockSend(EHLO)) return false;
    if (SockReadHasStartTls()) {
        //send STARTTLS and init tls connect
        if (SendAndReadCode("STARTTLS\r\n") != 220) {
            std::cerr << "STARTTLS fail on remote server\n";
        }
        if (StartSSL()) {
            ssl_connect = true;
            std::cout << "SSL connect success\n";
        } else {
            return false;
        }
    } else {
        std::cerr << "Server not support TLS\n";
    }
    //send AUTH LOGIN
    if (SendAndReadCode("AUTH LOGIN\r\n") != 334) {
        std::cerr << "Bad answer for AUTH LOGIN\n";
        return false;
    }
    //send login
    if (SendAndReadCode(login_ + "\r\n") != 334) {
        std::cerr << "Login no accept\n";
        return false;
    }
    //send pass
    if (SendAndReadCode(pass_ + "\r\n") != 235) {
        std::cerr << "Authorization faild\n";
        return false;
    }
    return true;
}

std::string SMTPClient::SockRead() {
    char buff[BUFF_SIZE];
    memset(buff, 0, BUFF_SIZE);
    if (ssl_connect) {
        if ((SSL_read(ssl_socket_, buff, BUFF_SIZE)) <= 0) {
            std::cerr << "Error while read from ssl_socket\n";
        }
    } else {
        if ((recv(server_socket_, buff, BUFF_SIZE, 0)) < 0) {
            std::cerr << "Error while read from socket\n";
        }
    }
    std::cout << "SSL: " << ssl_connect << "READ: " << buff << "\n";
    return std::string(buff);
}

int SMTPClient::ReadCode(const std::string& buff) {
    if (std::isdigit(buff[0])) {
        return std::atoi(buff.c_str());
    }
    return 0;
}

bool SMTPClient::SockReadHasStartTls() {
    sleep(1); // wait while data come
    std::string buff = SockRead();
    if (ReadCode(buff) != 250) {
        std::cerr << "Server not return 250 code\n";
        return false;
    }
    return buff.find("STARTTLS") != std::string::npos;
}

bool SMTPClient::SockSend(const std::string &data) {
    if (ssl_connect) {
        if (SSL_write(ssl_socket_, data.c_str(), data.size()) <= 0) {
            std::cerr << "SSL send fail\n";
            return false;
        }
    } else {
        if (send(server_socket_, data.c_str(), data.size(), 0) < 0) {
            std::cerr << "Send fail\n";
            return false;
        }
    }
    std::cout << "SSL: " << ssl_connect << "SEND: " << data << "\n";
    return true;
}

int SMTPClient::SendAndReadCode(const std::string &data) {
    if (!SockSend(data)) return -1;
    return ReadCode(SockRead());
}
