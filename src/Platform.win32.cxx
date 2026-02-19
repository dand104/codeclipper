#include "Platform.hxx"
#include <windows.h>

namespace codeclipper {
    void setupConsole() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
}