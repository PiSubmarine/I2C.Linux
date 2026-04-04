#include "PiSubmarine/I2C/Linux/Driver.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace
{
    void CreateWriteMessage(i2c_msg& message, uint8_t deviceAddress, uint8_t* data, std::size_t len)
    {
        message.addr = deviceAddress;
        message.flags = 0;
        message.len = len;
        message.buf = data;
    }

    void CreateReadMessage(i2c_msg& message, uint8_t deviceAddress, uint8_t* data, std::size_t len)
    {
        message.addr = deviceAddress;
        message.flags = I2C_M_RD; // write
        message.len = len;
        message.buf = data;
    }
}

namespace PiSubmarine::I2C::Linux
{
    Driver::Driver(const std::filesystem::path& device)
    {
        auto loggerName = "I2C::Linux::Driver(" + device.string() + ")";

        m_Logger = spdlog::get(loggerName);
        if (!m_Logger)
        {
            m_Logger = spdlog::stdout_color_mt(loggerName);
        }
        m_DeviceFd = open(device.c_str(), O_RDWR);
        if (m_DeviceFd < 0)
        {
            throw std::runtime_error("Failed to open I2C device: " + device.string());
        }
        m_Logger->info("I2C device opened: {}", m_DeviceFd);
    }

    Driver::~Driver()
    {
        if (m_DeviceFd >= 0)
        {
            close(m_DeviceFd);
            m_Logger->info("I2C device closed: {}", m_DeviceFd);
        }
        else
        {
            m_Logger->error("Driver destroyed, file descriptor invalid: {}", m_DeviceFd);
        }
    }

    bool Driver::Write(uint8_t deviceAddress, uint8_t* txData, std::size_t len)
    {
        i2c_msg messages[1];

        CreateWriteMessage(messages[0], deviceAddress, txData, len);

        i2c_rdwr_ioctl_data ioctl_data{};
        ioctl_data.msgs = messages;
        ioctl_data.nmsgs = 1;

        auto ret = ioctl(m_DeviceFd, I2C_RDWR, &ioctl_data);
        if (ret < 0)
        {
            m_Logger->error("Failed to write to I2C device: {} - {}", ret, std::strerror(errno));
            return false;
        }
        return true;
    }

    bool Driver::Read(uint8_t deviceAddress, uint8_t* rxData, std::size_t len)
    {
        i2c_msg messages[1];

        CreateReadMessage(messages[0], deviceAddress, rxData, len);

        i2c_rdwr_ioctl_data ioctl_data{};
        ioctl_data.msgs = messages;
        ioctl_data.nmsgs = 1;

        auto ret = ioctl(m_DeviceFd, I2C_RDWR, &ioctl_data);
        if (ret < 0)
        {
            m_Logger->error("Failed to read from I2C device: {} - {}", ret, std::strerror(errno));
            return false;
        }
        return true;
    }

    bool Driver::WriteRead(uint8_t deviceAddress, uint8_t* txData, std::size_t txLen, uint8_t* rxData,
                               std::size_t rxLen)
    {
        i2c_msg messages[2];

        CreateWriteMessage(messages[0], deviceAddress, txData, txLen);
        CreateReadMessage(messages[1], deviceAddress, rxData, rxLen);

        i2c_rdwr_ioctl_data ioctl_data{};
        ioctl_data.msgs = messages;
        ioctl_data.nmsgs = 2;

        auto ret = ioctl(m_DeviceFd, I2C_RDWR, &ioctl_data);
        if (ret < 0)
        {
            m_Logger->error("Failed to read from I2C device: {} - {}", ret, std::strerror(errno));
            return false;
        }
        return true;
    }
}
