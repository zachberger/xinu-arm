/**
 * mbox.h
 *
 * Author: Jedd Haberstro
 * Date: March 23, 2013
 */

#include <stdint.h>

#define BCM2835_ARM_MAPPED_REG_BASE             0x20000000
#define BCM2835_VC_MAPPED_REG_BASE              0x7E000000
#define BCM2835_VC_MEM_BASE_L2CACHED            0x40000000
#define BCM2835_VC_MEM_BASE_L2UNCACHED          0xC0000000

/*
 * Mailbox 0
 */
#define BCM2835_MBOX0_BASE                      0xB880

#define BCM2835_MBOX0_REG_PEEK                  (BCM2835_MBOX0_BASE + 0x10)
#define BCM2835_MBOX0_REG_READ                  (BCM2835_MBOX0_BASE + 0x00)
#define BCM2835_MBOX0_REG_WRITE                 (BCM2835_MBOX0_BASE + 0x20)
#define BCM2835_MBOX0_REG_STATUS                (BCM2835_MBOX0_BASE + 0x18)
#define BCM2835_MBOX0_REG_SENDER                (BCM2835_MBOX0_BASE + 0x14)
#define BCM2835_MBOX0_REG_CONFIG                (BCM2835_MBOX0_BASE + 0x1C)

#define BCM2835_MBOX0_CHAN_POWERMANAGEMENT      0
#define BCM2835_MBOX0_CHAN_FRAMEBUFER           1 
#define BCM2835_MBOX0_CHAN_VIRTUALUART          2 
#define BCM2835_MBOX0_CHAN_VCHIQ                3 
#define BCM2835_MBOX0_CHAN_LEDS                 4 
#define BCM2835_MBOX0_CHAN_BUTTONS              5 
#define BCM2835_MBOX0_CHAN_TOUCHSCREEN          6 
#define BCM2835_MBOX0_CHAN_ARMTOVC_PROP         8 
#define BCM2835_MBOX0_CHAN_VCTOARM_PROP         9 

#define BCM2835_MBOX0_STATUS_FULL               0x80000000
#define BCM2835_MBOX0_STATUS_EMPTY              0x40000000

/*
 * Convert ARM physical addresses to VC addresses.
 * Must be used when sending addresses with the mbox inteface.
 */
uint32_t arm_to_vcbase_addr(void* arm_addr);

/*
 * Convert VC addresses to ARM physical addresses.
 * Must be used when receiving an address from the VC via the mbox interface.
 */
void* vcbase_to_arm_addr(uint32_t vcbase_addr);

/*
 * Read a mailbox reg
 */
uint32_t mbox_read_reg(uint32_t reg);

/*
 * Write a mailbox reg
 */
uint32_t mbox_write_reg(uint32_t reg, uint32_t value);

/*
 * Read a specific channel of mailbox 0's 'read' register.
 *
 * Returns 0 on success, non-0 on failure.
 */
int mbox_read(uint8_t chan, uint32_t* value);

/*
 * Write to a specific channel of mailbox 0's 'write' register.
 *
 * Returns 0 on success, non-0 on failure.
 */
int mbox_write(uint8_t chan, uint32_t value);

/*
 * TODO: Added specific functions for getting framebuffer info, sending and receiving property tags
 */

