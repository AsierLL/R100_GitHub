################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/HAL/R100_Tables.c \
../src/HAL/thread_audio_hal.c \
../src/HAL/thread_comm_hal.c \
../src/HAL/thread_core_hal.c \
../src/HAL/thread_defibrillator_hal.c \
../src/HAL/thread_drd_hal.c \
../src/HAL/thread_hmi_hal.c \
../src/HAL/thread_patMon_hal.c \
../src/HAL/thread_recorder_hal.c \
../src/HAL/thread_sysMon_hal.c 

OBJS += \
./src/HAL/R100_Tables.o \
./src/HAL/thread_audio_hal.o \
./src/HAL/thread_comm_hal.o \
./src/HAL/thread_core_hal.o \
./src/HAL/thread_defibrillator_hal.o \
./src/HAL/thread_drd_hal.o \
./src/HAL/thread_hmi_hal.o \
./src/HAL/thread_patMon_hal.o \
./src/HAL/thread_recorder_hal.o \
./src/HAL/thread_sysMon_hal.o 

C_DEPS += \
./src/HAL/R100_Tables.d \
./src/HAL/thread_audio_hal.d \
./src/HAL/thread_comm_hal.d \
./src/HAL/thread_core_hal.d \
./src/HAL/thread_defibrillator_hal.d \
./src/HAL/thread_drd_hal.d \
./src/HAL/thread_hmi_hal.d \
./src/HAL/thread_patMon_hal.d \
./src/HAL/thread_recorder_hal.d \
./src/HAL/thread_sysMon_hal.d 


# Each subdirectory must supply rules for building sources it contributes
src/HAL/%.o: ../src/HAL/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_SYNERGY_ -DFLT_EVAL_METHOD=3 -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\bsp" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\driver" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\IPMK" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\FRCP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\DRD" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Drivers" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\HAL" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Resources" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\synergy_gen" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\ux" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx\tx_src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\BSP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd_application_layer" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd_application_layer\nxd_tls_secure" -std=gnu11 -O0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


