#include "teletext.h"
#include "mailbox.h"
#include <memutils.h>
#include <console.h>
#include <tty.h>
#include <memory.h>

/* Character cells are 6x10 */
#define CHARSIZE_X	6
#define CHARSIZE_Y	10

/* Screen parameters set in fb_init() */
static unsigned int screenbase, screensize;
static unsigned int fb_x, fb_y, pitch;
/* Max x/y character cell */
static unsigned int max_x, max_y;

/* Current console text cursor position (ie. where the next character will
 * be written
*/
static int consx = 0;
static int consy = 0;

/* Current fg/bg colour */
static unsigned short int fgcolour = 0xffff;
static unsigned short int bgcolour = 0;

/* A small stack to allow temporary colour changes in text */
static unsigned int colour_stack[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static unsigned int colour_sp = 8;

static int initialized = 0;

void init_constants(){
    screenbase = fb->fb_ptr; 
    screensize = fb->fb_size;
    fb_x = fb->width; 
    fb_y = fb->height; 
    pitch = fb->pitch;
    max_x = fb_x / CHARSIZE_X;
    max_y = fb_y / CHARSIZE_Y;

    initialized = 1;
    return;
}


/* Move to a new line, and, if at the bottom of the screen, scroll the
 * framebuffer 1 character row upwards, discarding the top row
 */
static void newline()
{

	unsigned int source;
	/* Number of bytes in a character row */
	register unsigned int rowbytes = CHARSIZE_Y * pitch;

	consx = 0;
	if(consy<(max_y-1))
	{
		consy++;
		return;
	}

	/* Copy a screen's worth of data (minus 1 character row) from the
	 * second row to the first
	 */

	/* Calculate the address to copy the screen data from */
	source = screenbase + rowbytes;
	memmove((void *)screenbase, (void *)source, (max_y-1)*rowbytes);

	/* Clear last line on screen */
	memclr((void *)(screenbase + (max_y-1)*rowbytes), rowbytes);
}

/* Write null-terminated text to the console
 * Supports control characters (see framebuffer.h) for colour and newline
 */
void console_write(char *text)
{
    if( !initialized ){
        init_constants();
    }
    
    volatile unsigned short int *ptr;

	unsigned int row, addr;
	int col;
	unsigned char ch;

	/* Double parentheses to silence compiler warnings about
	 * assignments as boolean values
	 */
	while((ch = (unsigned char)*text))
	{
		text++;

		/* Deal with control codes */
		switch(ch)
		{
			case 1: fgcolour = 0b1111100000000000; continue;
			case 2: fgcolour = 0b0000011111100000; continue;
			case 3: fgcolour = 0b0000000000011111; continue;
			case 4: fgcolour = 0b1111111111100000; continue;
			case 5: fgcolour = 0b1111100000011111; continue;
			case 6: fgcolour = 0b0000011111111111; continue;
			case 7: fgcolour = 0b1111111111111111; continue;
			case 8: fgcolour = 0b0000000000000000; continue;
				/* Half brightness */
			case 9: fgcolour = (fgcolour >> 1) & 0b0111101111101111; continue;
			case 10: newline(); continue;
			case 11: /* Colour stack push */
				if(colour_sp)
					colour_sp--;
				colour_stack[colour_sp] =
					fgcolour | (bgcolour<<16);
				continue;
			case 12: /* Colour stack pop */
				fgcolour = colour_stack[colour_sp] & 0xffff;
				bgcolour = colour_stack[colour_sp] >> 16;
				if(colour_sp<8)
					colour_sp++;
				continue;
			case 17: bgcolour = 0b1111100000000000; continue;
			case 18: bgcolour = 0b0000011111100000; continue;
			case 19: bgcolour = 0b0000000000011111; continue;
			case 20: bgcolour = 0b1111111111100000; continue;
			case 21: bgcolour = 0b1111100000011111; continue;
			case 22: bgcolour = 0b0000011111111111; continue;
			case 23: bgcolour = 0b1111111111111111; continue;
			case 24: bgcolour = 0b0000000000000000; continue;
				/* Half brightness */
			case 25: bgcolour = (bgcolour >> 1) & 0b0111101111101111; continue;
		}

		/* Unknown control codes, and anything >127, get turned into
		 * spaces. Anything >=32 <=127 gets 32 subtracted from it to
		 * turn it into a value between 0 and 95, to index into the
		 * character definitions table
		 */
		if(ch<32)
		{
			ch=0;
		}
		else
		{
			if(ch>127)
				ch=0;
			else
				ch-=32;
		}

		/* Plot character onto screen
		 *
		 * CHARSIZE_Y and CHARSIZE_X are the size of the block the
		 * character occupies. The character itself is one pixel
		 * smaller in each direction, and is located in the upper left
		 * of the block
		 */
		for(row=0; row<CHARSIZE_Y; row++)
		{
			addr = (row+consy*CHARSIZE_Y)*pitch + consx*CHARSIZE_X*2;

			for(col=(CHARSIZE_X-2); col>=0; col--)
			{
				ptr = (unsigned short int *)(screenbase+addr);

				addr+=2;

				if(row<(CHARSIZE_Y-1) && (teletext[ch][row] & (1<<col)))
					*ptr = fgcolour;
				else
					*ptr = bgcolour;
			}

			ptr = (unsigned short int *)(screenbase+addr);
			*ptr = bgcolour;
		}

		if(++consx >=max_x)
		{
			newline();
		}
	}
}

