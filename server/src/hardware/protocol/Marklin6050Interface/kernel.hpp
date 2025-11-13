#pragma once
#include <string>

namespace Marklin6050 {

class Kernel {
public:
    Kernel(const std::string& port, unsigned int baudrate = 2400);
    ~Kernel();

    bool start();
    void stop();

    bool sendByte(unsigned char byte);

    void setBaudRate(unsigned int baud) { m_baudrate = baud; }

private:
    std::string m_port;
    unsigned int m_baudrate;

#if defined(_WIN32)
    void* m_handle;
#else
    int m_fd;
#endif
    bool m_isOpen;
};

} // namespace Marklin6050

