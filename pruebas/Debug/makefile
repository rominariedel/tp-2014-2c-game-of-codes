################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
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
all: pruebas

dependents:
	-cd /home/utnso/git/tp-2014-2c-game-of-codes/Sockets/Debug && $(MAKE) all
	-cd /home/utnso/git/tp-2014-2c-game-of-codes/commons/Debug && $(MAKE) all

# Tool invocations
pruebas: $(OBJS) $(USER_OBJS) /home/utnso/git/tp-2014-2c-game-of-codes/Sockets/Debug/libSockets.so /home/utnso/git/tp-2014-2c-game-of-codes/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/git/tp-2014-2c-game-of-codes/commons/Debug" -L"/home/utnso/git/tp-2014-2c-game-of-codes/Sockets/Debug" -o "pruebas" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) pruebas
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/git/tp-2014-2c-game-of-codes/Sockets/Debug/libSockets.so:
/home/utnso/git/tp-2014-2c-game-of-codes/commons/Debug/libcommons.so:

-include ../makefile.targets
