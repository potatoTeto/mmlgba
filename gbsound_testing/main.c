
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include "mmlgba/mmlgba.h"
#include "mmlgba/music.h"
#include "data/bin/mainmenu.h"
#include "data/bin/plains.h"
#include "data/bin/space.h"

int timer = 0;

// Function called when timer 0 overflows
void musicUpdate(void) {
	// For display, can delete
	timer++;

	// Update MMLGBA's PSG channels (Pulse 1, Pulse 2, Wave, Noise)
	gbSoundUpdate();

	// Update the DirectSound stream
	directSoundUpdate();
}

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
	//---------------------------------------------------------------------------------

	REG_TM0CNT_L = -193; // ~106bpm starting value
	REG_TM0CNT_H = 0x3 | TIMER_IRQ | TIMER_START; // 16384 hz | use IRQ | On

	mus_init((UBYTE*)&mainmenu_data);

	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);
	irqEnable(IRQ_TIMER0);
	irqSet(IRQ_TIMER0, musicUpdate);

	consoleDemoInit();

	// ansi escape sequence to set print co-ordinates
	// /x1b[line;columnH
	while (1) {
		VBlankIntrWait();
		iprintf("\x1b[10;10HTest %d!\n", timer);
	}
}
