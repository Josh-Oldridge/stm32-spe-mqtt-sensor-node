################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lwip/src/netif/ppp/auth.c \
../lwip/src/netif/ppp/ccp.c \
../lwip/src/netif/ppp/chap-md5.c \
../lwip/src/netif/ppp/chap-new.c \
../lwip/src/netif/ppp/chap_ms.c \
../lwip/src/netif/ppp/demand.c \
../lwip/src/netif/ppp/eap.c \
../lwip/src/netif/ppp/ecp.c \
../lwip/src/netif/ppp/eui64.c \
../lwip/src/netif/ppp/fsm.c \
../lwip/src/netif/ppp/ipcp.c \
../lwip/src/netif/ppp/ipv6cp.c \
../lwip/src/netif/ppp/lcp.c \
../lwip/src/netif/ppp/magic.c \
../lwip/src/netif/ppp/mppe.c \
../lwip/src/netif/ppp/multilink.c \
../lwip/src/netif/ppp/ppp.c \
../lwip/src/netif/ppp/pppapi.c \
../lwip/src/netif/ppp/pppcrypt.c \
../lwip/src/netif/ppp/pppoe.c \
../lwip/src/netif/ppp/pppol2tp.c \
../lwip/src/netif/ppp/pppos.c \
../lwip/src/netif/ppp/upap.c \
../lwip/src/netif/ppp/utils.c \
../lwip/src/netif/ppp/vj.c 

OBJS += \
./lwip/src/netif/ppp/auth.o \
./lwip/src/netif/ppp/ccp.o \
./lwip/src/netif/ppp/chap-md5.o \
./lwip/src/netif/ppp/chap-new.o \
./lwip/src/netif/ppp/chap_ms.o \
./lwip/src/netif/ppp/demand.o \
./lwip/src/netif/ppp/eap.o \
./lwip/src/netif/ppp/ecp.o \
./lwip/src/netif/ppp/eui64.o \
./lwip/src/netif/ppp/fsm.o \
./lwip/src/netif/ppp/ipcp.o \
./lwip/src/netif/ppp/ipv6cp.o \
./lwip/src/netif/ppp/lcp.o \
./lwip/src/netif/ppp/magic.o \
./lwip/src/netif/ppp/mppe.o \
./lwip/src/netif/ppp/multilink.o \
./lwip/src/netif/ppp/ppp.o \
./lwip/src/netif/ppp/pppapi.o \
./lwip/src/netif/ppp/pppcrypt.o \
./lwip/src/netif/ppp/pppoe.o \
./lwip/src/netif/ppp/pppol2tp.o \
./lwip/src/netif/ppp/pppos.o \
./lwip/src/netif/ppp/upap.o \
./lwip/src/netif/ppp/utils.o \
./lwip/src/netif/ppp/vj.o 

C_DEPS += \
./lwip/src/netif/ppp/auth.d \
./lwip/src/netif/ppp/ccp.d \
./lwip/src/netif/ppp/chap-md5.d \
./lwip/src/netif/ppp/chap-new.d \
./lwip/src/netif/ppp/chap_ms.d \
./lwip/src/netif/ppp/demand.d \
./lwip/src/netif/ppp/eap.d \
./lwip/src/netif/ppp/ecp.d \
./lwip/src/netif/ppp/eui64.d \
./lwip/src/netif/ppp/fsm.d \
./lwip/src/netif/ppp/ipcp.d \
./lwip/src/netif/ppp/ipv6cp.d \
./lwip/src/netif/ppp/lcp.d \
./lwip/src/netif/ppp/magic.d \
./lwip/src/netif/ppp/mppe.d \
./lwip/src/netif/ppp/multilink.d \
./lwip/src/netif/ppp/ppp.d \
./lwip/src/netif/ppp/pppapi.d \
./lwip/src/netif/ppp/pppcrypt.d \
./lwip/src/netif/ppp/pppoe.d \
./lwip/src/netif/ppp/pppol2tp.d \
./lwip/src/netif/ppp/pppos.d \
./lwip/src/netif/ppp/upap.d \
./lwip/src/netif/ppp/utils.d \
./lwip/src/netif/ppp/vj.d 


# Each subdirectory must supply rules for building sources it contributes
lwip/src/netif/ppp/%.o lwip/src/netif/ppp/%.su lwip/src/netif/ppp/%.cyclo: ../lwip/src/netif/ppp/%.c lwip/src/netif/ppp/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSPI_PROT_EN -DSPI_OA_EN -DADIN1110 -DUSE_HAL_DRIVER -DUSE_LWIP -DSTM32L496xx -c -I../Core/Inc -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/api" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/apps" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/core" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/netif" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src" -I"C:/Users/ZOJOLDRI/STM32CubeIDE/workspace_InProgress/ADIN1110/lwip/src/include" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -Wno-format -Wno-format-extra-args -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-lwip-2f-src-2f-netif-2f-ppp

