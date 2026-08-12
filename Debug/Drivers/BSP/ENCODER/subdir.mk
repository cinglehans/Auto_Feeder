################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/ENCODER/encoder.c 

OBJS += \
./Drivers/BSP/ENCODER/encoder.o 

C_DEPS += \
./Drivers/BSP/ENCODER/encoder.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/ENCODER/%.o Drivers/BSP/ENCODER/%.su Drivers/BSP/ENCODER/%.cyclo: ../Drivers/BSP/ENCODER/%.c Drivers/BSP/ENCODER/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-ENCODER

clean-Drivers-2f-BSP-2f-ENCODER:
	-$(RM) ./Drivers/BSP/ENCODER/encoder.cyclo ./Drivers/BSP/ENCODER/encoder.d ./Drivers/BSP/ENCODER/encoder.o ./Drivers/BSP/ENCODER/encoder.su

.PHONY: clean-Drivers-2f-BSP-2f-ENCODER

