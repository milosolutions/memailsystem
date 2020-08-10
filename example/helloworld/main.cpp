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
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QDebug>

Q_LOGGING_CATEGORY(coreMain, "core.main")

//! Example use of Email::Sender class
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName("Example mail app");
    a.setOrganizationName("Milo");

    EmailConfig example;
    example.host = "smtp.gmail.com";
    example.port = 465;
    example.user = "your.email@gmail.com";
    example.password = "your.password";
    example.timeout = 10000;

    auto mailSender = new Email::Sender(&example, &a);
    Email::Message m;
    m.recipient = "any@recipient.pl";
    m.subject = QByteArray("HelloWorld! - subject");
    m.body = QByteArray("HelloWorld! - content");
    mailSender->send(m);

    return a.exec();
}
