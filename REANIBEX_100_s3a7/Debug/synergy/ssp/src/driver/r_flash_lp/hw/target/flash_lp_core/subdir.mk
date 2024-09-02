################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_codeflash.c \
../synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_codeflash_extra.c \
../synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_dataflash.c \
../synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_flash_common.c 

OBJS += \
./synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_codeflash.o \
./synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_codeflash_extra.o \
./synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_dataflash.o \
./synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_flash_common.o 

C_DEPS += \
./synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_codeflash.d \
./synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_codeflash_extra.d \
./synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_dataflash.d \
./synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/hw_flash_common.d 


# Each subdirectory must supply rules for building sources it contributes
synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/%.o: ../synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -D_RENESAS_SYNERGY_ -DFLT_EVAL_METHOD=3 -DSSP_SUPPRESS_ISR_g_spi1 -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\CUnit" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy_cfg\ssp_cfg\bsp" -I"C:\Users\asier\e2_studio\workspace2\CUnit_v3_Miren\src\Headers" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy_cfg\ssp_cfg\driver" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc\bsp" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc\driver\api" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc\driver\instances" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\DRD" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\FRCP" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\IPMK" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\SCE" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\SCE\crypto" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\SCE\calib" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\SCE\crypto\hashes" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\SCE\host" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\Drivers" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\Resources" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\HAL" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\BSP" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\src\synergy_gen" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy_cfg\ssp_cfg\framework\el" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\el" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\tx" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy_cfg\ssp_cfg\framework" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\api" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\instances" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\ux" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\tx\tx_src" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\nxd" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\el\nxd_application_layer" -I"C:\Users\asier\OneDrive\Desktop\VERSIONES R100\R100_Ultima_Version\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\nxd_application_layer\nxd_tls_secure" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


