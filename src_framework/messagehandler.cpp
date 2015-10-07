#include "messagehandler.h"
#include "settings.h"
#include <iostream>
#include <QTextStream>
#include <QTime>

#define COLOR_RESET "\033[0m"
#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"


void humanMessageWriterQt4(QtMsgType type, const char *msg)
{
    QTextStream err(stderr);

    switch (type) {
        case QtDebugMsg:
            err << QTime::currentTime().toString("hh:mm:ss.zzz")
                << BLUE   << "    Info: " << COLOR_RESET
                << msg << endl;
            break;
        case QtWarningMsg:
            err << QTime::currentTime().toString("hh:mm:ss.zzz")
                << YELLOW << " Warning: " << COLOR_RESET
                << msg << endl;
            break;
        case QtCriticalMsg:
            err << QTime::currentTime().toString("hh:mm:ss.zzz")
                << RED    << "   Error: " << COLOR_RESET
                << msg << endl;
            break;
        case QtFatalMsg:
            err << QTime::currentTime().toString("hh:mm:ss.zzz")
                << RED    << "   FATAL: " << COLOR_RESET
                << msg << endl;
            break;
        default:
          err << msg << endl;
    }
}

void humanMessageWriterQt5(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    QTextStream out(stdout);
    QTextStream err(stderr);

    QString function_src;
    if(CSettings::get("dbg_function").toBool()) {
        function_src = QString("%1:%2 ").
                arg(context.function).
                arg(context.line);
    }
    else {
        function_src = QString("");
    }

    switch (type) {
        case QtDebugMsg:
            out << COLOR_RESET
                << QTime::currentTime().toString("hh:mm:ss.zzz")
                << BLUE   << "    Info: " << GREEN
                << function_src << " " << COLOR_RESET;
            // Remove the '@' in messages that start with it.
            if(msg.at(0) == '@') {
                out << msg.mid(1) << endl;
            }
            else {
                out << msg << endl;
            }
            break;
        case QtWarningMsg:
            err << COLOR_RESET
                << QTime::currentTime().toString("hh:mm:ss.zzz")
                << YELLOW << " Warning: " << GREEN
                << function_src << " " << COLOR_RESET
                << msg << endl;
            break;
        case QtCriticalMsg:
            err << COLOR_RESET
                << QTime::currentTime().toString("hh:mm:ss.zzz")
                << RED    << "   Error: " << GREEN
                << function_src << " " << COLOR_RESET
                << msg << endl;
            break;
        case QtFatalMsg:
            err << COLOR_RESET
                << QTime::currentTime().toString("hh:mm:ss.zzz")
                << RED    << "   FATAL: " << GREEN
                << function_src << " " << COLOR_RESET
                << msg << endl;
            abort();
            break;
        default:
          err << msg << endl;
    }
}

void machineMessageWriterQt5(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    QTextStream out(stdout);
    QTextStream err(stderr);

    switch (type) {
        case QtDebugMsg:
          // Do not print debug messages unless the message starts with a '@'.
          if(msg.at(0) == '@') {
              // Chop the '@' sign before printing.
              out << msg.mid(1) << endl;
          }
          break;
      default:
        // Print everything else that is not a debug message.
        err << msg << endl;
    }
}
