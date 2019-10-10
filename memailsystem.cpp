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
#include "memailsystem.h"
#include "emailconfig.h"
#include <QSslSocket>
#include <QTextStream>
#include <QLoggingCategory>
#include <QRegularExpression>


/*! \class Sender
 *  \brief The sending e-mail system.
 *
 * Sends email using SMPT protocol (RFC 4954).
 */

namespace Email
{

Q_LOGGING_CATEGORY(EmailLog, "system.email")

// enable/disable class internal debug messages (default: 0)
#define Sender_INTERNAL_DEBUG 0
#if Sender_INTERNAL_DEBUG
// USE it on your own risk! It can expose your sensitive data like passwords.
#pragma message("WARNING! EMail protocol exposed! Set Sender_INTERNAL_DEBUG to 0 when you are done debugging!")
#define protocolDbg(direction, msg) qCDebug(EmailLog) << direction << qUtf8Printable(msg);
#else
#define protocolDbg(...)
#endif


Sender::Sender(EmailConfig* config, QObject *parent)
    : QObject(parent), m_config(config)
{
    m_socket = new QSslSocket(this);
    m_textStream = new QTextStream(m_socket);
    connect(m_socket, &QSslSocket::readyRead, this, &Sender::readFromSocket);
}

void Sender::send(const Message &email)
{
    if (m_config->host.isEmpty() || !m_config->port ||
        m_config->user.isEmpty() || m_config->password.isEmpty()) {
        emit finished(NotConfigured);
        return;
    }
    Q_ASSERT(email.recipient.size());
    Q_ASSERT(email.body.size());
    m_emailQueue.enqueue(email);
    processNextRequest();
}

void Sender::readFromSocket()
{
    auto response = m_socket->readAll();
    protocolDbg("RECEIVED: ", qUtf8Printable(response));
    Q_ASSERT(response.size() >= 3);
    auto code = response.left(3);

    QString protocolMessage;
    QTextStream stream(&protocolMessage);
    auto quit = [this, &stream]() {
        stream << "QUIT\r\n";
        m_state = Close;
    };

    switch (m_state) {
        case Init: {
            if (code == "220") {
                //Server sent prompt, say hello
                stream << "EHLO localhost" << "\r\n";
                m_state = HandShake;
            }
            break;
        }
        case HandShake: {
        if (code == "250") {
            // Handshake succesful, make sure login authentication is supported
            QRegularExpression re("250-AUTH\\s*(.+)");
            auto m = re.match(response);
            if (m.hasMatch() && m.captured(1).contains("LOGIN")) {
                // Autheticate with login mechanism
                stream << "AUTH LOGIN" << "\r\n";
                m_state = Auth;
            } else {
                m_exitCode = AuthLoginError;
                quit();
            }
        }
        break;
        }
        case Auth: {
            if (code == "334") {
                // AUTH started, now expects user name
                // all responses for AUTH must be encoded in BASE64 (rfc-4954)
                QByteArray user = m_config->base64Encoding ? m_config->user.toBase64() : m_config->user;
                stream << user << "\r\n";
                m_state = User;
            }
            break;
        }
        case User: {
        if (code == "334") {  // Trying pass
            // AUTH accepted user, provide password
            // all responses for AUTH must be encoded in BASE64 (rfc-4954)
            QByteArray password = m_config->base64Encoding ? m_config->password.toBase64() : m_config->password;
            stream << password << "\r\n";
            m_state = Pass;
        } else if (code == "535") {
            m_exitCode = UserFailed;
            quit();
        }
        break;

        }
        case Pass: {
        if (code == "235") {
            //AUTH successful, set sender address
            stream << "MAIL FROM:<" << m_config->user << ">\r\n";
            m_state = From;
        } else if (code == "535") {
            m_exitCode = PassFailed;
            quit();
        }
        break;
        }

    case From: {
        if (code == "250") {
            //OK, set recipient
            stream << "RCPT TO:<" << m_recipient << ">\r\n";
            m_state = Rcpt;
        }
        break;
    }

        case Rcpt: {
        if (code == "250") {
            //OK, start sending data
            stream << "DATA\r\n";
            m_state = Data;
        } else {  // ignore all errors
            m_exitCode = RcptError;
            quit();
        }
        break;
        }
        case Data: {
        if (code == "354") {
            //Server ready - send header, body and ending sequence
            stream << m_data << "\r\n.\r\n";
            m_state = Sent;
        }
        break;
        }
    case Sent: {
        if (code == "250") {
            m_exitCode = Success;
            quit();
        }
        break;
    }
    case Close: {
        m_socket->disconnectFromHost();
        m_socket->waitForDisconnected(m_config->timeout);
        emit finished(m_exitCode);
        m_processing = false;
        processNextRequest();
        return;
    }
        default:
        m_state = Close;
        m_exitCode = Unknown;
        break;
    }

    // flush after each state change
    protocolDbg("SENDING: ", protocolMessage);
    *m_textStream << protocolMessage;
    m_textStream->flush();
}

void Sender::processNextRequest()
{
    if (m_emailQueue.isEmpty() || m_processing) return;

    m_processing = true;
    const auto &email = m_emailQueue.dequeue();

    m_recipient = email.recipient;
    // Header
    m_data = "To: " + m_recipient + "\n";
    m_data.append("From: " + m_config->user + "\n");
    m_data.append("Subject: " + email.subject + "\n");
    m_data.append("MIME-Version: 1.0\n");
    m_data.append("Content-Type: text/html; charset=utf-8\n");
    // Body
    m_data.append(email.body);
    m_data.append("\n\n");
    m_data.replace("\n", "\r\n");
    m_data.replace("\r\n.\r\n", "\r\n..\r\n");

    m_state = Init;
    m_socket->connectToHostEncrypted(m_config->host, m_config->port);
    bool ok = m_socket->waitForConnected(m_config->timeout);
    if (ok) {
        m_state = Init;
    } else {
        emit finished(ConnectionError);
    }
}

QString Sender::exitCodeDescription(int code)
{
    static QMap<ExitCode,QString> description {
        {Success, tr("Message is sent.")},
        {NotConfigured, tr("Email account is not configured properly.")},
        {ConnectionError, tr("Could not connect to server within defined timeout.")},
        {AuthLoginError, tr("LOGIN authentication is not supported by server.")},
        {UserFailed, tr("Authentication failed. Are you using base64 encoding?")},
        {PassFailed, tr("Authentication failed. Maybe wrong password?")},
        {RcptError, tr("Failed to add email recipient.")},
        {Unknown, tr("Failed to send message due to unknown error.")}
    };
    return description.value(static_cast<ExitCode>(code));
}

}  // namespace Email
