################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../commons/config.o \
../commons/error.o \
../commons/log.o \
../commons/process.o \
../commons/string.o \
../commons/temporal.o \
../commons/txt.o 

C_SRCS += \
../commons/config.c \
../commons/error.c \
../commons/log.c \
../commons/process.c \
../commons/string.c \
../commons/temporal.c \
../commons/txt.c 

OBJS += \
./commons/config.o \
./commons/error.o \
./commons/log.o \
./commons/process.o \
./commons/string.o \
./commons/temporal.o \
./commons/txt.o 

C_DEPS += \
./commons/config.d \
./commons/error.d \
./commons/log.d \
./commons/process.d \
./commons/string.d \
./commons/temporal.d \
./commons/txt.d 


# Each subdirectory must supply rules for building sources it contributes
commons/%.o: ../commons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2014-2c-game-of-codes/Sockets" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


