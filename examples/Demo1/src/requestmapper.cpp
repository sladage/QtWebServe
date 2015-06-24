/**
  @file
  @author Stefan Frings
*/

#include "requestmapper.h"
#include "staticfilecontroller.h"
#include "filelogger.h"
#include "controller/dumpcontroller.h"
#include "controller/templatecontroller.h"
#include "controller/formcontroller.h"
#include "controller/fileuploadcontroller.h"
#include "controller/sessioncontroller.h"

/** Controller for static files */
extern StaticFileController* staticFileController;

/** Logger class */
extern FileLogger* logger;

RequestMapper::RequestMapper(QObject* parent)
    :HttpRequestHandler(parent) {}

void RequestMapper::service(HttpRequest& request, HttpResponse& response) {
    QByteArray path=request.getPath();
    qDebug("RequestMapper: path=%s",path.data());

    if (path.startsWith("/dump")) {
        DumpController().service(request, response);
    }

    else if (path.startsWith("/template")) {
        TemplateController().service(request, response);
    }

    else if (path.startsWith("/form")) {
        FormController().service(request, response);
    }

    else if (path.startsWith("/file")) {
        FileUploadController().service(request, response);
    }

    else if (path.startsWith("/session")) {
        SessionController().service(request, response);
    }

    // All other pathes are mapped to the static file controller.
    else {
        staticFileController->service(request, response);
    }
    qDebug("RequestMapper: finished request");
    logger->clear();
}
