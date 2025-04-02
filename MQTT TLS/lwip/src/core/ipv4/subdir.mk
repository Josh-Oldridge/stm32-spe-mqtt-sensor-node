################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/core/ipv4/autoip.c \
../lwip/src/core/ipv4/dhcp.c \
../lwip/src/core/ipv4/etharp.c \
../lwip/src/core/ipv4/icmp.c \
../lwip/src/core/ipv4/igmp.c \
../lwip/src/core/ipv4/ip4.c \
../lwip/src/core/ipv4/ip4_addr.c \
../lwip/src/core/ipv4/ip4_frag.c 

OBJS += \
./lwip/src/core/ipv4/autoip.o \
./lwip/src/core/ipv4/dhcp.o \
./lwip/src/core/ipv4/etharp.o \
./lwip/src/core/ipv4/icmp.o \
./lwip/src/core/ipv4/igmp.o \
./lwip/src/core/ipv4/ip4.o \
./lwip/src/core/ipv4/ip4_addr.o \
./lwip/src/core/ipv4/ip4_frag.o 

C_DEPS += \
./lwip/src/core/ipv4/autoip.d \
./lwip/src/core/ipv4/dhcp.d \
./lwip/src/core/ipv4/etharp.d \
./lwip/src/core/ipv4/icmp.d \
./lwip/src/core/ipv4/igmp.d \
./lwip/src/core/ipv4/ip4.d \
./lwip/src/core/ipv4/ip4_addr.d \
./lwip/src/core/ipv4/ip4_frag.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/core/ipv4/%.o lwip/src/core/ipv4/%.su lwip/src/core/ipv4/%.cyclo: ../lwip/src/core/ipv4/%.c lwip/src/core/ipv4/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DMQTT_TLS -DUSE_LWIP -DUSE_NUCLEO -DLWIP_ALTCP_TLS_MBEDTLS -DLWIP_ALTCP -DLWIP_ALTCP_TLS -DSPI_OA_EN -DSPI_PROT_EN -DADIN1110 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L496xx -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/netif" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I"C:/ST/workspace_InProgress/ADIN1110/lwip" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/library" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/include" -O0 -ffunction-sections -fdata-sections -Wall -w -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-core-2f-ipv4

clean-lwip-2f-src-2f-core-2f-ipv4:
	-$(RM) ./lwip/src/core/ipv4/autoip.cyclo ./lwip/src/core/ipv4/autoip.d ./lwip/src/core/ipv4/autoip.o ./lwip/src/core/ipv4/autoip.su ./lwip/src/core/ipv4/dhcp.cyclo ./lwip/src/core/ipv4/dhcp.d ./lwip/src/core/ipv4/dhcp.o ./lwip/src/core/ipv4/dhcp.su ./lwip/src/core/ipv4/etharp.cyclo ./lwip/src/core/ipv4/etharp.d ./lwip/src/core/ipv4/etharp.o ./lwip/src/core/ipv4/etharp.su ./lwip/src/core/ipv4/icmp.cyclo ./lwip/src/core/ipv4/icmp.d ./lwip/src/core/ipv4/icmp.o ./lwip/src/core/ipv4/icmp.su ./lwip/src/core/ipv4/igmp.cyclo ./lwip/src/core/ipv4/igmp.d ./lwip/src/core/ipv4/igmp.o ./lwip/src/core/ipv4/igmp.su ./lwip/src/core/ipv4/ip4.cyclo ./lwip/src/core/ipv4/ip4.d ./lwip/src/core/ipv4/ip4.o ./lwip/src/core/ipv4/ip4.su ./lwip/src/core/ipv4/ip4_addr.cyclo ./lwip/src/core/ipv4/ip4_addr.d ./lwip/src/core/ipv4/ip4_addr.o ./lwip/src/core/ipv4/ip4_addr.su ./lwip/src/core/ipv4/ip4_frag.cyclo ./lwip/src/core/ipv4/ip4_frag.d ./lwip/src/core/ipv4/ip4_frag.o ./lwip/src/core/ipv4/ip4_frag.su

.PHONY: clean-lwip-2f-src-2f-core-2f-ipv4

