################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../funciones/funcionesKernel.c \
../funciones/funcionesMSP.c \
../funciones/instruccionesESO.c 

OBJS += \
./funciones/funcionesKernel.o \
./funciones/funcionesMSP.o \
./funciones/instruccionesESO.o 

C_DEPS += \
./funciones/funcionesKernel.d \
./funciones/funcionesMSP.d \
./funciones/instruccionesESO.d 


# Each subdirectory must supply rules for building sources it contributes
funciones/%.o: ../funciones/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2014-2c-game-of-codes/Sockets" -I"/home/utnso/git/tp-2014-2c-game-of-codes/commons" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


