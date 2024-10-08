################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/DB_Episode.c \
../src/DB_Test.c \
../src/FSM_R100_AED.c \
../src/R100_Errors_cfg.c \
../src/device_init.c \
../src/hal_entry.c \
../src/init_code.c \
../src/secure_crypto.c \
../src/sysMon_Battery.c \
../src/sysMon_RF_Comms.c \
../src/thread_acc_entry.c \
../src/thread_audio_entry.c \
../src/thread_comm_entry.c \
../src/thread_core_entry.c \
../src/thread_defibrillator_entry.c \
../src/thread_drd_entry.c \
../src/thread_dummy_entry.c \
../src/thread_hmi_entry.c \
../src/thread_patMon_entry.c \
../src/thread_recorder_entry.c \
../src/thread_sysMon_entry.c 

OBJS += \
./src/DB_Episode.o \
./src/DB_Test.o \
./src/FSM_R100_AED.o \
./src/R100_Errors_cfg.o \
./src/device_init.o \
./src/hal_entry.o \
./src/init_code.o \
./src/secure_crypto.o \
./src/sysMon_Battery.o \
./src/sysMon_RF_Comms.o \
./src/thread_acc_entry.o \
./src/thread_audio_entry.o \
./src/thread_comm_entry.o \
./src/thread_core_entry.o \
./src/thread_defibrillator_entry.o \
./src/thread_drd_entry.o \
./src/thread_dummy_entry.o \
./src/thread_hmi_entry.o \
./src/thread_patMon_entry.o \
./src/thread_recorder_entry.o \
./src/thread_sysMon_entry.o 

C_DEPS += \
./src/DB_Episode.d \
./src/DB_Test.d \
./src/FSM_R100_AED.d \
./src/R100_Errors_cfg.d \
./src/device_init.d \
./src/hal_entry.d \
./src/init_code.d \
./src/secure_crypto.d \
./src/sysMon_Battery.d \
./src/sysMon_RF_Comms.d \
./src/thread_acc_entry.d \
./src/thread_audio_entry.d \
./src/thread_comm_entry.d \
./src/thread_core_entry.d \
./src/thread_defibrillator_entry.d \
./src/thread_drd_entry.d \
./src/thread_dummy_entry.d \
./src/thread_hmi_entry.d \
./src/thread_patMon_entry.d \
./src/thread_recorder_entry.d \
./src/thread_sysMon_entry.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -D_RENESAS_SYNERGY_ -DFLT_EVAL_METHOD=3 -DSSP_SUPPRESS_ISR_g_spi1 -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\CUnit" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\bsp" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\Headers" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\driver" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\DRD" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\FRCP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\IPMK" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE\crypto" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE\calib" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE\crypto\hashes" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE\host" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Drivers" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Resources" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\HAL" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\BSP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\synergy_gen" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\ux" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx\tx_src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd_application_layer" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd_application_layer\nxd_tls_secure" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


