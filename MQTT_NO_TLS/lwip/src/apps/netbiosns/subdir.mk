################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/apps/netbiosns/netbiosns.c 

OBJS += \
./lwip/src/apps/netbiosns/netbiosns.o 

C_DEPS += \
./lwip/src/apps/netbiosns/netbiosns.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/apps/netbiosns/%.o lwip/src/apps/netbiosns/%.su lwip/src/apps/netbiosns/%.cyclo: ../lwip/src/apps/netbiosns/%.c lwip/src/apps/netbiosns/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_LWIP -DUSE_NUCLEO -DSPI_OA_EN -DSPI_PROT_EN -DADIN1110 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L496xx -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/netif" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I"C:/ST/workspace_InProgress/ADIN1110/lwip" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/library" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/include" -O0 -ffunction-sections -fdata-sections -Wall -w -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-apps-2f-netbiosns

clean-lwip-2f-src-2f-apps-2f-netbiosns:
	-$(RM) ./lwip/src/apps/netbiosns/netbiosns.cyclo ./lwip/src/apps/netbiosns/netbiosns.d ./lwip/src/apps/netbiosns/netbiosns.o ./lwip/src/apps/netbiosns/netbiosns.su

.PHONY: clean-lwip-2f-src-2f-apps-2f-netbiosns

