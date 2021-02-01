#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <string>
#include <openssl/ssl.h>

#include "email.h"



class SMTPClient {
public:
    SMTPClient(const std::string& hostname, int port,
               const std::string& login, const std::string& pass);
    bool SendMail(const Email mail);
private:
    const std::string hostname_;
    const int port_;
    const std::string login_;
    const std::string pass_;
    int server_socket_;
    bool ssl_connect;

    SSL* ssl_socket_;
    SSL_CTX* ctx_;

    bool StartSSL();
    bool InitCtx();
    bool StartConnect();
    bool StartHandshake(const std::string domain);
    std::string SockRead();
    int ReadCode(const std::string& buff);
    bool SockReadHasStartTls();
    bool SockSend(const std::string& data);
    int SendAndReadCode(const std::string& data);

};

#endif
