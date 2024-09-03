################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../synergy/ssp/src/bsp/mcu/s3a7/bsp_cache.c \
../synergy/ssp/src/bsp/mcu/s3a7/bsp_clocks.c \
../synergy/ssp/src/bsp/mcu/s3a7/bsp_feature.c \
../synergy/ssp/src/bsp/mcu/s3a7/bsp_fmi_R7FS3A77C3A01CFB.c \
../synergy/ssp/src/bsp/mcu/s3a7/bsp_fmi_R7FS3A77C3A01CFP.c \
../synergy/ssp/src/bsp/mcu/s3a7/bsp_group_irq.c \
../synergy/ssp/src/bsp/mcu/s3a7/bsp_hw_locks.c \
../synergy/ssp/src/bsp/mcu/s3a7/bsp_module_stop.c \
../synergy/ssp/src/bsp/mcu/s3a7/bsp_rom_registers.c 

OBJS += \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_cache.o \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_clocks.o \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_feature.o \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_fmi_R7FS3A77C3A01CFB.o \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_fmi_R7FS3A77C3A01CFP.o \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_group_irq.o \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_hw_locks.o \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_module_stop.o \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_rom_registers.o 

C_DEPS += \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_cache.d \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_clocks.d \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_feature.d \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_fmi_R7FS3A77C3A01CFB.d \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_fmi_R7FS3A77C3A01CFP.d \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_group_irq.d \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_hw_locks.d \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_module_stop.d \
./synergy/ssp/src/bsp/mcu/s3a7/bsp_rom_registers.d 


# Each subdirectory must supply rules for building sources it contributes
synergy/ssp/src/bsp/mcu/s3a7/%.o: ../synergy/ssp/src/bsp/mcu/s3a7/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -D_RENESAS_SYNERGY_ -DFLT_EVAL_METHOD=3 -DSSP_SUPPRESS_ISR_g_spi1 -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\CUnit" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\bsp" -I"C:\Users\asier\e2_studio\workspace2\CUnit_v3_Miren\src\Headers" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\driver" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\driver\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\DRD" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\FRCP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\IPMK" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE\crypto" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE\calib" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE\crypto\hashes" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\SCE\host" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Drivers" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\Resources" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\HAL" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\BSP" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\src\synergy_gen" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy_cfg\ssp_cfg\framework" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\api" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\instances" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\ux" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\tx\tx_src" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\inc\framework\el\nxd_application_layer" -I"C:\Users\asier\OneDrive\Desktop\CUnit - Osatu\Apuntes\API\repo-github\R100_GitHub\REANIBEX_100_s3a7\synergy\ssp\src\framework\el\nxd_application_layer\nxd_tls_secure" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


