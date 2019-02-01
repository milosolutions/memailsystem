#include "memailconfig.h"

EmailConfig::EmailConfig() : MConfig("email")
{
    CONFIG_VALUE(timeout, QMetaType::Int);
    CONFIG_VALUE(port, QMetaType::Int);
    CONFIG_VALUE(host, QMetaType::QString);
    CONFIG_VALUE(user, QMetaType::QString);
    CONFIG_VALUE(password, QMetaType::QString);
    load("config.ini");
}
