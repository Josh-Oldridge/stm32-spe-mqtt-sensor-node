################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/core/ipv6/dhcp6.c \
../lwip/src/core/ipv6/ethip6.c \
../lwip/src/core/ipv6/icmp6.c \
../lwip/src/core/ipv6/inet6.c \
../lwip/src/core/ipv6/ip6.c \
../lwip/src/core/ipv6/ip6_addr.c \
../lwip/src/core/ipv6/ip6_frag.c \
../lwip/src/core/ipv6/mld6.c \
../lwip/src/core/ipv6/nd6.c 

OBJS += \
./lwip/src/core/ipv6/dhcp6.o \
./lwip/src/core/ipv6/ethip6.o \
./lwip/src/core/ipv6/icmp6.o \
./lwip/src/core/ipv6/inet6.o \
./lwip/src/core/ipv6/ip6.o \
./lwip/src/core/ipv6/ip6_addr.o \
./lwip/src/core/ipv6/ip6_frag.o \
./lwip/src/core/ipv6/mld6.o \
./lwip/src/core/ipv6/nd6.o 

C_DEPS += \
./lwip/src/core/ipv6/dhcp6.d \
./lwip/src/core/ipv6/ethip6.d \
./lwip/src/core/ipv6/icmp6.d \
./lwip/src/core/ipv6/inet6.d \
./lwip/src/core/ipv6/ip6.d \
./lwip/src/core/ipv6/ip6_addr.d \
./lwip/src/core/ipv6/ip6_frag.d \
./lwip/src/core/ipv6/mld6.d \
./lwip/src/core/ipv6/nd6.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/core/ipv6/%.o lwip/src/core/ipv6/%.su lwip/src/core/ipv6/%.cyclo: ../lwip/src/core/ipv6/%.c lwip/src/core/ipv6/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_LWIP -DUSE_NUCLEO -DSPI_OA_EN -DSPI_PROT_EN -DADIN1110 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L496xx -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/netif" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I"C:/ST/workspace_InProgress/ADIN1110/lwip" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/library" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/include" -O0 -ffunction-sections -fdata-sections -Wall -w -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-core-2f-ipv6

clean-lwip-2f-src-2f-core-2f-ipv6:
	-$(RM) ./lwip/src/core/ipv6/dhcp6.cyclo ./lwip/src/core/ipv6/dhcp6.d ./lwip/src/core/ipv6/dhcp6.o ./lwip/src/core/ipv6/dhcp6.su ./lwip/src/core/ipv6/ethip6.cyclo ./lwip/src/core/ipv6/ethip6.d ./lwip/src/core/ipv6/ethip6.o ./lwip/src/core/ipv6/ethip6.su ./lwip/src/core/ipv6/icmp6.cyclo ./lwip/src/core/ipv6/icmp6.d ./lwip/src/core/ipv6/icmp6.o ./lwip/src/core/ipv6/icmp6.su ./lwip/src/core/ipv6/inet6.cyclo ./lwip/src/core/ipv6/inet6.d ./lwip/src/core/ipv6/inet6.o ./lwip/src/core/ipv6/inet6.su ./lwip/src/core/ipv6/ip6.cyclo ./lwip/src/core/ipv6/ip6.d ./lwip/src/core/ipv6/ip6.o ./lwip/src/core/ipv6/ip6.su ./lwip/src/core/ipv6/ip6_addr.cyclo ./lwip/src/core/ipv6/ip6_addr.d ./lwip/src/core/ipv6/ip6_addr.o ./lwip/src/core/ipv6/ip6_addr.su ./lwip/src/core/ipv6/ip6_frag.cyclo ./lwip/src/core/ipv6/ip6_frag.d ./lwip/src/core/ipv6/ip6_frag.o ./lwip/src/core/ipv6/ip6_frag.su ./lwip/src/core/ipv6/mld6.cyclo ./lwip/src/core/ipv6/mld6.d ./lwip/src/core/ipv6/mld6.o ./lwip/src/core/ipv6/mld6.su ./lwip/src/core/ipv6/nd6.cyclo ./lwip/src/core/ipv6/nd6.d ./lwip/src/core/ipv6/nd6.o ./lwip/src/core/ipv6/nd6.su

.PHONY: clean-lwip-2f-src-2f-core-2f-ipv6

