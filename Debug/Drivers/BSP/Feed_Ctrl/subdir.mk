################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/Feed_Ctrl/Feed_Ctrl.c 

OBJS += \
./Drivers/BSP/Feed_Ctrl/Feed_Ctrl.o 

C_DEPS += \
./Drivers/BSP/Feed_Ctrl/Feed_Ctrl.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Feed_Ctrl/%.o Drivers/BSP/Feed_Ctrl/%.su Drivers/BSP/Feed_Ctrl/%.cyclo: ../Drivers/BSP/Feed_Ctrl/%.c Drivers/BSP/Feed_Ctrl/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xE -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Feed_Ctrl

clean-Drivers-2f-BSP-2f-Feed_Ctrl:
	-$(RM) ./Drivers/BSP/Feed_Ctrl/Feed_Ctrl.cyclo ./Drivers/BSP/Feed_Ctrl/Feed_Ctrl.d ./Drivers/BSP/Feed_Ctrl/Feed_Ctrl.o ./Drivers/BSP/Feed_Ctrl/Feed_Ctrl.su

.PHONY: clean-Drivers-2f-BSP-2f-Feed_Ctrl

