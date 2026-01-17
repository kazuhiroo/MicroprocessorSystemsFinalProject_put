################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../AddOns/Src/Filters.c \
../AddOns/Src/PID.c 

OBJS += \
./AddOns/Src/Filters.o \
./AddOns/Src/PID.o 

C_DEPS += \
./AddOns/Src/Filters.d \
./AddOns/Src/PID.d 


# Each subdirectory must supply rules for building sources it contributes
AddOns/Src/%.o AddOns/Src/%.su AddOns/Src/%.cyclo: ../AddOns/Src/%.c AddOns/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -c -I../Core/Inc -I../AddOns/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/Admin/Documents/studia/sem5/projekt systemy mikroprocesorowe/program/AddOns/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-AddOns-2f-Src

clean-AddOns-2f-Src:
	-$(RM) ./AddOns/Src/Filters.cyclo ./AddOns/Src/Filters.d ./AddOns/Src/Filters.o ./AddOns/Src/Filters.su ./AddOns/Src/PID.cyclo ./AddOns/Src/PID.d ./AddOns/Src/PID.o ./AddOns/Src/PID.su

.PHONY: clean-AddOns-2f-Src

