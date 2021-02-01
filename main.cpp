#include "smtpclient.h"
#include "email.h"

int main() {
    SMTPClient smtp("smtp.mailtrap.io", 2525, "MTQ0ZDI5Y2JiYjVkOTc=", "NGIxMzc2ZTZmNGM2ZTI=");
    Email mail("i@gmail.com", "to@gmail.com", "Hello", "text of msg is here");
    //Start_connect("smtp.mailtrap.io", 2525);
    mail.addTo("to2@gmail.com");
    smtp.SendMail(mail);
}
