#include "memailconfig.h"

EmailConfig::EmailConfig() : MConfig("email")
{
    CONFIG_VALUE(timeout, QMetaType::Int);
    CONFIG_VALUE(port, QMetaType::Int);
    CONFIG_VALUE(host, QMetaType::QByteArray);
    CONFIG_VALUE(user, QMetaType::QByteArray);
    CONFIG_VALUE(password, QMetaType::QByteArray);
    load("config.ini");
}
