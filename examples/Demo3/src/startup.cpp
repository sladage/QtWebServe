/**
  @file
  @author Stefan Frings
*/

#include "static.h"
#include "startup.h"
#include "filelogger.h"
#include "httplistener.h"
#include "requesthandler.h"
#include <QDir>
#include <QFile>

/** Name of this application */
#define APPNAME "Demo3"

/** Publisher of this application */
#define ORGANISATION "Butterfly"

/** Short description of the Windows service */
#define DESCRIPTION "Demo application for QtWebApp"

/** The HTTP listener of the application */
HttpListener* listener;

/** Logger class */
FileLogger* logger;

/** Search the configuration file */
QString searchConfigFile() {
    QString binDir=QCoreApplication::applicationDirPath();
    QString appName=QCoreApplication::applicationName();
    QString fileName(appName+".ini");

    QStringList searchList;
    searchList.append(binDir);
    searchList.append(binDir+"/etc");
    searchList.append(binDir+"/../etc");
    searchList.append(binDir+"/../../etc"); // for development without shadow build
    searchList.append(binDir+"/../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(binDir+"/../../../../../"+appName+"/etc"); // for development with shadow build
    searchList.append(QDir::rootPath()+"etc/opt");
    searchList.append(QDir::rootPath()+"etc");

    foreach (QString dir, searchList) {
        QFile file(dir+"/"+fileName);
        if (file.exists()) {
            // found
            fileName=QDir(file.fileName()).canonicalPath();
            qDebug("Using config file %s",qPrintable(fileName));
            return fileName;
        }
    }

    // not found
    foreach (QString dir, searchList) {
        qWarning("%s/%s not found",qPrintable(dir),qPrintable(fileName));
    }
    qFatal("Cannot find config file %s",qPrintable(fileName));
    return 0;
}

void Startup::start() {
    // Initialize the core application
    QCoreApplication* app = application();
    app->setApplicationName(APPNAME);
    app->setOrganizationName(ORGANISATION);

    // Find the configuration file
    QString configFileName=searchConfigFile();

    // Configure logging.
    // This part is optional but highly recommended to be able to see error messages.
    QSettings* logSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    logSettings->beginGroup("logging");
    logger=new FileLogger(logSettings,10000,app);
    logger->installMsgHandler();

    // Log the library version
    qDebug("QtWebAppLib has version %s",getQtWebAppLibVersion());

    // Configure and start the TCP listener
    qDebug("ServiceHelper: Starting service");
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    listenerSettings->beginGroup("listener");
    listener=new HttpListener(listenerSettings,new RequestHandler(app),app);

    if (logSettings->value("bufferSize",0).toInt()>0 && logSettings->value("minLevel",0).toInt()>0) {
        qDebug("You see these debug messages because the logging buffer is enabled");
    }
    qWarning("ServiceHelper: Service has started");
}

void Startup::stop() {
    // Note that the stop method is not called when you terminate the application abnormally
    // by pressing Ctrl-C or when you kill it by the task manager of your operating system.

    // Deleting the listener here is optionally because QCoreApplication does it already.
    // However, QCoreApplication closes the logger at first, so we would not see the shutdown
    // debug messages, without the following line of code:
    delete listener;

    qWarning("ServiceHelper: Service has been stopped");
}


Startup::Startup(int argc, char *argv[])
    : QtService<QCoreApplication>(argc, argv, APPNAME)
{
    setServiceDescription(DESCRIPTION);
    setStartupType(QtServiceController::AutoStartup);
}



