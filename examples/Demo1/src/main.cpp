/**
  @file
  @author Stefan Frings
*/

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include "httplistener.h"
#include "templatecache.h"
#include "httpsessionstore.h"
#include "staticfilecontroller.h"
#include "filelogger.h"
#include "requestmapper.h"

/** Name of this application */
#define APPNAME "Demo1"

/** Publisher of this application */
#define ORGANISATION "Butterfly"


/** The HTTP listener of the application */
HttpListener* listener;

/** Cache for template files */
TemplateCache* templateCache;

/** Storage for session cookies */
HttpSessionStore* sessionStore;

/** Controller for static files */
StaticFileController* staticFileController;

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

/**
  Entry point of the program.
*/
int main(int argc, char *argv[]) {

    // Initialize the core application
    QCoreApplication* app=new QCoreApplication(argc, argv);
    app->setApplicationName(APPNAME);
    app->setOrganizationName(ORGANISATION);

    // Find the configuration file
    QString configFileName=searchConfigFile();

    // Configure logging
    QSettings* logSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    logSettings->beginGroup("logging");
    logger=new FileLogger(logSettings,10000,app);
    logger->installMsgHandler();

    // Log the library version
    qDebug("QtWebAppLib has version %s",getQtWebAppLibVersion());

    // Configure template loader and cache
    QSettings* templateSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    templateSettings->beginGroup("templates");
    templateCache=new TemplateCache(templateSettings,app);

    // Configure session store
    QSettings* sessionSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    sessionSettings->beginGroup("sessions");
    sessionStore=new HttpSessionStore(sessionSettings,app);

    // Configure static file controller
    QSettings* fileSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    fileSettings->beginGroup("docroot");
    staticFileController=new StaticFileController(fileSettings,app);

    // Configure and start the TCP listener
    qDebug("ServiceHelper: Starting service");
    QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,app);
    listenerSettings->beginGroup("listener");
    listener=new HttpListener(listenerSettings,new RequestMapper(app),app);

    if (logSettings->value("bufferSize",0).toInt()>0 && logSettings->value("minLevel",0).toInt()>0) {
        qDebug("You see these debug messages because the logging buffer is enabled");
    }
    qWarning("Application has started");

    return app->exec();
}
