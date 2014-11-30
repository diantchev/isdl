
# Add inputs and outputs from these tool invocations to the build variables 
CoreUnitTests_SRC= \
CoreUnitTests.cpp

CoreUnitTests_INCLUDE = ./ /usr/include/libxml2

CoreUnitTests_LIBS=-lpthread -lstdc++ -lrt -lgtest -lgtest_main

CoreUnitTests_LIBPATH=gtest-1.6.0/libs/linux/$(PLATFORM)/$(CONFIGURATION)

_BIN_TARGETS=CoreUnitTests
