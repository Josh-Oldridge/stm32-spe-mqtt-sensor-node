################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/apps/http/makefsdata/makefsdata.c 

OBJS += \
./lwip/src/apps/http/makefsdata/makefsdata.o 

C_DEPS += \
./lwip/src/apps/http/makefsdata/makefsdata.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/apps/http/makefsdata/%.o lwip/src/apps/http/makefsdata/%.su lwip/src/apps/http/makefsdata/%.cyclo: ../lwip/src/apps/http/makefsdata/%.c lwip/src/apps/http/makefsdata/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_LWIP -DSPI_PROT_EN -DSPI_OA_EN -DADIN1110 -DUSE_HAL_DRIVER -DSTM32L496xx -c -I../Core/Inc -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/netif" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/include" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-apps-2f-http-2f-makefsdata

clean-lwip-2f-src-2f-apps-2f-http-2f-makefsdata:
	-$(RM) ./lwip/src/apps/http/makefsdata/makefsdata.cyclo ./lwip/src/apps/http/makefsdata/makefsdata.d ./lwip/src/apps/http/makefsdata/makefsdata.o ./lwip/src/apps/http/makefsdata/makefsdata.su

.PHONY: clean-lwip-2f-src-2f-apps-2f-http-2f-makefsdata

