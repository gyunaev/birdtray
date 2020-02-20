#include "TestResources.h"
#include <QtCore/QDir>


QString TestResources::getAbsoluteResourcePath(const QString &name) {
    return getResourceBasePath() + QDir::separator() + QDir::toNativeSeparators(name);
}

QString TestResources::getResourceBasePath() {
    QDir sourceFile(__FILE__);
    sourceFile.cdUp();
    sourceFile.cdUp();
    return QDir::toNativeSeparators(sourceFile.absoluteFilePath("res"));
}
