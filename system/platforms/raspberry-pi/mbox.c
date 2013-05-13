//Eric Warrington

#include "mbox.h"

static u32 mailbox_empty();
static u32 mailbox_full();

static u32 mailbox_status()
{
    return *(u32*)MAILBOX_ADDR_STATUS;
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


