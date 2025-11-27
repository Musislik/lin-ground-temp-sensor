################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/Documents/LIN_projekt/lin_commander_mod_upload_test" -I"D:/Documents/LIN_projekt/lin_commander_mod_upload_test/Debug" -I"D:/ti/ccs2031/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/ccs2031/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1761050207: ../lin_commander.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"D:/ti/ccs2031/ccs/utils/sysconfig_1.25.0/sysconfig_cli.bat" --script "D:/Documents/LIN_projekt/lin_commander_mod_upload_test/lin_commander.syscfg" -o "." -s "D:/ti/ccs2031/mspm0_sdk_2_07_00_05/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-1761050207 ../lin_commander.syscfg
device.opt: build-1761050207
device.cmd.genlibs: build-1761050207
ti_msp_dl_config.c: build-1761050207
ti_msp_dl_config.h: build-1761050207
Event.dot: build-1761050207

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/Documents/LIN_projekt/lin_commander_mod_upload_test" -I"D:/Documents/LIN_projekt/lin_commander_mod_upload_test/Debug" -I"D:/ti/ccs2031/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/ccs2031/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0c110x_ticlang.o: D:/ti/ccs2031/mspm0_sdk_2_07_00_05/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0c110x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/Documents/LIN_projekt/lin_commander_mod_upload_test" -I"D:/Documents/LIN_projekt/lin_commander_mod_upload_test/Debug" -I"D:/ti/ccs2031/mspm0_sdk_2_07_00_05/source/third_party/CMSIS/Core/Include" -I"D:/ti/ccs2031/mspm0_sdk_2_07_00_05/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


