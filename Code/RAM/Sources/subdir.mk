################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../Sources/Exceptions.c" \
"../Sources/IntcInterrupts.c" \
"../Sources/camera.c" \
"../Sources/control.c" \
"../Sources/drive.c" \
"../Sources/ivor_branch_table.c" \
"../Sources/main.c" \
"../Sources/serial.c" \
"../Sources/utilities.c" \

C_SRCS += \
../Sources/Exceptions.c \
../Sources/IntcInterrupts.c \
../Sources/camera.c \
../Sources/control.c \
../Sources/drive.c \
../Sources/ivor_branch_table.c \
../Sources/main.c \
../Sources/serial.c \
../Sources/utilities.c \

OBJS += \
./Sources/Exceptions_c.obj \
./Sources/IntcInterrupts_c.obj \
./Sources/camera_c.obj \
./Sources/control_c.obj \
./Sources/drive_c.obj \
./Sources/ivor_branch_table_c.obj \
./Sources/main_c.obj \
./Sources/serial_c.obj \
./Sources/utilities_c.obj \

OBJS_QUOTED += \
"./Sources/Exceptions_c.obj" \
"./Sources/IntcInterrupts_c.obj" \
"./Sources/camera_c.obj" \
"./Sources/control_c.obj" \
"./Sources/drive_c.obj" \
"./Sources/ivor_branch_table_c.obj" \
"./Sources/main_c.obj" \
"./Sources/serial_c.obj" \
"./Sources/utilities_c.obj" \

C_DEPS += \
./Sources/Exceptions_c.d \
./Sources/IntcInterrupts_c.d \
./Sources/camera_c.d \
./Sources/control_c.d \
./Sources/drive_c.d \
./Sources/ivor_branch_table_c.d \
./Sources/main_c.d \
./Sources/serial_c.d \
./Sources/utilities_c.d \

OBJS_OS_FORMAT += \
./Sources/Exceptions_c.obj \
./Sources/IntcInterrupts_c.obj \
./Sources/camera_c.obj \
./Sources/control_c.obj \
./Sources/drive_c.obj \
./Sources/ivor_branch_table_c.obj \
./Sources/main_c.obj \
./Sources/serial_c.obj \
./Sources/utilities_c.obj \

C_DEPS_QUOTED += \
"./Sources/Exceptions_c.d" \
"./Sources/IntcInterrupts_c.d" \
"./Sources/camera_c.d" \
"./Sources/control_c.d" \
"./Sources/drive_c.d" \
"./Sources/ivor_branch_table_c.d" \
"./Sources/main_c.d" \
"./Sources/serial_c.d" \
"./Sources/utilities_c.d" \


# Each subdirectory must supply rules for building sources it contributes
Sources/Exceptions_c.obj: ../Sources/Exceptions.c
	@echo 'Building file: $<'
	@echo 'Executing target #1 $<'
	@echo 'Invoking: PowerPC Compiler'
	"$(PAToolsDirEnv)/mwcceppc" @@"Sources/Exceptions.args" -o "Sources/Exceptions_c.obj" "$<" -MD -gccdep
	@echo 'Finished building: $<'
	@echo ' '

Sources/%.d: ../Sources/%.c
	@echo 'Regenerating dependency file: $@'
	
	@echo ' '

Sources/IntcInterrupts_c.obj: ../Sources/IntcInterrupts.c
	@echo 'Building file: $<'
	@echo 'Executing target #2 $<'
	@echo 'Invoking: PowerPC Compiler'
	"$(PAToolsDirEnv)/mwcceppc" @@"Sources/IntcInterrupts.args" -o "Sources/IntcInterrupts_c.obj" "$<" -MD -gccdep
	@echo 'Finished building: $<'
	@echo ' '

Sources/camera_c.obj: ../Sources/camera.c
	@echo 'Building file: $<'
	@echo 'Executing target #3 $<'
	@echo 'Invoking: PowerPC Compiler'
	"$(PAToolsDirEnv)/mwcceppc" @@"Sources/camera.args" -o "Sources/camera_c.obj" "$<" -MD -gccdep
	@echo 'Finished building: $<'
	@echo ' '

Sources/control_c.obj: ../Sources/control.c
	@echo 'Building file: $<'
	@echo 'Executing target #4 $<'
	@echo 'Invoking: PowerPC Compiler'
	"$(PAToolsDirEnv)/mwcceppc" @@"Sources/control.args" -o "Sources/control_c.obj" "$<" -MD -gccdep
	@echo 'Finished building: $<'
	@echo ' '

Sources/drive_c.obj: ../Sources/drive.c
	@echo 'Building file: $<'
	@echo 'Executing target #5 $<'
	@echo 'Invoking: PowerPC Compiler'
	"$(PAToolsDirEnv)/mwcceppc" @@"Sources/drive.args" -o "Sources/drive_c.obj" "$<" -MD -gccdep
	@echo 'Finished building: $<'
	@echo ' '

Sources/ivor_branch_table_c.obj: ../Sources/ivor_branch_table.c
	@echo 'Building file: $<'
	@echo 'Executing target #6 $<'
	@echo 'Invoking: PowerPC Compiler'
	"$(PAToolsDirEnv)/mwcceppc" @@"Sources/ivor_branch_table.args" -o "Sources/ivor_branch_table_c.obj" "$<" -MD -gccdep
	@echo 'Finished building: $<'
	@echo ' '

Sources/main_c.obj: ../Sources/main.c
	@echo 'Building file: $<'
	@echo 'Executing target #7 $<'
	@echo 'Invoking: PowerPC Compiler'
	"$(PAToolsDirEnv)/mwcceppc" @@"Sources/main.args" -o "Sources/main_c.obj" "$<" -MD -gccdep
	@echo 'Finished building: $<'
	@echo ' '

Sources/serial_c.obj: ../Sources/serial.c
	@echo 'Building file: $<'
	@echo 'Executing target #8 $<'
	@echo 'Invoking: PowerPC Compiler'
	"$(PAToolsDirEnv)/mwcceppc" @@"Sources/serial.args" -o "Sources/serial_c.obj" "$<" -MD -gccdep
	@echo 'Finished building: $<'
	@echo ' '

Sources/utilities_c.obj: ../Sources/utilities.c
	@echo 'Building file: $<'
	@echo 'Executing target #9 $<'
	@echo 'Invoking: PowerPC Compiler'
	"$(PAToolsDirEnv)/mwcceppc" @@"Sources/utilities.args" -o "Sources/utilities_c.obj" "$<" -MD -gccdep
	@echo 'Finished building: $<'
	@echo ' '


