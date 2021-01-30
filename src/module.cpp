#define LUNAR_CLIENT_MODULE
#include <sol/sol.hpp>

#define DLLEXPORT extern "C" __attribute((visibility("default")))

#include "joystick.hpp"

static JS::JoystickDriver* g_joy_driver;

DLLEXPORT int gmod13_open(lua_State* L) {
    Lunar::Loader::Initialize();

    g_joy_driver = new JS::JoystickDriver();
    g_joy_driver->begin_poll();

    sol::state_view lua(L);
    sol::table      joystick = lua.create_table();

    // original library has this refresh function to update axes,
    // but since mine polls continously this is just a no-op
    joystick["refresh"] = [] {};

    joystick["axis"] = [](int i, __u8 axis) -> int {
        auto joy = g_joy_driver->get_joy(i);
        if (joy == nullptr)
            return 0;

        return joy->get_axis(axis);
    };

    // mine doesnt support povs, these are axes anyways
    joystick["pov"] = [](int, __u8) -> __s16 { return 0; };

    joystick["button"] = [](int i, __u8 axis) -> int {
        auto joy = g_joy_driver->get_joy(i);
        if (joy == nullptr)
            return 0;

        return joy->get_button(axis);
    };

    joystick["name"] = [](int i) -> std::string {
        auto joy = g_joy_driver->get_joy(i);
        if (joy == nullptr)
            return "";

        return joy->get_name();
    };

    joystick["count"] = [](sol::variadic_args va) -> std::size_t {
        if (va.size() == 0) {
            return g_joy_driver->num_joys();
        }
        else if (va.size() == 2) {
            int i    = va.get<int>(0);
            int type = va.get<int>(1);

            auto joy = g_joy_driver->get_joy(i);
            if (joy == nullptr)
                return 0;

            switch (type) {
            case 1: return joy->num_axes();
            case 2: return 0; // again, no povs
            case 3: return joy->num_buttons();
            default: return 0;
            }
        }

        return 0;
    };

    lua["joystick"] = joystick;

    return 0;
}

DLLEXPORT int gmod13_close(lua_State*) {
    Lunar::Loader::Deinitialize();
    g_joy_driver->~JoystickDriver();
    return 0;
}
