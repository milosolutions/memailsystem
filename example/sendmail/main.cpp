#include "emailconfig.h"
#include "emailsystem.h"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTimer>

#define STR_VALUE(arg) #arg
#define TO_STRING(name) STR_VALUE(name)

// To make application safer provide CONFIG_PASS as parameter to you build
// e.g. qmake "DEFINES+=CONFIG_PASS=your-pass" sendemail.pro
#ifdef CONFIG_PASS
#define PASS TO_STRING(CONFIG_PASS)
#else
#define PASS "y3sN%2hd6"
#endif

#ifdef Q_OS_UNIX
// on unix we can use default QSettings location
#define CONFIG_FILE
#else
// windows registry does not take well our encrypted values
// we need to store setting in file
#define CONFIG_FILE "sendmail.conf"
#endif

void configure(EmailConfig& config);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Milo Solutions");
    QCoreApplication::setApplicationName("SendMail");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Tool for sending emails with accout defined in settings");
    parser.addPositionalArgument("recipient", "Email address of recipient.");
    parser.addPositionalArgument("subject", "Subject of mail.");
    parser.addPositionalArgument("body", "Path to html file with mail content.");
    parser.addOption({{"c", "configure"}, "Configure account for sending emails and quit."});
    parser.addHelpOption();
    parser.process(a);

    EmailConfig config;
    config.setPassphrase(PASS);
    config.load(CONFIG_FILE);

    if (parser.isSet("configure")) {
        configure(config);
        return 0;
    }

    Email::Sender sender(&config, &a);
    a.connect(&sender, &Email::Sender::finished, [&a](int code) {
        if (code) qCritical() << Email::Sender::exitCodeDescription(code);
        a.exit(code);
    });

    if (parser.positionalArguments().size() != 3) {
        parser.showHelp();
    }
    Email::Message message;
    message.recipient = parser.positionalArguments().at(0).toUtf8();
    message.subject = parser.positionalArguments().at(1).toUtf8();
    qInfo() << message.recipient << message.subject;
    QFile html(parser.positionalArguments().at(2));
    if (html.open(QIODevice::ReadOnly)) {
        message.body = html.readAll();
        html.close();
    } else {
        qCritical("Cannot load provided file: %s", qPrintable(html.fileName()));
        return 1;
    }
    QTimer::singleShot(0, [&sender, &message](){
        sender.send(message);
    });

    return a.exec();
}

void configure(EmailConfig& config)
{
    QTextStream in(stdin);
    QTextStream out(stdout);
    auto readString = [&in, &out](const QString& prompt, const QString& defaultValue, bool hide = false) -> QString {
        out << prompt << " (" << (hide ? QString("*").repeated(defaultValue.size()) : defaultValue) <<  "): ";
        out.flush();
        QString str = in.readLine();
        if (str.isEmpty()) return defaultValue;
        return str;
    };

    out << "Provide data for email account that will be used to send all emails.\n";
    out << "If you press <Enter> value from brackets will be saved.\n";
    out.flush();
    config.host = readString("Email service host", config.host).toUtf8();
    config.port = readString("Email service port", QString::number(config.port)).toInt();
    config.user = readString("Email account username", config.user).toUtf8();
    config.password = readString("Email account password", config.password, true).toUtf8();
    config.timeout = readString("Connection timeout [ms]", QString::number(config.timeout)).toInt();
    config.base64Encoding = readString("Use Base64 encoding? [yes/no]", config.base64Encoding ? "yes" : "no") == "yes";
    config.save(CONFIG_FILE);
    out << "All settings has been saved. You can run tool again.\n";
    out.flush();
}
