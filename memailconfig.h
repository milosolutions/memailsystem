#ifndef EMAILCONFIG_H
#define EMAILCONFIG_H

#include "mconfig/mconfig.h"

class EmailConfig : public MConfig
{
 public:
    EmailConfig();
    int timeout;
    int port;
    QString host;
    QString user;
    QString password;
};

#endif  // EMAILCONFIG_H
