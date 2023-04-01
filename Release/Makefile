
DEVICE						= STM32_L011D4Px
TARGET_NAME                 = boot_$(DEVICE)

ROOT_DIR = ../..
CURRENT_DIR = .
BUILD_DIR                   = $(CURRENT_DIR)/build

INCLUDE_FOLDERS             = $(CURRENT_DIR)/CMSIS/Core/Include
INCLUDE_FOLDERS             +=$(CURRENT_DIR)/CMSIS/Device/

INCLUDES                    = $(INCLUDE_FOLDERS:%=-I%)

CROSS_PREFIX                = arm-none-eabi-
GCC                         = $(CROSS_PREFIX)gcc
AS                          = $(CROSS_PREFIX)as
SIZE                        = $(CROSS_PREFIX)size
OBJCOPY                     = $(CROSS_PREFIX)objcopy
OBJDUMP                     = $(CROSS_PREFIX)objdump
NM                          = $(CROSS_PREFIX)nm

SRC_C                       = init.c
SRC_C                       += main.c

DEFINES                     = -DSTM32L011xx

OBJ                         = $(SRC_AS:%.S=%.o) $(SRC_C:%.c=%.o)

# optimization flags
OPTIMIZATION_FLAG           = s

MCU_FLAGS                   = -mcpu=cortex-m0 -mthumb -D__CORTEX_M0 -std=c11

FLAGS_CC                    = $(INCLUDES) $(DEFINES) -I. -O$(OPTIMIZATION_FLAG) $(MCU_FLAGS) -g -nodefaultlibs -nostdlib

FLAGS_LD                    = -Wl,--gc-sections -nostartfiles -nodefaultlibs -nostdlib

all: $(TARGET_NAME).elf

%.elf: $(OBJ)
	@echo LD: $(OBJ) $(OBJ_SPEED)	
	@$(GCC) $(FLAGS_LD) -T linker.ld -o $(BUILD_DIR)/$@ $(OBJ:%.o=$(BUILD_DIR)/%.o)
	@echo '-----------------------------------------------------------'
	@$(SIZE) $(BUILD_DIR)/$(TARGET_NAME).elf 
	@$(OBJCOPY) -O binary $(BUILD_DIR)/$(TARGET_NAME).elf   $(BUILD_DIR)/$(TARGET_NAME).bin
	@$(OBJCOPY) -O ihex   $(BUILD_DIR)/$(TARGET_NAME).elf   $(BUILD_DIR)/$(TARGET_NAME).hex
	@$(OBJDUMP) -h -S -z  $(BUILD_DIR)/$(TARGET_NAME).elf > $(BUILD_DIR)/$(TARGET_NAME).lss
	@$(NM) -n 			  $(BUILD_DIR)/$(TARGET_NAME).elf > $(BUILD_DIR)/$(TARGET_NAME).sym
.c.o:
	@-mkdir -p $(BUILD_DIR)
	@echo CC: $<
	@$(GCC) $(FLAGS_CC) -c ./$< -o $(BUILD_DIR)/$@

.S.o:
	@-mkdir -p $(BUILD_DIR)
	@echo AS_C: $<
	@$(GCC) $(INCLUDES) -I. $(DEFINES) -c -x assembler-with-cpp ./$<       \
	    -o $(BUILD_DIR)/$@