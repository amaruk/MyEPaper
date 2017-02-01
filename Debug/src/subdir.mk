################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/drv_adc.c \
../src/drv_uart.c \
../src/lib_epd.c \
../src/main.c 

OBJS += \
./src/drv_adc.o \
./src/drv_uart.o \
./src/lib_epd.o \
./src/main.o 

C_DEPS += \
./src/drv_adc.d \
./src/drv_uart.d \
./src/lib_epd.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


