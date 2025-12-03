################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/Documents/LIN_projekt/wspc/lin_responder_LP_MSPM0C1104_nortos_ticlang" -I"D:/Documents/LIN_projekt/wspc/lin_responder_LP_MSPM0C1104_nortos_ticlang/Debug" -I"C:/TI/mspm0_sdk_2_08_00_03/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_08_00_03/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-584727409: ../lin_responder.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"D:/ti/ccs2031/ccs/utils/sysconfig_1.25.0/sysconfig_cli.bat" --script "D:/Documents/LIN_projekt/wspc/lin_responder_LP_MSPM0C1104_nortos_ticlang/lin_responder.syscfg" -o "." -s "C:/TI/mspm0_sdk_2_08_00_03/.metadata/product.json" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-584727409 ../lin_responder.syscfg
device.opt: build-584727409
device.cmd.genlibs: build-584727409
ti_msp_dl_config.c: build-584727409
ti_msp_dl_config.h: build-584727409
Event.dot: build-584727409

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/Documents/LIN_projekt/wspc/lin_responder_LP_MSPM0C1104_nortos_ticlang" -I"D:/Documents/LIN_projekt/wspc/lin_responder_LP_MSPM0C1104_nortos_ticlang/Debug" -I"C:/TI/mspm0_sdk_2_08_00_03/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_08_00_03/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0c110x_ticlang.o: C:/TI/mspm0_sdk_2_08_00_03/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0c110x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2031/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"D:/Documents/LIN_projekt/wspc/lin_responder_LP_MSPM0C1104_nortos_ticlang" -I"D:/Documents/LIN_projekt/wspc/lin_responder_LP_MSPM0C1104_nortos_ticlang/Debug" -I"C:/TI/mspm0_sdk_2_08_00_03/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_08_00_03/source" -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


