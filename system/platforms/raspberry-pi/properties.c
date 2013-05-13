#include "mbox.h"
#include "properties.h"

/* use a global variable since we have no malloc */

mem_region get_arm_memory()
{
	mem_region MR;
	
	volatile u32* buf=(u32*)BUFFER_ADDRESS;
	
	return MR;
}

mem_region get_vc_memory()
{
	mem_region MR;
	
	volatile u32* buf=(u32*)BUFFER_ADDRESS;
}

u32 get_temperature();
u32 get_max_temperature();


/**
 * Device IDs
 */
#define RESERVED 0x000000000
#define CORE 	 0x000000001
#define SDRAM_C  0x000000002
#define SDRAM_P  0x000000003
#define SDRAM_I  0x000000004

u32 get_voltage( u32 device_id );
u32 get_max_voltage( u32 device_id );
u32 set_voltage( u32 device_id, u32 voltage );


#define SD_CARD 0x00000000
#define UART0 	0x00000001
#define UART1   0x00000002
#define USB_HCD 0x00000003
#define I2C0 	0x00000004
#define I2C1 	0x00000005
#define I2C2 	0x00000006 
#define SPI 	0x00000007
#define CCP2TX 	0x00000008

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