#ifndef EMAILCONFIG_H
#define EMAILCONFIG_H

#include "mmetaconfig.h"

class EmailConfig : public MMetaConfig
{
    Q_OBJECT
    M_OBJECT(EmailConfig)
    M_MEMBER(int, timeout)
    M_MEMBER(int, port)
    M_MEMBER(QByteArray, host)
    M_MEMBER(QByteArray, user)
    M_MEMBER(QByteArray, password)
    // TODO (lkorbel) I copied that variable from existing code for compatibity
    // althogh RFC-4954 says AUTH LOGIN requires base64 always.
    // IMHO we can get rid of it, but my experience is limited to only 2 diffrent
    // smpt servers and maybe I dont know about something
    M_MEMBER_V(bool, base64Encoding, true)
 };

#endif  // EMAILCONFIG_H
