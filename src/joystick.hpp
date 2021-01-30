#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>

namespace JS {

    class JoystickState {
    public:
        explicit JoystickState(const std::string& path)
            : m_device(path, std::ios::binary | std::ios::in)
            , m_path(path) {
        }

        bool poll() {
            js_event evnt;
            m_device.read((char*)(&evnt), sizeof(evnt));

            if (!m_device.fail()) {
                handle_event(evnt);
                return true;
            }

            return false;
        }

        auto get_button(__u8 btn) const {
            return m_btn_state[btn];
        }

        auto get_axis(__u8 axis) const {
            return m_axis_state[axis];
        }

        const auto& last_event() const {
            return m_last_event;
        }

        std::string get_name() const {
            auto fd = open(m_path.c_str(), O_RDONLY);
            if (fd < 0)
                return "Generic Joystick";

            char buf[256] = {0};
            if (ioctl(fd, JSIOCGNAME(sizeof(buf)), buf) != -1) {
                close(fd);
                return std::string(buf);
            }

            return "Generic Joystick";
        }

        __u8 num_axes() const {
            auto fd = open(m_path.c_str(), O_RDONLY);
            if (fd < 0)
                return 0;

            __u8 num;
            if (ioctl(fd, JSIOCGAXES, &num) != -1) {
                close(fd);
                return num;
            }

            return 0;
        }

        __u8 num_buttons() const {
            auto fd = open(m_path.c_str(), O_RDONLY);
            if (fd < 0)
                return 0;

            __u8 num;
            if (ioctl(fd, JSIOCGBUTTONS, &num) != -1) {
                close(fd);
                return num;
            }

            return 0;
        }

    private:
        void handle_event(const js_event& evnt) {
            m_last_event = evnt;

            switch (evnt.type) {
            case JS_EVENT_BUTTON: m_btn_state[evnt.number] = evnt.value; break;
            case JS_EVENT_AXIS: m_axis_state[evnt.number] = evnt.value; break;
            default: break;
            }
        }

    private:
        std::fstream m_device;
        std::string  m_path;

        __s16 m_btn_state[256]  = {0};
        __s16 m_axis_state[256] = {0};

        js_event m_last_event;
    };

    class JoystickDriver {
    public:
        explicit JoystickDriver() {
            namespace fs = std::filesystem;

            for (const auto& entry : fs::directory_iterator("/dev/input"))
                if (entry.path().filename().generic_string().starts_with("js"))
                    m_joys.push_back(JoystickState(entry.path()));
        }

        ~JoystickDriver() {
            if (m_should_poll)
                end_poll();
        }

        void begin_poll() {
            m_should_poll = true;

            m_poll_thread = std::thread([this] {
                while (m_should_poll) {
                    std::lock_guard g(m_joys_mtx);

                    for (auto& joy : m_joys)
                        joy.poll();

                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(1ms);
                }
            });
        }

        void end_poll() {
            m_should_poll = false;
            m_poll_thread.join();
        }

        auto num_joys() const {
            return m_joys.size();
        }

        const JoystickState* get_joy(std::size_t i) const {
            if (i >= m_joys.size())
                return nullptr;

            return &m_joys.at(i);
        }

    private:
        std::vector<JoystickState> m_joys;
        std::mutex                 m_joys_mtx;

        bool        m_should_poll;
        std::thread m_poll_thread;
    };

} // namespace JS
