################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../panel/commons/collections/dictionary.c \
../panel/commons/collections/list.c \
../panel/commons/collections/queue.c 

OBJS += \
./panel/commons/collections/dictionary.o \
./panel/commons/collections/list.o \
./panel/commons/collections/queue.o 

C_DEPS += \
./panel/commons/collections/dictionary.d \
./panel/commons/collections/list.d \
./panel/commons/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
panel/commons/collections/%.o: ../panel/commons/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2014-2c-game-of-codes/Sockets" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


