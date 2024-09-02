################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../synergy/ssp/src/driver/r_gpt/r_gpt.c 

OBJS += \
./synergy/ssp/src/driver/r_gpt/r_gpt.o 

C_DEPS += \
./synergy/ssp/src/driver/r_gpt/r_gpt.d 


# Each subdirectory must supply rules for building sources it contributes
synergy/ssp/src/driver/r_gpt/%.o: ../synergy/ssp/src/driver/r_gpt/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	C:\Renesas\e2_studio\Utilities\\/isdebuild arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_SYNERGY_ -DFLT_EVAL_METHOD=3 -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy_cfg\ssp_cfg\bsp" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy_cfg\ssp_cfg\driver" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc\bsp" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc\bsp\cmsis\Include" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc\driver\api" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc\driver\instances" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src\SCE" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src\IPMK" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src\FRCP" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src\DRD" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src\Drivers" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src\HAL" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src\Resources" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src\synergy_gen" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy_cfg\ssp_cfg\framework\el" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\el" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\tx" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy_cfg\ssp_cfg\framework" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\api" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\instances" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\ux" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\tx\tx_src" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\src\BSP" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\el\nxd" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\nxd" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\inc\framework\el\nxd_application_layer" -I"C:\Users\ivicente\Desktop\BRANCH_ACC_WIFI\synergy\ssp\src\framework\el\nxd_application_layer\nxd_tls_secure" -std=gnu11 -O0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" -x c "$<"
	@echo 'Finished building: $<'
	@echo ' '


