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
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DMQTT_TLS -DUSE_LWIP -DUSE_NUCLEO -DLWIP_ALTCP_TLS_MBEDTLS -DLWIP_ALTCP -DLWIP_ALTCP_TLS -DSPI_OA_EN -DSPI_PROT_EN -DADIN1110 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L496xx -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/netif" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I"C:/ST/workspace_InProgress/ADIN1110/lwip" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/library" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/include" -O0 -ffunction-sections -fdata-sections -Wall -w -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-apps-2f-lwiperf

clean-lwip-2f-src-2f-apps-2f-lwiperf:
	-$(RM) ./lwip/src/apps/lwiperf/lwiperf.cyclo ./lwip/src/apps/lwiperf/lwiperf.d ./lwip/src/apps/lwiperf/lwiperf.o ./lwip/src/apps/lwiperf/lwiperf.su

.PHONY: clean-lwip-2f-src-2f-apps-2f-lwiperf

