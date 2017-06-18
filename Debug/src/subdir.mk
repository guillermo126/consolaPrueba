################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/config.o 

C_SRCS += \
../src/config.c \
../src/consola.c \
../src/funcionesConsola.c 

OBJS += \
./src/config.o \
./src/consola.o \
./src/funcionesConsola.o 

C_DEPS += \
./src/config.d \
./src/consola.d \
./src/funcionesConsola.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2017-1c-The-Big-Bang-Practice/SharedLibrary" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


