#include "mbox.h"
#include "properties.h"

/* use a global variable since we have no malloc *///static struct mem_reigon MR;

mem_region get_arm_memory()
{
	mem_region MR;
	
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	//u32 addr, size;	buf[0] = 8 * 4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get ARM memory*/	buf[2] = 0x00010005; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = 0; /* req: mem address */	buf[6] = 0; /* req: mem size */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		fb_error();		return;	}	MR.base_address=buf[5];	MR.block_size=buf[6];
	
	return MR;
}

mem_region get_vc_memory()
{
	mem_region MR;
	
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	//u32 addr, size;	buf[0] = 8 * 4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get VC memory*/	buf[2] = 0x00010006; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = 0; /* req: mem address */	buf[6] = 0; /* req: mem size */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		fb_error();		return;	}	MR.base_address=buf[5];	MR.block_size=buf[6];
}


//mailbox properties of length 7
u32 mb_prop7(u32 tag_id, u32 p1, u32 p2)
{
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	buf[0] = 7*4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get temperature*/	buf[2] = tag_id; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = p1; /* req: 1 */	buf[6] = p2; /* req: 2 */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		fb_error();		return;	}	return buf[6];
}




u32 get_temperature()
{	/* tag - get temperature*/
	u32 tag=0x00030006;	return mb_prop7(tag, 0, 0);
}

u32 get_max_temperature()
{	/* tag - get max temperature*/	u32 tag=0x0003000A;
	return mb_prop7(tag, 0, 0);
}


//(offset from 1.2V in units of 0.025V)
u32 get_voltage(u32 device_id)
{	/* tag - get voltage*/	u32 tag=0x00030003;
	return mb_prop7(tag, device_id, 0);
}

u32 set_voltage(u32 device_id, u32 voltage)
{	/* tag - set voltage*/	u32 tag=0x00038003;	return mb_prop7(tag, device_id, voltage); 
}

//Return the maximum supported voltage rate for the given id. Voltages should not be set higher than this. 
u32 get_max_voltage(u32 device_id)
{	/* tag - get max voltage*/	u32 tag=0x00030005;
	return mb_prop7(tag, device_id, 0);
}

//Return the minimum supported voltage rate for the given id. This may be used when idle. 
u32 get_min_voltage(u32 device_id)
{
	/* tag - get max voltage*/	u32 tag=0x00030008;
	return mb_prop7(tag, device_id, 0);
}


/*
  State:
    Bit 0: 0=off, 1=on
    Bit 1: 0=device exists, 1=device does not exist
    Bits 2-31: reserved for future use
*/

u32 get_device_state(u32 device_id)
{
	/* tag - get device state*/	u32 tag=0x00020001;
	return mb_prop7(tag, device_id, 0);
}

u32 get_device_timing(u32 device_id)
{
	/* tag - get device timing*/	u32 tag=0x00020002;
	return mb_prop7(tag, device_id, 0);
}

u32 set_device_state(u32 device_id, u32 state)
{
	/* tag - set device state*/	u32 tag=0x00028001;
	return mb_prop7(tag, device_id, state);
}

u32 get_turbo();
u32 set_turbo(u32 turbo);




u32 get_clock_state(u32 clock_id)
{
	/* tag - get clock state*/	u32 tag=0x00030001;
	return mb_prop7(tag, clock_id, 0);
}

u32 set_clock_state(u32 clock_id, u32 clock_state)
{
	/* tag - set clock state*/	u32 tag=0x00038001;
	return mb_prop7(tag, clock_id, clock_state);
}

u32 get_max_clock_rate(u32 clock_id)
{
	/* tag - get max clock rate*/	u32 tag=0x00030004;
	return mb_prop7(tag, clock_id, 0);
}
u32 get_min_clock_rate(u32 clock_id)
{
	/* tag - get min clock rate*/	u32 tag=0x00030007;
	return mb_prop7(tag, clock_id, 0);
}

u32 get_clock_rate(u32 clock_id)
{
	/* tag - get clock rate*/	u32 tag=0x00030002;
	return mb_prop7(tag, clock_id, 0);
}
u32 set_clock_rate(u32 clock_id, u32 clock_rate)
{
	/* tag - set clock rate*/	u32 tag=0x00038002;
	return mb_prop7(tag, clock_id, clock_rate);
}
