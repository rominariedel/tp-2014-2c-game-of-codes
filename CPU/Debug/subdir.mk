################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CPU.c 

OBJS += \
./CPU.o 

C_DEPS += \
./CPU.d 


# Each subdirectory must supply rules for building sources it contributes
CPU.o: ../CPU.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2014-2c-game-of-codes/commons" -Im -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"CPU.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


