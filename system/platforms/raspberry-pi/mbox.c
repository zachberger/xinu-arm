//Eric Warrington

#include "mbox.h"

uint32_t arm_to_vcbase_addr(void* arm_addr)
{
	// TODO - somehow detect if L2 is enabled?
	return ((uint32_t)arm_addr) + BCM2835_VC_MEM_BASE_L2UNCACHED;
}

void* vcbase_to_arm_addr(uint32_t vcbase_addr)
{
	// TODO - somehow detect if L2 is enabled?
	return (void*)(vcbase_addr - BCM2835_VC_MEM_BASE_L2UNCACHED);
}


static u32 mailbox_empty();
static u32 mailbox_full();

static u32 mailbox_status()
{
    return *(u32*)MAILBOX_ADDR_STATUS;
}




uint32_t mbox_read_reg(uint32_t reg)
{
	return *(uint32_t*)(BCM2835_ARM_MAPPED_REG_BASE + reg);
}

uint32_t mbox_write_reg(uint32_t reg, uint32_t value)
{
	*(uint32_t*)(BCM2835_ARM_MAPPED_REG_BASE + reg) = value;
}


int mbox_read(uint8_t chan, uint32_t* value)
{
	return -1;
}

//return read value; 0 on error
u32 mailbox_read(u8 chan)
{
    u32 val;

    // channel: 4 bits
    if(chan>15)
    {
        return 0;
    }

    while(1)
    {
        while(mailbox_empty()) {}
        
        val = *(u32*)MAILBOX_ADDR_READ;
        if((val & 0b1111) == chan)
        {
            return val & ~0b1111;
        }
    }
}


int mbox_write(uint8_t chan, uint32_t value)
{
	return -1;
}

// return status; 0 on error
int mailbox_write(u8 chan, u32 val)
{
    //channel: 4 bits
	//val: lowest 4 bits must be 0
	
    if(chan > 15 || val & 0b1111)
    {
        return 0;
    }

    while(mailbox_full()) {}

    *(u32*)MAILBOX_ADDR_WRITE=val | chan;

    return 1;
}


/* return 1 if mailbox is empty */
static u32 mailbox_empty()
{
    return mailbox_status() & MAILBOX_EMPTY;
}

/* return 1 if mailbox is full */
static u32 mailbox_full()
{
    return mailbox_status() & MAILBOX_FULL;
}

void mailbox_error()
{
    //while(1)
    {
        printf("Mailbox Error\n");
        //led_flash_num(num, FB_ERROR_MAX_BITS);
    }
}
