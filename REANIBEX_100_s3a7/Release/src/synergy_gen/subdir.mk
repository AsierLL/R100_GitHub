################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/synergy_gen/common_data.c \
../src/synergy_gen/hal_data.c \
../src/synergy_gen/main.c \
../src/synergy_gen/pin_data.c \
../src/synergy_gen/thread_acc.c \
../src/synergy_gen/thread_audio.c \
../src/synergy_gen/thread_comm.c \
../src/synergy_gen/thread_core.c \
../src/synergy_gen/thread_defibrillator.c \
../src/synergy_gen/thread_drd.c \
../src/synergy_gen/thread_dummy.c \
../src/synergy_gen/thread_hmi.c \
../src/synergy_gen/thread_patMon.c \
../src/synergy_gen/thread_recorder.c \
../src/synergy_gen/thread_sysMon.c 

OBJS += \
./src/synergy_gen/common_data.o \
./src/synergy_gen/hal_data.o \
./src/synergy_gen/main.o \
./src/synergy_gen/pin_data.o \
./src/synergy_gen/thread_acc.o \
./src/synergy_gen/thread_audio.o \
./src/synergy_gen/thread_comm.o \
./src/synergy_gen/thread_core.o \
./src/synergy_gen/thread_defibrillator.o \
./src/synergy_gen/thread_drd.o \
./src/synergy_gen/thread_dummy.o \
./src/synergy_gen/thread_hmi.o \
./src/synergy_gen/thread_patMon.o \
./src/synergy_gen/thread_recorder.o \
./src/synergy_gen/thread_sysMon.o 

C_DEPS += \
./src/synergy_gen/common_data.d \
./src/synergy_gen/hal_data.d \
./src/synergy_gen/main.d \
./src/synergy_gen/pin_data.d \
./src/synergy_gen/thread_acc.d \
./src/synergy_gen/thread_audio.d \
./src/synergy_gen/thread_comm.d \
./src/synergy_gen/thread_core.d \
./src/synergy_gen/thread_defibrillator.d \
./src/synergy_gen/thread_drd.d \
./src/synergy_gen/thread_dummy.d \
./src/synergy_gen/thread_hmi.d \
./src/synergy_gen/thread_patMon.d \
./src/synergy_gen/thread_recorder.d \
./src/synergy_gen/thread_sysMon.d 


# Each subdirectory must supply rules for building sources it contributes
src/synergy_gen/%.o: ../src/synergy_gen/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_SYNERGY_ -DFLT_EVAL_METHOD=3 -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\bsp" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\driver" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\IPMK" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\FRCP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\DRD" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Drivers" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\HAL" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Resources" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\synergy_gen" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\ux" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx\tx_src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\BSP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd_application_layer" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd_application_layer\nxd_tls_secure" -std=gnu11 -O0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/synergy_gen/main.o: ../src/synergy_gen/main.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_SYNERGY_ -DFLT_EVAL_METHOD=3 -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\bsp" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\driver" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\IPMK" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\FRCP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\DRD" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Drivers" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\HAL" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Resources" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\synergy_gen" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\ux" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx\tx_src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\BSP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd_application_layer" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd_application_layer\nxd_tls_secure" -std=gnu11 -O0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/synergy_gen/main.d" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


