################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm

# All of the sources participating in the build are defined here
-include sources.mk
-include panel/subdir.mk
-include auxiliares/subdir.mk
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
all: Kernel

dependents:
	-cd /home/utnso/Escritorio/tp-2014-2c-game-of-codes/commons/Debug && $(MAKE) clean all

# Tool invocations
Kernel: $(OBJS) $(USER_OBJS) /home/utnso/Escritorio/tp-2014-2c-game-of-codes/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/Escritorio/tp-2014-2c-game-of-codes/commons/Debug" -o "Kernel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Kernel
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/Escritorio/tp-2014-2c-game-of-codes/commons/Debug/libcommons.so:

-include ../makefile.targets
