#include "Application.h"

// #define BREAK_INFINITE_LOOP
// #define BREAK_EXCEPTION
// #define BREAK_RETURN

#if defined(_MSC_VER)
#pragma comment(                                                               \
    linker,                                                                    \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifdef BREAK_INFINITE_LOOP
#include <chrono>
#include <thread>

#endif

int main(int argc, char *argv[]) {
#ifdef BREAK_INFINITE_LOOP
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
#endif
#ifdef BREAK_EXCEPTION
  throw 42;
#endif
#ifdef BREAK_RETURN
  return 42;
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

  // initialize Qt
  Application app(argc, argv);

  switch (app.status()) {
  case Application::StartingUp:
  case Application::Initialized: {
    Q_INIT_RESOURCE(backgrounds);
    Q_INIT_RESOURCE(documents);
    Q_INIT_RESOURCE(logo);

    Q_INIT_RESOURCE(flat);
    Q_INIT_RESOURCE(colored);
    Q_INIT_RESOURCE(white);
    Q_INIT_RESOURCE(builtin);

    Q_INIT_RESOURCE(skins);
    return app.exec();
  }
  case Application::Failed:
    return 1;
  case Application::Succeeded:
    return 0;
  }
}
