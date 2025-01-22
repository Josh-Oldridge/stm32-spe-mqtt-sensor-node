################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/apps/http/altcp_proxyconnect.c \
../lwip/src/apps/http/fs.c \
../lwip/src/apps/http/http_client.c \
../lwip/src/apps/http/httpd.c 

OBJS += \
./lwip/src/apps/http/altcp_proxyconnect.o \
./lwip/src/apps/http/fs.o \
./lwip/src/apps/http/http_client.o \
./lwip/src/apps/http/httpd.o 

C_DEPS += \
./lwip/src/apps/http/altcp_proxyconnect.d \
./lwip/src/apps/http/fs.d \
./lwip/src/apps/http/http_client.d \
./lwip/src/apps/http/httpd.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/apps/http/%.o lwip/src/apps/http/%.su lwip/src/apps/http/%.cyclo: ../lwip/src/apps/http/%.c lwip/src/apps/http/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSPI_PROT_EN -DSPI_OA_EN -DADIN1110 -DUSE_HAL_DRIVER -DUSE_LWIP -DSTM32L496xx -c -I../Core/Inc -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/netif" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/include" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -Wno-format-extra-args -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-apps-2f-http

clean-lwip-2f-src-2f-apps-2f-http:
	-$(RM) ./lwip/src/apps/http/altcp_proxyconnect.cyclo ./lwip/src/apps/http/altcp_proxyconnect.d ./lwip/src/apps/http/altcp_proxyconnect.o ./lwip/src/apps/http/altcp_proxyconnect.su ./lwip/src/apps/http/fs.cyclo ./lwip/src/apps/http/fs.d ./lwip/src/apps/http/fs.o ./lwip/src/apps/http/fs.su ./lwip/src/apps/http/http_client.cyclo ./lwip/src/apps/http/http_client.d ./lwip/src/apps/http/http_client.o ./lwip/src/apps/http/http_client.su ./lwip/src/apps/http/httpd.cyclo ./lwip/src/apps/http/httpd.d ./lwip/src/apps/http/httpd.o ./lwip/src/apps/http/httpd.su

.PHONY: clean-lwip-2f-src-2f-apps-2f-http

