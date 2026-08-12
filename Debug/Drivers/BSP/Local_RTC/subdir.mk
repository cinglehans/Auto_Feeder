################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Local_RTC/RTC.c 

OBJS += \
./Drivers/BSP/Local_RTC/RTC.o 

C_DEPS += \
./Drivers/BSP/Local_RTC/RTC.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Local_RTC/%.o Drivers/BSP/Local_RTC/%.su Drivers/BSP/Local_RTC/%.cyclo: ../Drivers/BSP/Local_RTC/%.c Drivers/BSP/Local_RTC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Local_RTC

clean-Drivers-2f-BSP-2f-Local_RTC:
	-$(RM) ./Drivers/BSP/Local_RTC/RTC.cyclo ./Drivers/BSP/Local_RTC/RTC.d ./Drivers/BSP/Local_RTC/RTC.o ./Drivers/BSP/Local_RTC/RTC.su

.PHONY: clean-Drivers-2f-BSP-2f-Local_RTC

