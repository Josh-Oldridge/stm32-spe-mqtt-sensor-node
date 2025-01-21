################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/apps/altcp_tls/altcp_tls_mbedtls.c \
../lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.c 

OBJS += \
./lwip/src/apps/altcp_tls/altcp_tls_mbedtls.o \
./lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.o 

C_DEPS += \
./lwip/src/apps/altcp_tls/altcp_tls_mbedtls.d \
./lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/apps/altcp_tls/%.o lwip/src/apps/altcp_tls/%.su lwip/src/apps/altcp_tls/%.cyclo: ../lwip/src/apps/altcp_tls/%.c lwip/src/apps/altcp_tls/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_LWIP -DSPI_PROT_EN -DSPI_OA_EN -DADIN1110 -DUSE_HAL_DRIVER -DSTM32L496xx -c -I../Core/Inc -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/netif" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/include" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -Wno-format-extra-args -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-apps-2f-altcp_tls

clean-lwip-2f-src-2f-apps-2f-altcp_tls:
	-$(RM) ./lwip/src/apps/altcp_tls/altcp_tls_mbedtls.cyclo ./lwip/src/apps/altcp_tls/altcp_tls_mbedtls.d ./lwip/src/apps/altcp_tls/altcp_tls_mbedtls.o ./lwip/src/apps/altcp_tls/altcp_tls_mbedtls.su ./lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.cyclo ./lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.d ./lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.o ./lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.su

.PHONY: clean-lwip-2f-src-2f-apps-2f-altcp_tls

