#include <gtest/gtest.h>
#include "PiSubmarine/I2C/Linux/Driver.h"

namespace PiSubmarine::I2C::Linux
{
    TEST(DriverTest, Contruct)
    {
        Driver driver("/dev/i2c-1");
    }

    TEST(DriverTest, ContructNoSuchDevice)
    {
        EXPECT_ANY_THROW(Driver driver("/dev/i2c-xyz"));
    }
}