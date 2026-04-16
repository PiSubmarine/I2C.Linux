#pragma once

#include <filesystem>
#include <span>
#include <spdlog/spdlog.h>

#include "PiSubmarine/I2C/Api/IDriver.h"

namespace PiSubmarine::I2C::Linux
{
    class Driver : public Api::IDriver
    {
    public:
        explicit Driver(const std::filesystem::path& device);
        ~Driver() override;
        [[nodiscard]] PiSubmarine::Error::Api::Result<void> Write(
            uint8_t deviceAddress,
            std::span<const uint8_t> txData) override;
        [[nodiscard]] PiSubmarine::Error::Api::Result<void> Read(
            uint8_t deviceAddress,
            std::span<uint8_t> rxData) override;
        [[nodiscard]] PiSubmarine::Error::Api::Result<void> WriteRead(
            uint8_t deviceAddress,
            std::span<const uint8_t> txData,
            std::span<uint8_t> rxData) override;

    private:
        int m_DeviceFd = 0;
        std::shared_ptr<spdlog::logger> m_Logger;

    };
}
