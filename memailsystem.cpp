#include "memailsystem.h"
#include <QSslSocket>
#include <QTextStream>
#include <QLoggingCategory>
#include <QDebug>
#include "memailconfig.h"

// enable/disable class internal debug messages (default: 0)
#define Sender_INTERNAL_DEBUG 0
#if Sender_INTERNAL_DEBUG
#include <QDebug>
#define internDbg(...) qDebug() << __VA_ARGS__
#else
#define internDbg(...)
#endif

/*! \class Sender
 *  \brief The sending e-mail system.
 *
 * Sends email via SMPT server.
 */

namespace Email
{

Q_LOGGING_CATEGORY(EmailLog, "system.email")

Sender::Sender(Email::EmailConfig *config, QObject *parent) : QObject(parent), m_config(config)
{
    m_socket = new QSslSocket(this);
    m_textStream = new QTextStream(m_socket);
    connect(m_socket, &QSslSocket::readyRead, this, &Sender::readFromSocket);
}

void Sender::send(const Message &email)
{
    Q_ASSERT(email.recipient.size());
    Q_ASSERT(email.body.size());
    m_emailQueue.enqueue(email);
    processNextRequest();
}

void Sender::readFromSocket()
{
    auto response = m_socket->readAll();
    internDbg(response);
    Q_ASSERT(response.size() >= 3);
    auto code = response.left(3);

    auto quit = [this]() {
        *m_textStream << "QUIT\r\n";
        m_state = Close;
    };

    switch (m_state) {
        case Init: {
            if (code == "220") {  // banner was okay, let's go on
                *m_textStream << "EHLO localhost"
                              << "\r\n";
                m_state = HandShake;
            }
            break;
        }
        case HandShake: {
            if (code == "250") {  // Send EHLO once again but now encrypted
                *m_textStream << "EHLO localhost"
                              << "\r\n";
                m_state = Auth;
            }
            break;
        }
        case Auth: {
            if (code == "250") {  // Trying AUTH
                *m_textStream << "AUTH LOGIN"
                              << "\r\n";
                m_state = User;
            }
            break;
        }
        case User: {
            if (code == "334") {  // Trying User
                // GMAIL is using XOAUTH2 protocol, which basically means that password and username
                // has to be sent in base64 coding
                // https://developers.google.com/gmail/xoauth2_protocol
                *m_textStream << toBase64(m_config->user) << "\r\n";
                m_state = Pass;
            }
            break;
        }
        case Pass: {
            if (code == "334") {  // Trying pass
                *m_textStream << toBase64(m_config->password) << "\r\n";
                m_state = Mail;
            }
            break;
        }
        case Mail: {
            if (code == "235") {
                // Apperantly for Google it is mandatory to have MAIL FROM and RCPT email formated
                // the following way -> <email@gmail.com>
                *m_textStream << "MAIL FROM:<" << m_config->user << ">\r\n";
                m_state = Rcpt;
            } else if (code == "535") {
                qCWarning(EmailLog) << QString("Authentication error");
                m_state = Close;
            }
            break;
        }
        case Rcpt: {
            if (code == "250") {
                // Apperantly for Google it is mandatory to have MAIL FROM and RCPT email formated
                // the following way -> <email@gmail.com>
                *m_textStream << "RCPT TO:<" << m_recipient << ">\r\n";
                m_state = Data;
            }
            break;
        }
        case Data: {
            if (code == "250") {
                *m_textStream << "DATA\r\n";
                m_state = Body;
            } else {  // ignore all errors
                qCWarning(EmailLog) << QString("Failed to add recipient: %1, error code: %2")
                                  .arg(QString(m_recipient))
                                  .arg(QString(code));
                quit();
            }
            break;
        }
        case Body: {
            if (code == "354") {
                *m_textStream << m_data << "\r\n.\r\n";
                m_state = Quit;
            }
            break;
        }
        case Quit: {
            if (code == "250") {
                qCDebug(EmailLog) << "Message sent";
                quit();
            }
            break;
        }
        case Close: {
            qDebug("closing socket");
            m_socket->disconnectFromHost();
            if (m_socket->waitForDisconnected(m_config->timeout)) {
                m_processing = false;
                processNextRequest();
            } else {
                qCCritical(EmailLog) << QString("Cannot disconnect from mail server: %s")
                          .arg(qPrintable(m_socket->errorString()));
            }
            return;
        }
        default:
            m_state = Close;
            emit qCritical("Failed to send message");
            break;
    }

    // flush after each state change
    m_textStream->flush();
}

void Sender::processNextRequest()
{
    if (m_emailQueue.isEmpty() || m_processing) return;

    m_processing = true;
    const auto &email = m_emailQueue.dequeue();

    m_recipient = email.recipient;
    m_data = "To: " + m_recipient + "\n";
    m_data.append("From: " + m_config->user + "\n");
    m_data.append("Subject: " + email.subject + "\n");
    m_data.append("MIME-Version: 1.0\n");
    m_data.append("Content-Type: text/html; charset=utf-8\n");
    m_data.append(email.body);
    m_data.append("\n\n");
    m_data.replace("\n", "\r\n");
    m_data.replace("\r\n.\r\n", "\r\n..\r\n");

    m_state = Init;
    m_socket->connectToHostEncrypted(m_config->host, m_config->port);
    if (!m_socket->waitForConnected(m_config->timeout)) {
        qCCritical(EmailLog) << QString("Cannot connect with mail server: %s").arg(qPrintable(m_socket->errorString()));
    }
    qCDebug(EmailLog) << "Connection established.";
}

QString toBase64(const QString& string)
{
    QByteArray ba;
    ba.append(string);
    return ba.toBase64();
}


}  // namespace Email
