#pragma once

#include "PiSubmarine/I2C/Api/IDriver.h"
#include <filesystem>

namespace PiSubmarine::I2C::Linux
{
    class Driver : public Api::IDriver
    {
    public:
        explicit Driver(const std::filesystem::path& device);
        ~Driver() override;
        bool Write(uint8_t deviceAddress, uint8_t* txData, std::size_t len) override;
        bool Read(uint8_t deviceAddress, uint8_t* rxData, std::size_t len) override;
        bool WriteRead(uint8_t deviceAddress, uint8_t* txData, std::size_t txLen, uint8_t* rxData,
            std::size_t rxLen) override;

    private:
        int m_DeviceFd = 0;

    };
}
