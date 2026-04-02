#include "main.h"

/**
 * @brief The main entry point of the application.
 * Initializes the Qt application event loop and launches the primary MainWindow.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Exit code of the QApplication event loop.
 */
int main(int argc, char *argv[]) {
    // Instantiate the UI event loop manager
    QApplication app(argc, argv);
    
    // Set application information for platform-specific integration (e.g. settings storage)
    QCoreApplication::setApplicationName("Pixeon Image Viewer");
    QCoreApplication::setOrganizationName("Pixeon");
    
    // Create and display the main window interface
    MainWindow window;
    window.show();
    
    // Enter the Qt Main Event Loop
    return app.exec();
}
