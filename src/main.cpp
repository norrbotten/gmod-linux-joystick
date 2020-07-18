#include <iostream>

#include "joystick.hpp"

int main() {
    JS::JoystickDriver driver;
    driver.begin_poll();

    auto joy0 = driver.get_joy(0);

    while (true) {
        std::cout << joy0->get_axis(0) << "\n";

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }
}
