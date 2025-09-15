#include <QApplication>
#include <QFile>
#include <QLoggingCategory>

#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

#include "MainWindow.hpp"

int main(int argc, char *argv[]) {
	QLoggingCategory::setFilterRules("*.debug=false\n*.warning=false");

	QApplication app(argc, argv);
	MainWindow window;
	window.setWindowTitle("Audio Redirector");
	window.setWindowIcon(QIcon(":/icons/app.ico"));
	window.resize(435, 398);

	QFile styleFile(":/styles/style.qss");
	if (styleFile.open(QFile::ReadOnly)) {
		QString styleSheet = QLatin1String(styleFile.readAll());
		window.setStyleSheet(styleSheet);
	}

	const HWND hwnd = reinterpret_cast<HWND>(window.winId());
	constexpr BOOL enable = TRUE; // DWMWA_USE_IMMERSIVE_DARK_MODE (official as of Win 10 1809+)
	::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &enable, sizeof(enable));

	window.show();
	return app.exec();
}