clean-lwip-2f-src-2f-netif-2f-ppp:
	-$(RM) ./lwip/src/netif/ppp/auth.cyclo ./lwip/src/netif/ppp/auth.d ./lwip/src/netif/ppp/auth.o ./lwip/src/netif/ppp/auth.su ./lwip/src/netif/ppp/ccp.cyclo ./lwip/src/netif/ppp/ccp.d ./lwip/src/netif/ppp/ccp.o ./lwip/src/netif/ppp/ccp.su ./lwip/src/netif/ppp/chap-md5.cyclo ./lwip/src/netif/ppp/chap-md5.d ./lwip/src/netif/ppp/chap-md5.o ./lwip/src/netif/ppp/chap-md5.su ./lwip/src/netif/ppp/chap-new.cyclo ./lwip/src/netif/ppp/chap-new.d ./lwip/src/netif/ppp/chap-new.o ./lwip/src/netif/ppp/chap-new.su ./lwip/src/netif/ppp/chap_ms.cyclo ./lwip/src/netif/ppp/chap_ms.d ./lwip/src/netif/ppp/chap_ms.o ./lwip/src/netif/ppp/chap_ms.su ./lwip/src/netif/ppp/demand.cyclo ./lwip/src/netif/ppp/demand.d ./lwip/src/netif/ppp/demand.o ./lwip/src/netif/ppp/demand.su ./lwip/src/netif/ppp/eap.cyclo ./lwip/src/netif/ppp/eap.d ./lwip/src/netif/ppp/eap.o ./lwip/src/netif/ppp/eap.su ./lwip/src/netif/ppp/ecp.cyclo ./lwip/src/netif/ppp/ecp.d ./lwip/src/netif/ppp/ecp.o ./lwip/src/netif/ppp/ecp.su ./lwip/src/netif/ppp/eui64.cyclo ./lwip/src/netif/ppp/eui64.d ./lwip/src/netif/ppp/eui64.o ./lwip/src/netif/ppp/eui64.su ./lwip/src/netif/ppp/fsm.cyclo ./lwip/src/netif/ppp/fsm.d ./lwip/src/netif/ppp/fsm.o ./lwip/src/netif/ppp/fsm.su ./lwip/src/netif/ppp/ipcp.cyclo ./lwip/src/netif/ppp/ipcp.d ./lwip/src/netif/ppp/ipcp.o ./lwip/src/netif/ppp/ipcp.su ./lwip/src/netif/ppp/ipv6cp.cyclo ./lwip/src/netif/ppp/ipv6cp.d ./lwip/src/netif/ppp/ipv6cp.o ./lwip/src/netif/ppp/ipv6cp.su ./lwip/src/netif/ppp/lcp.cyclo ./lwip/src/netif/ppp/lcp.d ./lwip/src/netif/ppp/lcp.o ./lwip/src/netif/ppp/lcp.su ./lwip/src/netif/ppp/magic.cyclo ./lwip/src/netif/ppp/magic.d ./lwip/src/netif/ppp/magic.o ./lwip/src/netif/ppp/magic.su ./lwip/src/netif/ppp/mppe.cyclo ./lwip/src/netif/ppp/mppe.d ./lwip/src/netif/ppp/mppe.o ./lwip/src/netif/ppp/mppe.su ./lwip/src/netif/ppp/multilink.cyclo ./lwip/src/netif/ppp/multilink.d ./lwip/src/netif/ppp/multilink.o ./lwip/src/netif/ppp/multilink.su ./lwip/src/netif/ppp/ppp.cyclo ./lwip/src/netif/ppp/ppp.d ./lwip/src/netif/ppp/ppp.o ./lwip/src/netif/ppp/ppp.su ./lwip/src/netif/ppp/pppapi.cyclo ./lwip/src/netif/ppp/pppapi.d ./lwip/src/netif/ppp/pppapi.o ./lwip/src/netif/ppp/pppapi.su ./lwip/src/netif/ppp/pppcrypt.cyclo ./lwip/src/netif/ppp/pppcrypt.d ./lwip/src/netif/ppp/pppcrypt.o ./lwip/src/netif/ppp/pppcrypt.su ./lwip/src/netif/ppp/pppoe.cyclo ./lwip/src/netif/ppp/pppoe.d ./lwip/src/netif/ppp/pppoe.o ./lwip/src/netif/ppp/pppoe.su ./lwip/src/netif/ppp/pppol2tp.cyclo ./lwip/src/netif/ppp/pppol2tp.d ./lwip/src/netif/ppp/pppol2tp.o ./lwip/src/netif/ppp/pppol2tp.su ./lwip/src/netif/ppp/pppos.cyclo ./lwip/src/netif/ppp/pppos.d ./lwip/src/netif/ppp/pppos.o ./lwip/src/netif/ppp/pppos.su ./lwip/src/netif/ppp/upap.cyclo ./lwip/src/netif/ppp/upap.d ./lwip/src/netif/ppp/upap.o ./lwip/src/netif/ppp/upap.su ./lwip/src/netif/ppp/utils.cyclo ./lwip/src/netif/ppp/utils.d ./lwip/src/netif/ppp/utils.o ./lwip/src/netif/ppp/utils.su ./lwip/src/netif/ppp/vj.cyclo ./lwip/src/netif/ppp/vj.d ./lwip/src/netif/ppp/vj.o ./lwip/src/netif/ppp/vj.su

.PHONY: clean-lwip-2f-src-2f-netif-2f-ppp

