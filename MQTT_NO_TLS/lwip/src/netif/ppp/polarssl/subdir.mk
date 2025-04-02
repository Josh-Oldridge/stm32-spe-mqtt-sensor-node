################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/netif/ppp/polarssl/arc4.c \
../lwip/src/netif/ppp/polarssl/des.c \
../lwip/src/netif/ppp/polarssl/md4.c \
../lwip/src/netif/ppp/polarssl/md5.c \
../lwip/src/netif/ppp/polarssl/sha1.c 

OBJS += \
./lwip/src/netif/ppp/polarssl/arc4.o \
./lwip/src/netif/ppp/polarssl/des.o \
./lwip/src/netif/ppp/polarssl/md4.o \
./lwip/src/netif/ppp/polarssl/md5.o \
./lwip/src/netif/ppp/polarssl/sha1.o 

C_DEPS += \
./lwip/src/netif/ppp/polarssl/arc4.d \
./lwip/src/netif/ppp/polarssl/des.d \
./lwip/src/netif/ppp/polarssl/md4.d \
./lwip/src/netif/ppp/polarssl/md5.d \
./lwip/src/netif/ppp/polarssl/sha1.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/netif/ppp/polarssl/%.o lwip/src/netif/ppp/polarssl/%.su lwip/src/netif/ppp/polarssl/%.cyclo: ../lwip/src/netif/ppp/polarssl/%.c lwip/src/netif/ppp/polarssl/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_LWIP -DUSE_NUCLEO -DSPI_OA_EN -DSPI_PROT_EN -DADIN1110 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L496xx -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/netif" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I"C:/ST/workspace_InProgress/ADIN1110/lwip" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/library" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/include" -O0 -ffunction-sections -fdata-sections -Wall -w -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-netif-2f-ppp-2f-polarssl

clean-lwip-2f-src-2f-netif-2f-ppp-2f-polarssl:
	-$(RM) ./lwip/src/netif/ppp/polarssl/arc4.cyclo ./lwip/src/netif/ppp/polarssl/arc4.d ./lwip/src/netif/ppp/polarssl/arc4.o ./lwip/src/netif/ppp/polarssl/arc4.su ./lwip/src/netif/ppp/polarssl/des.cyclo ./lwip/src/netif/ppp/polarssl/des.d ./lwip/src/netif/ppp/polarssl/des.o ./lwip/src/netif/ppp/polarssl/des.su ./lwip/src/netif/ppp/polarssl/md4.cyclo ./lwip/src/netif/ppp/polarssl/md4.d ./lwip/src/netif/ppp/polarssl/md4.o ./lwip/src/netif/ppp/polarssl/md4.su ./lwip/src/netif/ppp/polarssl/md5.cyclo ./lwip/src/netif/ppp/polarssl/md5.d ./lwip/src/netif/ppp/polarssl/md5.o ./lwip/src/netif/ppp/polarssl/md5.su ./lwip/src/netif/ppp/polarssl/sha1.cyclo ./lwip/src/netif/ppp/polarssl/sha1.d ./lwip/src/netif/ppp/polarssl/sha1.o ./lwip/src/netif/ppp/polarssl/sha1.su

.PHONY: clean-lwip-2f-src-2f-netif-2f-ppp-2f-polarssl

