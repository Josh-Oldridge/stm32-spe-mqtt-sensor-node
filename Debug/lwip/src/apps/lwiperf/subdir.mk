################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/apps/lwiperf/lwiperf.c 

OBJS += \
./lwip/src/apps/lwiperf/lwiperf.o 

C_DEPS += \
./lwip/src/apps/lwiperf/lwiperf.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/apps/lwiperf/%.o lwip/src/apps/lwiperf/%.su lwip/src/apps/lwiperf/%.cyclo: ../lwip/src/apps/lwiperf/%.c lwip/src/apps/lwiperf/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSPI_PROT_EN -DSPI_OA_EN -DADIN1110 -DUSE_HAL_DRIVER -DUSE_LWIP -DSTM32L496xx -c -I../Core/Inc -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/netif" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/include" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -Wno-format-extra-args -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-apps-2f-lwiperf

clean-lwip-2f-src-2f-apps-2f-lwiperf:
	-$(RM) ./lwip/src/apps/lwiperf/lwiperf.cyclo ./lwip/src/apps/lwiperf/lwiperf.d ./lwip/src/apps/lwiperf/lwiperf.o ./lwip/src/apps/lwiperf/lwiperf.su

.PHONY: clean-lwip-2f-src-2f-apps-2f-lwiperf

