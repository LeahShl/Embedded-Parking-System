################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../GPS_sim/Src/gps_sim.c \
../GPS_sim/Src/i2c_send.c 

OBJS += \
./GPS_sim/Src/gps_sim.o \
./GPS_sim/Src/i2c_send.o 

C_DEPS += \
./GPS_sim/Src/gps_sim.d \
./GPS_sim/Src/i2c_send.d 


# Each subdirectory must supply rules for building sources it contributes
GPS_sim/Src/%.o GPS_sim/Src/%.su GPS_sim/Src/%.cyclo: ../GPS_sim/Src/%.c GPS_sim/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"/home/leah/Documents/RT_Embedded_Course/6.Embedded Linux/Embedded-Parking-System/STM32/GPS_sim/Inc" -I"/home/leah/Documents/RT_Embedded_Course/6.Embedded Linux/Embedded-Parking-System/STM32/GPS_sim/Src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-GPS_sim-2f-Src

clean-GPS_sim-2f-Src:
	-$(RM) ./GPS_sim/Src/gps_sim.cyclo ./GPS_sim/Src/gps_sim.d ./GPS_sim/Src/gps_sim.o ./GPS_sim/Src/gps_sim.su ./GPS_sim/Src/i2c_send.cyclo ./GPS_sim/Src/i2c_send.d ./GPS_sim/Src/i2c_send.o ./GPS_sim/Src/i2c_send.su

.PHONY: clean-GPS_sim-2f-Src

