/*******************************************************************************
Copyright (C) 2019 Milo Solutions
Contact: https://www.milosolutions.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/
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

struct EmailConfig
{
    int timeout;
    quint16 port;
    QString host;
    QString user;
    QString password;
    bool base64Encoding = true;
};

struct Message
{
    QString recipient;
    QString subject;
    QString body;
};

QString toBase64(const QString& string);

class Sender : public QObject
{
    Q_OBJECT

public:
    Sender(const EmailConfig& config, QObject *parent);
    void send(const Message &email);

private:
    void readFromSocket();
    void processNextRequest();
    QSslSocket *m_socket = nullptr;
    QTextStream *m_textStream = nullptr;
    int m_state;
    EmailConfig m_config;
    QString m_recipient;
    QString m_data;
    enum States { Init, HandShake, Auth, User, Pass, Rcpt, Mail, Data, Body,
                  Quit, Close };
    QQueue<Message> m_emailQueue;
    bool m_processing = false;
};

}  // namespace Email
#endif  // EMAILSYSTEM_H
