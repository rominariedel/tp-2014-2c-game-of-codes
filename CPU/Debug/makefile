################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm

# All of the sources participating in the build are defined here
-include sources.mk
-include funciones/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: CPU

dependents:
	-cd /home/utnso/tp-2014-2c-game-of-codes/commons/Debug && $(MAKE) all
	-cd /home/utnso/workspace/Sockets/Debug && $(MAKE) all

# Tool invocations
CPU: $(OBJS) $(USER_OBJS) /home/utnso/tp-2014-2c-game-of-codes/commons/Debug/libcommons.so /home/utnso/workspace/Sockets/Debug/libSockets.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/Sockets/Debug" -L"/home/utnso/tp-2014-2c-game-of-codes/commons/Debug" -o "CPU" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) CPU
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/tp-2014-2c-game-of-codes/commons/Debug/libcommons.so:
/home/utnso/workspace/Sockets/Debug/libSockets.so:

-include ../makefile.targets
