#ifndef EMAILCONFIG_H
#define EMAILCONFIG_H

#include "mconfig/mconfig.h"

class EmailConfig : public MConfig
{
 public:
    EmailConfig();
    int timeout;
    int port;
    QByteArray host;
    QByteArray user;
    QByteArray password;
};

#endif  // EMAILCONFIG_H
