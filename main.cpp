#include <QApplication>
#include "OilLabelGUI.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    OilLabelGUI window;
    window.show();
    return app.exec();
}

