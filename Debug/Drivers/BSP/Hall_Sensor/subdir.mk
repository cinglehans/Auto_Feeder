################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Hall_Sensor/Hall_Sensor.c 

OBJS += \
./Drivers/BSP/Hall_Sensor/Hall_Sensor.o 

C_DEPS += \
./Drivers/BSP/Hall_Sensor/Hall_Sensor.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Hall_Sensor/%.o Drivers/BSP/Hall_Sensor/%.su Drivers/BSP/Hall_Sensor/%.cyclo: ../Drivers/BSP/Hall_Sensor/%.c Drivers/BSP/Hall_Sensor/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Hall_Sensor

clean-Drivers-2f-BSP-2f-Hall_Sensor:
	-$(RM) ./Drivers/BSP/Hall_Sensor/Hall_Sensor.cyclo ./Drivers/BSP/Hall_Sensor/Hall_Sensor.d ./Drivers/BSP/Hall_Sensor/Hall_Sensor.o ./Drivers/BSP/Hall_Sensor/Hall_Sensor.su

.PHONY: clean-Drivers-2f-BSP-2f-Hall_Sensor

