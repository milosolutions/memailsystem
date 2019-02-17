#ifndef EMAILSYSTEM_H
#define EMAILSYSTEM_H

#include <QObject>
#include <QQueue>
#include <QString>
#include <QLoggingCategory>

class QSslSocket;
class QTextStream;

namespace Email
{
Q_DECLARE_LOGGING_CATEGORY(EmailLog);
class EmailConfig;
struct Message {
    QString recipient;
    QString subject;
    QString body;
};

QString toBase64(const QString& string);

class Sender : public QObject
{
    Q_OBJECT
 public:
    Sender(EmailConfig *config, QObject *parent);
    void send(const Message &email);

 private:
    void readFromSocket();
    void processNextRequest();
    QSslSocket *m_socket;
    QTextStream *m_textStream;
    int m_state;
    EmailConfig *m_config;
    QString m_recipient;
    QString m_data;
    enum States { Init, HandShake, Auth, User, Pass, Rcpt, Mail, Data, Body, Quit, Close };
    QQueue<Message> m_emailQueue;
    bool m_processing = false;
};

}  // namespace Email
#endif  // EMAILSYSTEM_H
