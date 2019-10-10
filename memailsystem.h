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
class EmailConfig;

namespace Email
{

Q_DECLARE_LOGGING_CATEGORY(EmailLog);

struct Message
{
    QByteArray recipient;
    QByteArray subject;
    QByteArray body;
};

class Sender : public QObject
{
    Q_OBJECT

public:
    enum ExitCode {Success, NotConfigured, ConnectionError, AuthLoginError,
                   UserFailed, PassFailed, RcptError, Unknown};
    Sender(EmailConfig* config, QObject *parent);
    void send(const Message &email);
    static QString exitCodeDescription(int code);
signals:
    void finished(int exitCode) const;
private:
    enum States { Init, HandShake, Auth, User, Pass, From, Rcpt, Mail, Data, Sent, Close };
    void readFromSocket();
    void processNextRequest();
    QSslSocket *m_socket{};
    QTextStream *m_textStream{};
    int m_state;
    ExitCode m_exitCode{Unknown};
    EmailConfig *m_config;
    QByteArray m_recipient;
    QByteArray m_data;
    QQueue<Message> m_emailQueue;
    bool m_processing = false;
};

}  // namespace Email
#endif  // EMAILSYSTEM_H
