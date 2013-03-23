#include "mbox.h"

uint32_t arm_to_vcbase_addr(void* arm_addr) {
    // TODO - some how detect if L2 is enabled?
    return ((uint32_t)arm_addr) + BCM2835_VC_MEM_BASE_L2UNCACHED;
}

void* vcbase_to_arm_addr(uint32_t vcbase_addr) {
    // TODO - some how detect if L2 is enabled?
    return (void*)(vcbase_addr - BCM2835_VC_MEM_BASE_L2UNCACHED);
}

uint32_t mbox_read_reg(uint32_t reg) {
    return *(uint32_t*)(BCM2835_ARM_MAPPED_REG_BASE + reg);
}

uint32_t mbox_write_reg(uint32_t reg, uint32_t value) {
    *(uint32_t*)(BCM2835_ARM_MAPPED_REG_BASE + reg) = value;
}

int mbox_read(uint8_t chan, uint32_t* value) {
    return -1;
}

int mbox_write(uint8_t chan, uint32_t value) {
    return -1;
}

