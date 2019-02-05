#ifndef EMAILCONFIG_H
#define EMAILCONFIG_H
#include <QString>
namespace Email {

class EmailConfig 
{
 public:
    EmailConfig();
    int timeout;
    int port;
    QString host;
    QString user;
    QString password;
};


};
#endif  // EMAILCONFIG_H
