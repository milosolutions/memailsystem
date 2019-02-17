#ifndef EMAILCONFIG_H
#define EMAILCONFIG_H
#include <QString>
namespace Email {

class EmailConfig 
{
 public:
    EmailConfig();
    int timeout;
    quint16 port;
    QString host;
    QString user;
    QString password;
    bool base64Encoding = true;
};


};
#endif  // EMAILCONFIG_H
