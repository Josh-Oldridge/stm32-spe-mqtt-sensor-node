################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/netif/bridgeif.c \
../lwip/src/netif/bridgeif_fdb.c \
../lwip/src/netif/ethernet.c \
../lwip/src/netif/lowpan6.c \
../lwip/src/netif/lowpan6_ble.c \
../lwip/src/netif/lowpan6_common.c \
../lwip/src/netif/slipif.c \
../lwip/src/netif/zepif.c 

OBJS += \
./lwip/src/netif/bridgeif.o \
./lwip/src/netif/bridgeif_fdb.o \
./lwip/src/netif/ethernet.o \
./lwip/src/netif/lowpan6.o \
./lwip/src/netif/lowpan6_ble.o \
./lwip/src/netif/lowpan6_common.o \
./lwip/src/netif/slipif.o \
./lwip/src/netif/zepif.o 

C_DEPS += \
./lwip/src/netif/bridgeif.d \
./lwip/src/netif/bridgeif_fdb.d \
./lwip/src/netif/ethernet.d \
./lwip/src/netif/lowpan6.d \
./lwip/src/netif/lowpan6_ble.d \
./lwip/src/netif/lowpan6_common.d \
./lwip/src/netif/slipif.d \
./lwip/src/netif/zepif.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/netif/%.o lwip/src/netif/%.su lwip/src/netif/%.cyclo: ../lwip/src/netif/%.c lwip/src/netif/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_LWIP -DUSE_NUCLEO -DSPI_OA_EN -DSPI_PROT_EN -DADIN1110 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L496xx -DSTM32_THREAD_SAFE_STRATEGY=4 -c -I../Core/Inc -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/netif" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Core/ThreadSafe -I"C:/ST/workspace_InProgress/ADIN1110/lwip" -I"C:/ST/workspace_InProgress/ADIN1110/lwip/src/include" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/library" -I"C:/ST/workspace_InProgress/ADIN1110/mbedtls-2.16.12/include" -O0 -ffunction-sections -fdata-sections -Wall -w -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-netif

clean-lwip-2f-src-2f-netif:
	-$(RM) ./lwip/src/netif/bridgeif.cyclo ./lwip/src/netif/bridgeif.d ./lwip/src/netif/bridgeif.o ./lwip/src/netif/bridgeif.su ./lwip/src/netif/bridgeif_fdb.cyclo ./lwip/src/netif/bridgeif_fdb.d ./lwip/src/netif/bridgeif_fdb.o ./lwip/src/netif/bridgeif_fdb.su ./lwip/src/netif/ethernet.cyclo ./lwip/src/netif/ethernet.d ./lwip/src/netif/ethernet.o ./lwip/src/netif/ethernet.su ./lwip/src/netif/lowpan6.cyclo ./lwip/src/netif/lowpan6.d ./lwip/src/netif/lowpan6.o ./lwip/src/netif/lowpan6.su ./lwip/src/netif/lowpan6_ble.cyclo ./lwip/src/netif/lowpan6_ble.d ./lwip/src/netif/lowpan6_ble.o ./lwip/src/netif/lowpan6_ble.su ./lwip/src/netif/lowpan6_common.cyclo ./lwip/src/netif/lowpan6_common.d ./lwip/src/netif/lowpan6_common.o ./lwip/src/netif/lowpan6_common.su ./lwip/src/netif/slipif.cyclo ./lwip/src/netif/slipif.d ./lwip/src/netif/slipif.o ./lwip/src/netif/slipif.su ./lwip/src/netif/zepif.cyclo ./lwip/src/netif/zepif.d ./lwip/src/netif/zepif.o ./lwip/src/netif/zepif.su

.PHONY: clean-lwip-2f-src-2f-netif

