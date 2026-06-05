#include <QCoreApplication>
#include <QDebug>
#include "launcher/translations/POTranslator.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    POTranslator trans("launcher/translations/uk.po");
    if (trans.isEmpty()) {
        qDebug() << "Failed to load!";
    } else {
        qDebug() << "Loaded!";
        qDebug() << "Search:" << trans.translate("MainWindow", "Search instances...", nullptr, -1);
        qDebug() << "Welcome:" << trans.translate("LanguageWizardPage", "Welcome", nullptr, -1);
    }
    return 0;
}
