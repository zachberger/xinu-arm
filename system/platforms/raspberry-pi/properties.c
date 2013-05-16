//Eric Warrington
//Zach Berger

#include "mbox.h"
#include "properties.h"

/*
 * generic function for mailbox properties using a buffer of length 7
 *
 * @param tag_id	the id of the property to query
 * @param p1		the first property (often a device or clock id)
 * @param p2		the second property (usually zero, unless it's being set)
 */
u32 mb_prop7(u32 tag_id, u32 p1, u32 p2)
{
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	buf[0] = 7*4; /* buffer size */	buf[1] = 0; /* request/response code */	buf[2] = tag_id; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = p1; /* req: 1 */	buf[6] = p2; /* req: 2 */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		mailbox_error();		return;	}	return buf[6];
}



/*
 * retrieve firmware revision information
 */
u32 get_vc_firmware_revision()
{
	/* tag - get board revision */
	u32 tag=0x00010002;
	return mb_prop7(tag, 0, 0);
}

/*
 * retrieve board model information
 */
u32 get_board_model()
{
	/* tag - get board revision */
	u32 tag=0x00010001;
	return mb_prop7(tag, 0, 0);
}

/*
 * retrieve board serial number
 */
u64 get_board_serial()
{
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	//u32 addr, size;	buf[0] = 7 * 4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get board serial*/	buf[2] = 0x00010004; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = 0; /* resp: */	buf[6] = 0; /* resp: cont. */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		mailbox_error();		return;	}	return (u64)buf[5];
}

/*
 * retrieve MAC address of the board
 */
u8 get_mac_addr()
{
	/* tag - get MAC address */
	u32 tag=0x00010003;
	return (u8)mb_prop7(tag, 0, 0);
}


/*
 * retrieve ARM memory addr and size
 */
mem_region get_arm_memory()
{
	mem_region MR;
	
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	//u32 addr, size;	buf[0] = 8 * 4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get ARM memory*/	buf[2] = 0x00010005; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = 0; /* req: mem address */	buf[6] = 0; /* req: mem size */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		mailbox_error();		return;	}	MR.base_address=buf[5];	MR.block_size=buf[6];
	
	return MR;
}

/*
 * retrieve VC memory addr and size
 */
mem_region get_vc_memory()
{
	mem_region MR;
	
	volatile u32* buf=(u32*)BUFFER_ADDRESS;	//u32 addr, size;	buf[0] = 8 * 4; /* buffer size */	buf[1] = 0; /* request/response code */	/* tag - get VC memory*/	buf[2] = 0x00010006; /* tag id */	buf[3] = 8; /* value buffer size */	buf[4] = 0+0; /* request indicator + size */	buf[5] = 0; /* req: mem address */	buf[6] = 0; /* req: mem size */	buf[7] = 0; /* end tags */	mailbox_write(MAIL_CHAN_ARMTOVC_PROP, (u32)buf);	mailbox_read(MAIL_CHAN_ARMTOVC_PROP);	if(buf[1]!=MAIL_PROP_RESPONSE_OK)	{		mailbox_error();		return;	}	MR.base_address=buf[5];	MR.block_size=buf[6];
}


//return the temperature
/*
 * return whether or not turbo is turned on
 */
u32 get_temperature()
{	/* tag - get temperature*/
	u32 tag=0x00030006;	return mb_prop7(tag, 0, 0);
}

//get the max temperature
u32 get_max_temperature()
{	/* tag - get max temperature*/	u32 tag=0x0003000A;
	return mb_prop7(tag, 0, 0);
}


/*
 * return voltage as offset from 1.2V in units of 0.025V
 *	The voltage value may be clamped to the supported range. A value of 0x80000000 means the id was not valid.
 *
 * @param device_id	the id of the device (for a complete list, see the .h file)
 */
u32 get_voltage(u32 device_id)
{	/* tag - get voltage*/	u32 tag=0x00030003;
	return mb_prop7(tag, device_id, 0);
}


/*
 * set voltage as offset from 1.2V in units of 0.025V
 *
 * @param device_id	the id of the device (for a complete list, see the .h file)
 * @param voltage	the voltage to set
 */
