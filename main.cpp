
#include <QApplication>
#include <QFile>
#include <QFileInfo>

#include "ErrP.h"
#include "EEGOpenVibe.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    EEGOpenVibe::getInstance();

    QFileInfo styleFileInfo(":/ErrP/Resources/DefaultStyle.qss");
    QFile styleFile(":/ErrP/Resources/DefaultStyle.qss");
    if (true == styleFileInfo.exists())
    {
      if (styleFile.open(QIODevice::ReadOnly))
      {
        qApp->setStyleSheet(styleFile.readAll());
        styleFile.close();
      }
    }
    ErrP ErrPInstance;
    ErrPInstance.show();
    return app.exec();
}
