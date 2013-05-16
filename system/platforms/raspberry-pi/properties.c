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






u32 get_temperature()
{
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	buf[0] = 7*4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get temperature*/	buf[2] = 0x00030006; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = 0; /* req: temperature id */	buf[6] = 0; /* req: value */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		fb_error();		return;	}	return buf[6];
}

u32 get_max_temperature()
{
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	//u32 addr, size;	buf[0] = 7*4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get max temperature*/	buf[2] = 0x0003000A; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = 0; /* req: temperature id */	buf[6] = 0; /* req: value */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		fb_error();		return;	}	return buf[6];
}




//(offset from 1.2V in units of 0.025V)
u32 get_voltage(u32 device_id)
{
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	buf[0] = 7*4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get voltage*/	buf[2] = 0x00030003; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = device_id; /* req: voltage id */	buf[6] = 0; /* req: value (offset from 1.2V in units of 0.025V) */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		fb_error();		return;	}	return buf[6];
}

//Return the maximum supported voltage rate for the given id. Voltages should not be set higher than this. 
u32 get_max_voltage(u32 device_id)
{
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	buf[0] = 7*4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get max voltage*/	buf[2] = 0x00030005; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = device_id; /* req: voltage id */	buf[6] = 0; /* req: value (offset from 1.2V in units of 0.025V) */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		fb_error();		return;	}	return buf[6];
}

u32 set_voltage(u32 device_id, u32 voltage)
{
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	buf[0] = 7*4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - set voltage*/	buf[2] = 0x00038003; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = device_id; /* req: voltage id */	buf[6] = voltage; /* req: value (offset from 1.2V in units of 0.025V) */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		fb_error();		return;	}	return buf[6];
}



u32 get_device_state( u32 device_id );
u32 get_device_timing( u32 device_id );
u32 set_device_state( u32 device_id, u32 state );

u32 get_turbo();
u32 set_turbo( u32 turbo );



#define RESERVED 	0x000000000
#define EMMC     	0x000000001 
#define UART  	 	0x000000002
#define ARM   		0x000000003 
#define CORE  		0x000000004
#define V3D   		0x000000005
#define H264  		0x000000006
#define ISP   		0x000000007
#define SDRAM 		0x000000008
#define PIXEL 		0x000000009
#define PWM   		0x00000000a

u32 get_clock_state( u32 clock_id );
u32 set_slock_state( u32 clock_id, u32 clock_state);

u32 get_min_clock_rate( u32 clock_id );
u32 get_max_clock_rate( u32 clock_id );

u32 get_clock_rate( u32 clock_id );
u32 set_clock_rate( u32 clock_id, u32 clock_rate );
