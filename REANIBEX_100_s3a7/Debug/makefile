################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

-include makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include synergy/ssp/src/framework/sf_spi/subdir.mk
-include synergy/ssp/src/framework/sf_el_ux/subdir.mk
-include synergy/ssp/src/framework/sf_el_fx/subdir.mk
-include synergy/ssp/src/framework/sf_block_media_sdmmc/subdir.mk
-include synergy/ssp/src/framework/el/nxd_application_layer/nxd_tls_secure/subdir.mk
-include synergy/ssp/src/framework/el/nxd_application_layer/nxd_crypto_generic/subdir.mk
-include synergy/ssp/src/driver/r_sdmmc/subdir.mk
-include synergy/ssp/src/driver/r_sci_uart/subdir.mk
-include synergy/ssp/src/driver/r_rtc/subdir.mk
-include synergy/ssp/src/driver/r_rspi/subdir.mk
-include synergy/ssp/src/driver/r_iwdt/subdir.mk
-include synergy/ssp/src/driver/r_ioport/subdir.mk
-include synergy/ssp/src/driver/r_icu/subdir.mk
-include synergy/ssp/src/driver/r_gpt/subdir.mk
-include synergy/ssp/src/driver/r_fmi/subdir.mk
-include synergy/ssp/src/driver/r_flash_lp/hw/target/flash_lp_core/subdir.mk
-include synergy/ssp/src/driver/r_flash_lp/hw/target/subdir.mk
-include synergy/ssp/src/driver/r_flash_lp/subdir.mk
-include synergy/ssp/src/driver/r_elc/subdir.mk
-include synergy/ssp/src/driver/r_dtc/subdir.mk
-include synergy/ssp/src/driver/r_dmac/subdir.mk
-include synergy/ssp/src/driver/r_dac/subdir.mk
-include synergy/ssp/src/driver/r_cgc/hw/subdir.mk
-include synergy/ssp/src/driver/r_cgc/subdir.mk
-include synergy/ssp/src/driver/r_adc/subdir.mk
-include synergy/ssp/src/bsp/mcu/s3a7/subdir.mk
-include synergy/ssp/src/bsp/mcu/all/subdir.mk
-include synergy/ssp/src/bsp/cmsis/Device/RENESAS/S3A7/Source/subdir.mk
-include synergy/board/s3a7_R100/subdir.mk
-include src/synergy_gen/subdir.mk
-include src/Resources/subdir.mk
-include src/IPMK/subdir.mk
-include src/HAL/subdir.mk
-include src/FRCP/subdir.mk
-include src/Drivers/subdir.mk
-include src/CUnit/subdir.mk
-include src/BSP/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(SX_DEPS)),)
-include $(SX_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
REANIBEX_100_s3a7.bin \

SECONDARY_SIZE += \
REANIBEX_100_s3a7.siz \


# All Target
all: REANIBEX_100_s3a7.elf secondary-outputs

# Tool invocations
REANIBEX_100_s3a7.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C Linker'
	$(file > REANIBEX_100_s3a7.elf.in,-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g3 -T "C:\\Users\\asier\\OneDrive\\Desktop\\CUnit - Osatu\\Apuntes\\API\\repo-github\\R100_GitHub\\REANIBEX_100_s3a7\\script/s3a7.ld" -Xlinker --gc-sections -L "C:\\Users\\asier\\OneDrive\\Desktop\\CUnit - Osatu\\Apuntes\\API\\repo-github\\R100_GitHub\\REANIBEX_100_s3a7\\synergy\\ssp\\src\\bsp\\cmsis\\DSP_Lib\\cm4_gcc" -L "C:\\Users\\asier\\OneDrive\\Desktop\\CUnit - Osatu\\Apuntes\\API\\repo-github\\R100_GitHub\\REANIBEX_100_s3a7\\src\\SCE" -L "C:\\Users\\asier\\OneDrive\\Desktop\\CUnit - Osatu\\Apuntes\\API\\repo-github\\R100_GitHub\\REANIBEX_100_s3a7\\synergy\\ssp\\src\\framework\\el\\tx\\cm4_gcc" -L "C:\\Users\\asier\\OneDrive\\Desktop\\CUnit - Osatu\\Apuntes\\API\\repo-github\\R100_GitHub\\REANIBEX_100_s3a7\\synergy\\ssp\\src\\framework\\el\\fx\\cm4_gcc" -L "C:\\Users\\asier\\OneDrive\\Desktop\\CUnit - Osatu\\Apuntes\\API\\repo-github\\R100_GitHub\\REANIBEX_100_s3a7\\src\\DRD" -L "C:\\Users\\asier\\OneDrive\\Desktop\\CUnit - Osatu\\Apuntes\\API\\repo-github\\R100_GitHub\\REANIBEX_100_s3a7\\synergy\\ssp\\src\\framework\\el\\ux\\cm4_gcc" -L "C:\\Users\\asier\\OneDrive\\Desktop\\CUnit - Osatu\\Apuntes\\API\\repo-github\\R100_GitHub\\REANIBEX_100_s3a7\\synergy\\ssp\\src\\framework\\el\\ux_device_class_storage\\cm4_gcc" -L "C:\\Users\\asier\\OneDrive\\Desktop\\CUnit - Osatu\\Apuntes\\API\\repo-github\\R100_GitHub\\REANIBEX_100_s3a7\\synergy\\ssp\\src\\framework\\el\\nxd\\cm4_gcc" -Wl,-Map,"REANIBEX_100_s3a7.map" --specs=nano.specs --specs=rdimon.specs -o "REANIBEX_100_s3a7.elf" -Wl,--start-group $(OBJS) $(USER_OBJS) $(LIBS) -Wl,--end-group)
	arm-none-eabi-gcc @"REANIBEX_100_s3a7.elf.in"
	@echo 'Finished building target: $@'
	@echo ' '

REANIBEX_100_s3a7.bin: REANIBEX_100_s3a7.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O binary "REANIBEX_100_s3a7.elf"  "REANIBEX_100_s3a7.bin"
	@echo 'Finished building: $@'
	@echo ' '

REANIBEX_100_s3a7.siz: REANIBEX_100_s3a7.elf
	@echo 'Invoking: GNU ARM Cross Print Size'
	arm-none-eabi-size --format=berkeley "REANIBEX_100_s3a7.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
ifneq ($(strip $(SX_DEPS)),)
	-$(file > clean_SX_DEPS.tmp, $(SX_DEPS))
	-@ xargs -s 6000 -t -a clean_SX_DEPS.tmp $(RM)
	-@$(RM) clean_SX_DEPS.tmp
endif
ifneq ($(strip $(OBJS)),)
	-$(file > clean_OBJS.tmp, $(OBJS))
	-@ xargs -s 6000 -t -a clean_OBJS.tmp $(RM)
	-@$(RM) clean_OBJS.tmp
endif
ifneq ($(strip $(SECONDARY_FLASH)),)
	-$(file > clean_SECONDARY_FLASH.tmp, $(SECONDARY_FLASH))
	-@ xargs -s 6000 -t -a clean_SECONDARY_FLASH.tmp $(RM)
	-@$(RM) clean_SECONDARY_FLASH.tmp
endif
ifneq ($(strip $(SECONDARY_SIZE)),)
	-$(file > clean_SECONDARY_SIZE.tmp, $(SECONDARY_SIZE))
	-@ xargs -s 6000 -t -a clean_SECONDARY_SIZE.tmp $(RM)
	-@$(RM) clean_SECONDARY_SIZE.tmp
endif
ifneq ($(strip $(ASM_DEPS)),)
	-$(file > clean_ASM_DEPS.tmp, $(ASM_DEPS))
	-@ xargs -s 6000 -t -a clean_ASM_DEPS.tmp $(RM)
	-@$(RM) clean_ASM_DEPS.tmp
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
	-$(file > clean_S_UPPER_DEPS.tmp, $(S_UPPER_DEPS))
	-@ xargs -s 6000 -t -a clean_S_UPPER_DEPS.tmp $(RM)
	-@$(RM) clean_S_UPPER_DEPS.tmp
endif
ifneq ($(strip $(C_DEPS)),)
	-$(file > clean_C_DEPS.tmp, $(C_DEPS))
	-@ xargs -s 6000 -t -a clean_C_DEPS.tmp $(RM)
	-@$(RM) clean_C_DEPS.tmp
endif
	-$(RM)  REANIBEX_100_s3a7.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents

-include ../makefile.targets
