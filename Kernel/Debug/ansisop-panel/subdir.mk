################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ansisop-panel/kernel.c \
../ansisop-panel/panel.c 

OBJS += \
./ansisop-panel/kernel.o \
./ansisop-panel/panel.o 

C_DEPS += \
./ansisop-panel/kernel.d \
./ansisop-panel/panel.d 


# Each subdirectory must supply rules for building sources it contributes
ansisop-panel/%.o: ../ansisop-panel/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2014-2c-game-of-codes/commons" -I"/home/utnso/git/tp-2014-2c-game-of-codes/Sockets" -O2 -g -Wall -c -fmessage-length=0 -Wno-unused-result -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


