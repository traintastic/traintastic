#include "kernel.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#endif

using namespace Marklin6050;

Kernel::Kernel(const std::string& port)
    : m_port(port), m_isOpen(false)
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

    m_isOpen = true;
    return true;

#else
    m_fd = open(m_port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_fd < 0)
        return false;

    termios options{};
    tcgetattr(m_fd, &options);
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
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