u32 set_voltage(u32 device_id, u32 voltage)
{	/* tag - set voltage*/	u32 tag=0x00038003;	return mb_prop7(tag, device_id, voltage); 
}

/*
 * Return the maximum supported voltage rate for the given id.
 *	Voltages should not be set higher than this.
 *
 * @param device_id	the id of the device (for a complete list, see the .h file)
 */
u32 get_max_voltage(u32 device_id)
{	/* tag - get max voltage*/	u32 tag=0x00030005;
	return mb_prop7(tag, device_id, 0);
}
 
/*
 * Return the minimum supported voltage rate for the given id. This may be used when idle.
 *
 * @param device_id	the id of the device (for a complete list, see the .h file)
 */
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


/*
 * get the power state of the provided device
 * 
 * @param device_id	the id of the device (for a complete list, see the .h file)
 */
u32 get_device_state(u32 device_id)
{
	/* tag - get device state*/	u32 tag=0x00020001;
	return mb_prop7(tag, device_id, 0);
}

/*
 * set the power state of the provided device
 *
 * @param device_id	the id of the device (for a complete list, see the .h file)
 */
u32 set_device_state(u32 device_id, u32 state)
{
	/* tag - set device state*/	u32 tag=0x00028001;
	return mb_prop7(tag, device_id, state);
}

/*
 * get the timing of the provided device
 *	Response indicates wait time required after turning a device on before power is stable.
 *	Returns 0 wait time if the device does not exist.
 *
 * @param device_id	the id of the device (for a complete list, see the .h file)
 */
u32 get_device_timing(u32 device_id)
{
	/* tag - get device timing*/	u32 tag=0x00020002;
	return mb_prop7(tag, device_id, 0);
}



/*
 * return whether or not turbo is turned on
 */
u32 get_turbo()
{
	/* tag - get turbo*/	u32 tag=0x00030009;
	return mb_prop7(tag, 0, 0);
}


/*
 * set whether or not turbo is turned on
 *	Level will be zero for non-turbo and one for turbo.
 *	This will cause GPU clocks to be set to maximum when enabled and minimum when disabled.
 */
u32 set_turbo(u32 turbo)
{
	/* tag - set turbo*/	u32 tag=0x00038009;
	return mb_prop7(tag, 0, turbo);
}



/*
 * get the state of the clock
 *
 * @param clock_id	the id of the clock (for a complete list, see the .h file)
 */
u32 get_clock_state(u32 clock_id)
{
	/* tag - get clock state*/	u32 tag=0x00030001;
	return mb_prop7(tag, clock_id, 0);
}

/*
 * set the state of the clock
 *
 * @param clock_id	the id of the clock (for a complete list, see the .h file)
 */
u32 set_clock_state(u32 clock_id, u32 clock_state)
{
	/* tag - set clock state*/	u32 tag=0x00038001;
	return mb_prop7(tag, clock_id, clock_state);
}


/*
 * get the max rate of the clock
 *
 * @param clock_id	the id of the clock (for a complete list, see the .h file)
 */
u32 get_max_clock_rate(u32 clock_id)
{
	/* tag - get max clock rate*/	u32 tag=0x00030004;
	return mb_prop7(tag, clock_id, 0);
}

/*
 * get the min rate of the clock
 *
 * @param clock_id	the id of the clock (for a complete list, see the .h file)
 */
u32 get_min_clock_rate(u32 clock_id)
{
	/* tag - get min clock rate*/	u32 tag=0x00030007;
	return mb_prop7(tag, clock_id, 0);
}

/*
 * get the rate of the clock
 *
 * @param clock_id	the id of the clock (for a complete list, see the .h file)
 */
u32 get_clock_rate(u32 clock_id)
{
	/* tag - get clock rate*/	u32 tag=0x00030002;
	return mb_prop7(tag, clock_id, 0);
}

/*
 * set the rate of the clock
 *
 * @param clock_id	the id of the clock (for a complete list, see the .h file)
 */
u32 set_clock_rate(u32 clock_id, u32 clock_rate)
{
	/* tag - set clock rate*/	u32 tag=0x00038002;
	return mb_prop7(tag, clock_id, clock_rate);
}
