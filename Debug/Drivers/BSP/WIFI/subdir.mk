################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/WIFI/WIFI.c 

OBJS += \
./Drivers/BSP/WIFI/WIFI.o 

C_DEPS += \
./Drivers/BSP/WIFI/WIFI.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/WIFI/%.o Drivers/BSP/WIFI/%.su Drivers/BSP/WIFI/%.cyclo: ../Drivers/BSP/WIFI/%.c Drivers/BSP/WIFI/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-WIFI

clean-Drivers-2f-BSP-2f-WIFI:
	-$(RM) ./Drivers/BSP/WIFI/WIFI.cyclo ./Drivers/BSP/WIFI/WIFI.d ./Drivers/BSP/WIFI/WIFI.o ./Drivers/BSP/WIFI/WIFI.su

.PHONY: clean-Drivers-2f-BSP-2f-WIFI

