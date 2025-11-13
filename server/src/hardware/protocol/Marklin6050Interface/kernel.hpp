#pragma once
#include <string>

namespace Marklin6050 {

class Kernel {
public:
    Kernel(const std::string& port);
    ~Kernel();

    bool start();
    void stop();

    bool sendByte(unsigned char byte);

private:
    std::string m_port;

#if defined(_WIN32)
    void* m_handle;
#else
    int m_fd;
#endif
    bool m_isOpen;
};

} // namespace Marklin6050

