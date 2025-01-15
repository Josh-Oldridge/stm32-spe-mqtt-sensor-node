################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/ADIN1110/adi_mac.c \
../Core/Src/ADIN1110/adi_phy.c \
../Core/Src/ADIN1110/adi_spi_generic.c \
../Core/Src/ADIN1110/adi_spi_oa.c \
../Core/Src/ADIN1110/adin1110.c \
../Core/Src/ADIN1110/boardsupport.c \
../Core/Src/ADIN1110/dma.c \
../Core/Src/ADIN1110/fcs.c \
../Core/Src/ADIN1110/gpio.c \
../Core/Src/ADIN1110/hal.c \
../Core/Src/ADIN1110/spi.c \
../Core/Src/ADIN1110/sysclock.c \
../Core/Src/ADIN1110/usart.c 

OBJS += \
./Core/Src/ADIN1110/adi_mac.o \
./Core/Src/ADIN1110/adi_phy.o \
./Core/Src/ADIN1110/adi_spi_generic.o \
./Core/Src/ADIN1110/adi_spi_oa.o \
./Core/Src/ADIN1110/adin1110.o \
./Core/Src/ADIN1110/boardsupport.o \
./Core/Src/ADIN1110/dma.o \
./Core/Src/ADIN1110/fcs.o \
./Core/Src/ADIN1110/gpio.o \
./Core/Src/ADIN1110/hal.o \
./Core/Src/ADIN1110/spi.o \
./Core/Src/ADIN1110/sysclock.o \
./Core/Src/ADIN1110/usart.o 

C_DEPS += \
./Core/Src/ADIN1110/adi_mac.d \
./Core/Src/ADIN1110/adi_phy.d \
./Core/Src/ADIN1110/adi_spi_generic.d \
./Core/Src/ADIN1110/adi_spi_oa.d \
./Core/Src/ADIN1110/adin1110.d \
./Core/Src/ADIN1110/boardsupport.d \
./Core/Src/ADIN1110/dma.d \
./Core/Src/ADIN1110/fcs.d \
./Core/Src/ADIN1110/gpio.d \
./Core/Src/ADIN1110/hal.d \
./Core/Src/ADIN1110/spi.d \
./Core/Src/ADIN1110/sysclock.d \
./Core/Src/ADIN1110/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/ADIN1110/%.o Core/Src/ADIN1110/%.su Core/Src/ADIN1110/%.cyclo: ../Core/Src/ADIN1110/%.c Core/Src/ADIN1110/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSPI_OA_EN -DADIN1110 -DUSE_HAL_DRIVER -DSTM32L496xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-ADIN1110

clean-Core-2f-Src-2f-ADIN1110:
	-$(RM) ./Core/Src/ADIN1110/adi_mac.cyclo ./Core/Src/ADIN1110/adi_mac.d ./Core/Src/ADIN1110/adi_mac.o ./Core/Src/ADIN1110/adi_mac.su ./Core/Src/ADIN1110/adi_phy.cyclo ./Core/Src/ADIN1110/adi_phy.d ./Core/Src/ADIN1110/adi_phy.o ./Core/Src/ADIN1110/adi_phy.su ./Core/Src/ADIN1110/adi_spi_generic.cyclo ./Core/Src/ADIN1110/adi_spi_generic.d ./Core/Src/ADIN1110/adi_spi_generic.o ./Core/Src/ADIN1110/adi_spi_generic.su ./Core/Src/ADIN1110/adi_spi_oa.cyclo ./Core/Src/ADIN1110/adi_spi_oa.d ./Core/Src/ADIN1110/adi_spi_oa.o ./Core/Src/ADIN1110/adi_spi_oa.su ./Core/Src/ADIN1110/adin1110.cyclo ./Core/Src/ADIN1110/adin1110.d ./Core/Src/ADIN1110/adin1110.o ./Core/Src/ADIN1110/adin1110.su ./Core/Src/ADIN1110/boardsupport.cyclo ./Core/Src/ADIN1110/boardsupport.d ./Core/Src/ADIN1110/boardsupport.o ./Core/Src/ADIN1110/boardsupport.su ./Core/Src/ADIN1110/dma.cyclo ./Core/Src/ADIN1110/dma.d ./Core/Src/ADIN1110/dma.o ./Core/Src/ADIN1110/dma.su ./Core/Src/ADIN1110/fcs.cyclo ./Core/Src/ADIN1110/fcs.d ./Core/Src/ADIN1110/fcs.o ./Core/Src/ADIN1110/fcs.su ./Core/Src/ADIN1110/gpio.cyclo ./Core/Src/ADIN1110/gpio.d ./Core/Src/ADIN1110/gpio.o ./Core/Src/ADIN1110/gpio.su ./Core/Src/ADIN1110/hal.cyclo ./Core/Src/ADIN1110/hal.d ./Core/Src/ADIN1110/hal.o ./Core/Src/ADIN1110/hal.su ./Core/Src/ADIN1110/spi.cyclo ./Core/Src/ADIN1110/spi.d ./Core/Src/ADIN1110/spi.o ./Core/Src/ADIN1110/spi.su ./Core/Src/ADIN1110/sysclock.cyclo ./Core/Src/ADIN1110/sysclock.d ./Core/Src/ADIN1110/sysclock.o ./Core/Src/ADIN1110/sysclock.su ./Core/Src/ADIN1110/usart.cyclo ./Core/Src/ADIN1110/usart.d ./Core/Src/ADIN1110/usart.o ./Core/Src/ADIN1110/usart.su

.PHONY: clean-Core-2f-Src-2f-ADIN1110

