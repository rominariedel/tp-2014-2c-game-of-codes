################################################################################

# Automatically-generated file. Do not edit!

################################################################################



-include ../makefile.init



RM := rm -rf

CC=gcc



C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')

H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')

OBJS=$(C_SRCS:./%.c=build/%.o)

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

all: libSockets.so



# Tool invocations

libSockets.so: $(OBJS)

	$(CC) -shared -o "libSockets.so" $(OBJS)



../%.o: ../%.c

	$(CC) -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"



# Add debug parameters and compile

debug: CC += -DDEBUG -g

debug: all



# Other Targets

clean:

	-$(RM) $(OBJS)$(C_DEPS)$(LIBRARIES) libSockets.so

	-@echo ' '





install: all

	cp -u libSockets.so /usr/lib

	cp --parents -u libSockets.so $(H_SRCS) /usr/include



uninstall:

	rm -f /usr/lib/libSockets.so

	rm -rf /usr/include/Sockets



.PHONY: all clean install uninstall
