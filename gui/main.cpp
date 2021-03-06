/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 *
 * Copyright: 2012-2013 Boomaga team https://github.com/Boomaga
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include "kernel/psproject.h"
#include "mainwindow.h"
#include "dbus.h"

#include <QApplication>
#include <QTextStream>
#include <QLocale>
#include <QTranslator>
#include <QDBusConnection>
#include <QFileInfo>
#include <QLibraryInfo>
#include <QDebug>
#include <QProcessEnvironment>

/************************************************

 ************************************************/
void printHelp()
{
    QTextStream out(stdout);
    out << "Usage: boomaga [options] [file]" << endl;
    out << endl;

    out << "Boomaga provides a virtual printer for CUPS. This can be used" << endl;
    out << "for print preview or for print booklets." << endl;
    out << endl;

    out << "Options:" << endl;
    //out << "  --debug               Keep output results from scripts" << endl;
    out << "  -h, --help              Show help about options" << endl;
    out << "  -t, --title <title>     The job name/title" << endl;
    out << "  -n, --num <copies>      Sets the number of copies to print" << endl;
    out << "  -V, --version           Print program version" << endl;
    out << endl;

    out << "Arguments:" << endl;
    out << "  file                    Postscript file" << endl;


}


/************************************************

 ************************************************/
void printVersion()
{
    QTextStream out(stdout);
    out << "boomaga " << FULL_VERSION << endl;
}


/************************************************

 ************************************************/
int printError(const QString &msg)
{
    QTextStream out(stdout);
    out << msg << endl << endl;
    out << "Use --help to get a list of available command line options." << endl;
    return 1;
}


/************************************************

 ************************************************/
int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    application.installTranslator(&qtTranslator);


    QTranslator translator;
    translator.load(QString("%1/boomaga_%2.qm").arg(TRANSLATIONS_DIR, QLocale::system().name()));
    application.installTranslator(&translator);


    QFileInfo file;
    QString jobTitle;
    int copiesCount=0;

    QStringList args = application.arguments();
    for (int i=1; i < args.count(); ++i)
    {
        QString arg = args.at(i);

        //*************************************************
        if (arg == "-h" || arg == "--help")
        {
            printHelp();
            return 0;
        }

        //*************************************************
        if (arg == "-V" || arg == "--version")
        {
            printVersion();
            return 0;
        }

        //*************************************************
        if (arg == "-t" || arg == "--title")
        {
            if (i+1 < args.count())
            {
                jobTitle = args.at(i+1);
                i++;
                continue;
            }
            else
            {
                return printError("'title' is missing.");
            }
        }

        //*************************************************
        if (arg == "-n" || arg == "--num")
        {
            if (i+1 < args.count())
            {
                bool ok;
                copiesCount = args.at(i+1).toInt(&ok);
                if (ok)
                {
                    i++;
                    continue;
                }
                else
                {
                    return printError("'number of copies' is invalid.");
                }
            }
            else
            {
                return printError(QString("expected copies after \"%1\" option.").arg(arg));
            }
        }

        //*************************************************
        file.setFile(args.at(i));
    }


    if (!file.filePath().isEmpty())
    {
        if (!file.exists())
            return printError(QString("Cannot open file \"%1\" (No such file or directory)")
                              .arg(file.filePath()));

        if (!file.isReadable())
            return printError(QString("Cannot open file \"%1\" (Access denied)")
                              .arg(file.filePath()));
    }

#if 0
    QFile f(QString("/tmp/boomaga-%1.env").arg(QDateTime::currentDateTime().toString("yyyy.MM.dd-hh.mm.ss")));
    f.open(QFile::WriteOnly | QFile::Text);
    foreach(QString s, QProcessEnvironment::systemEnvironment().toStringList())
    {
        f.write(s.toLocal8Bit());
        f.write("\n");
    }
    f.close();
#endif

    PsProject project;
    DBusProjectAdaptor dbus(&project);
    QDBusConnection::sessionBus().registerService("org.boomaga");
    QDBusConnection::sessionBus().registerObject("/Project", &project);

    if (!file.filePath().isEmpty())
        project.addFile(file.absoluteFilePath());

    MainWindow mainWindow(&project);
    mainWindow.show();
    return application.exec();
}
