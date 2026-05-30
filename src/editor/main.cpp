#include "editor/MainWindow.h"
#include <QApplication>
#include <stdexcept>
#include <QMessageBox>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("SabakaEngine Editor");
    app.setOrganizationName("SabakaEngine");

    try {
        engine::editor::MainWindow window;
        window.show();
        return app.exec();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "Editor Error", e.what());
        return -1;
    }
}