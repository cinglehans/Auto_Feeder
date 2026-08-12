################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/key/key.c 

OBJS += \
./Drivers/BSP/key/key.o 

C_DEPS += \
./Drivers/BSP/key/key.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/key/%.o Drivers/BSP/key/%.su Drivers/BSP/key/%.cyclo: ../Drivers/BSP/key/%.c Drivers/BSP/key/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-key

clean-Drivers-2f-BSP-2f-key:
	-$(RM) ./Drivers/BSP/key/key.cyclo ./Drivers/BSP/key/key.d ./Drivers/BSP/key/key.o ./Drivers/BSP/key/key.su

.PHONY: clean-Drivers-2f-BSP-2f-key

