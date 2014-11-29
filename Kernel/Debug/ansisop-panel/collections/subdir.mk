################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ansisop-panel/collections/list.c 

OBJS += \
./ansisop-panel/collections/list.o 

C_DEPS += \
./ansisop-panel/collections/list.d 


# Each subdirectory must supply rules for building sources it contributes
ansisop-panel/collections/%.o: ../ansisop-panel/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2014-2c-game-of-codes/commons" -I"/home/utnso/tp-2014-2c-game-of-codes/Sockets" -O2 -g -Wall -c -fmessage-length=0 -Wno-unused-result -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


