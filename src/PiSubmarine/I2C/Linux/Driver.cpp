#include "PiSubmarine/I2C/Linux/Driver.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <system_error>
#include <sys/ioctl.h>
#include <unistd.h>

#include "PiSubmarine/Error/Api/MakeError.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace
{
	using Result = PiSubmarine::Error::Api::Result<void>;
	using ErrorCondition = PiSubmarine::Error::Api::ErrorCondition;

	void CreateWriteMessage(i2c_msg& message, const uint8_t deviceAddress, std::span<const uint8_t> data)
	{
		message.addr = deviceAddress;
		message.flags = 0;
		message.len = static_cast<decltype(message.len)>(data.size());
		message.buf = const_cast<uint8_t*>(data.data());
	}

	void CreateReadMessage(i2c_msg& message, const uint8_t deviceAddress, std::span<uint8_t> data)
	{
		message.addr = deviceAddress;
		message.flags = I2C_M_RD;
		message.len = static_cast<decltype(message.len)>(data.size());
		message.buf = data.data();
	}

	[[nodiscard]] Result MakeContractError() noexcept
	{
		return std::unexpected(PiSubmarine::Error::Api::MakeError(
			ErrorCondition::ContractError,
			std::make_error_code(std::errc::invalid_argument)));
	}

	[[nodiscard]] Result MakeIoctlError() noexcept
	{
		return std::unexpected(PiSubmarine::Error::Api::MakeError(
			ErrorCondition::CommunicationError,
			std::error_code(errno, std::generic_category())));
	}

	template<typename T>
	[[nodiscard]] Result ValidateBuffer(const std::span<T> data) noexcept
	{
		if (data.size() > 0 && data.data() == nullptr)
		{
			return MakeContractError();
		}

		return {};
	}
}

namespace PiSubmarine::I2C::Linux
{
	Driver::Driver(const std::filesystem::path& device)
	{
		const auto loggerName = "I2C::Linux::Driver(" + device.string() + ")";

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

	Result Driver::Write(const uint8_t deviceAddress, const std::span<const uint8_t> txData)
	{
		if (const auto validationResult = ValidateBuffer(txData); !validationResult.has_value())
		{
			return validationResult;
		}

		i2c_msg messages[1];
		CreateWriteMessage(messages[0], deviceAddress, txData);

		i2c_rdwr_ioctl_data ioctlData{};
		ioctlData.msgs = messages;
		ioctlData.nmsgs = 1;

		const auto ret = ioctl(m_DeviceFd, I2C_RDWR, &ioctlData);
		if (ret < 0)
		{
			m_Logger->error("Failed to write to I2C device: {} - {}", ret, std::strerror(errno));
			return MakeIoctlError();
		}

		return {};
	}

	Result Driver::Read(const uint8_t deviceAddress, const std::span<uint8_t> rxData)
	{
		if (const auto validationResult = ValidateBuffer(rxData); !validationResult.has_value())
		{
			return validationResult;
		}

		i2c_msg messages[1];
		CreateReadMessage(messages[0], deviceAddress, rxData);

		i2c_rdwr_ioctl_data ioctlData{};
		ioctlData.msgs = messages;
		ioctlData.nmsgs = 1;

		const auto ret = ioctl(m_DeviceFd, I2C_RDWR, &ioctlData);
		if (ret < 0)
		{
			m_Logger->error("Failed to read from I2C device: {} - {}", ret, std::strerror(errno));
			return MakeIoctlError();
		}

		return {};
	}

	Result Driver::WriteRead(
		const uint8_t deviceAddress,
		const std::span<const uint8_t> txData,
		const std::span<uint8_t> rxData)
	{
		if (const auto txValidationResult = ValidateBuffer(txData); !txValidationResult.has_value())
		{
			return txValidationResult;
		}

		if (const auto rxValidationResult = ValidateBuffer(rxData); !rxValidationResult.has_value())
		{
			return rxValidationResult;
		}

		i2c_msg messages[2];
		CreateWriteMessage(messages[0], deviceAddress, txData);
		CreateReadMessage(messages[1], deviceAddress, rxData);

		i2c_rdwr_ioctl_data ioctlData{};
		ioctlData.msgs = messages;
		ioctlData.nmsgs = 2;

		const auto ret = ioctl(m_DeviceFd, I2C_RDWR, &ioctlData);
		if (ret < 0)
		{
			m_Logger->error("Failed to read from I2C device: {} - {}", ret, std::strerror(errno));
			return MakeIoctlError();
		}

		return {};
	}
}
