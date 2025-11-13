#include "kernel.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#endif

using namespace Marklin6050;

Kernel::Kernel(const std::string& port, unsigned int baudrate)
    : m_port(port), m_baudrate(baudrate), m_isOpen(false)
{
#if defined(_WIN32)
    m_handle = INVALID_HANDLE_VALUE;
#else
    m_fd = -1;
#endif
}

Kernel::~Kernel()
{
    stop();
}

bool Kernel::start()
{
#if defined(_WIN32)
    std::string devicePath = "\\\\.\\" + m_port;
    m_handle = CreateFileA(
        devicePath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (m_handle == INVALID_HANDLE_VALUE)
        return false;

    // Configure baud rate on Windows
    DCB dcb{};
    if (!GetCommState(m_handle, &dcb))
        return false;

    dcb.BaudRate = m_baudrate;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    if (!SetCommState(m_handle, &dcb))
        return false;

    m_isOpen = true;
    return true;

#else
    m_fd = open(m_port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_fd < 0)
        return false;

    termios options{};
    tcgetattr(m_fd, &options);

    // Map common baud rates
    speed_t speed;
    switch (m_baudrate) {
        case 1200:   speed = B1200; break;
        case 2400:   speed = B2400; break;
        case 4800:   speed = B4800; break;
        case 9600:   speed = B9600; break;
        case 19200:  speed = B19200; break;
        case 38400:  speed = B38400; break;
        case 57600:  speed = B57600; break;
        case 115200: speed = B115200; break;
        default:     speed = B2400; break; // fallback
    }

    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    tcsetattr(m_fd, TCSANOW, &options);

    m_isOpen = true;
    return true;
#endif
}

void Kernel::stop()
{
    if (!m_isOpen)
        return;

#if defined(_WIN32)
    if (m_handle != INVALID_HANDLE_VALUE)
        CloseHandle(m_handle);
    m_handle = INVALID_HANDLE_VALUE;
#else
    if (m_fd >= 0)
        close(m_fd);
    m_fd = -1;
#endif

    m_isOpen = false;
}

bool Kernel::sendByte(unsigned char byte)
{
    if (!m_isOpen)
        return false;

#if defined(_WIN32)
    DWORD written = 0;
    WriteFile(m_handle, &byte, 1, &written, nullptr);
    return written == 1;
#else
    return write(m_fd, &byte, 1) == 1;
#endif
}

void Kernel::setBaudRate(unsigned int baud)
{
    m_baudrate = baud;
}
