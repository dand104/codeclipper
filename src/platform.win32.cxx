module;
#include <windows.h>

module codeclipper;
import :platform;

namespace codeclipper::platform {
    void setupConsole() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
}