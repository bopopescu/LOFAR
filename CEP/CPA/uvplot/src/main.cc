#include <qapplication.h>


#include <UVPMainWindow.h>

#include <Common/Debug.h>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  Debug::initLevels(argc, (const char **)argv);       // Initialize debugging

  UVPMainWindow *mainwin = new UVPMainWindow;
  mainwin->resize(800, 800);

  app.setMainWidget(mainwin);
  mainwin->show();
  return app.exec();
}
