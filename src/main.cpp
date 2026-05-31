#include <iostream>

int main() {
    std::cout << "LightKV server starting..." << '\n';
    std::cout << "Stage 0 - project scaffold" << '\n';

#if defined(_WIN32)
    std::cout << "Platform: Windows scaffold build" << '\n';
#elif defined(__unix__) || defined(__APPLE__)
    std::cout << "Platform: Linux/Unix target build" << '\n';
#else
    std::cout << "Platform: Unknown target build" << '\n';
#endif

    return 0;
}

