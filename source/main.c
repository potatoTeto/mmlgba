
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_timers.h>
#include <gba_input.h>
#include <gba_dma.h>
#include <stdio.h>
#include <stdlib.h>
#include "mmlgba/mmlgba.h"
#include "mmlgba/music.h"
#include "mmlgba/directsound.h"
#include "data/bgm/mainmenu.h"
#include "data/bgm/plains.h"
#include "data/bgm/space.h"
#include "data/bgm/test.h"
#include "data/bgm/streams/musMonchi.h"
#include "data/samples/bigKick.h"
#include "data/samples/smpPiano.h"


s32 i, keys_pressed, keys_released;
char strMsg[] = "xxxxxx";

// Function called when timer 0 overflows
void musicUpdate(void) {
	// Update MMLGBA's PSG and DirectSound channels (Pulse 1, Pulse 2, Wave, Noise, DirectSound Ch. 1-6)
	musicSequencerUpdate();
}

  void DSSwapAudioBuffers()
  {
  	if(sndVars.activeBuffer == 1)	// buffer 1 just got over
  	{
  		// Start playing buffer 0
  		//*(volatile u32*)0x40000C4 = 0;
  		REG_DMA1CNT = 0;
  		//*(volatile u32*)0x40000BC = (u32)sndVars.mixBufferBase;
  		REG_DMA1SAD = (u32) sndVars.mixBufferBase; //(u32) &musMonchi; //
  		//*(volatile u16*)0x40000C6 = BIT(6) | BIT(9) | BIT(10) | (BIT(12) | BIT(13)) | BIT(15); //DMA_DST_FIXED | DMA_REPEAT | DMA16 | DMA_MODE_FIFO | DMA_ENABLE;
  		REG_DMA1CNT_H =	0xB640;//0xB640;//BIT(6) | BIT(9) | BIT(10) | (BIT(12) | BIT(13)) | BIT(15);//0xB640;

  		// Set the current buffer pointer to the start of buffer 1
  		sndVars.curMixBuffer = sndVars.mixBufferBase + sndVars.mixBufferSize;
  		sndVars.activeBuffer = 0;
  	}
  	else	// buffer 0 just got over
  	{
  		// DMA points to buffer 1 already, so don't bother stopping and resetting it

  		// Set the current buffer pointer to the start of buffer 0
  		sndVars.curMixBuffer = sndVars.mixBufferBase;
  		sndVars.activeBuffer = 1;
  	}
  }
//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
	//---------------------------------------------------------------------------------

	REG_TM3CNT_L = -193; // ~106bpm starting value
	REG_TM3CNT_H = 0x3 | TIMER_IRQ | TIMER_START; // 16384 hz | use IRQ | On

	mus_init((UBYTE*)&test_data);

	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqSet(IRQ_VBLANK, DSSwapAudioBuffers);
	irqEnable(IRQ_VBLANK);
	irqEnable(IRQ_TIMER3);
	irqSet(IRQ_TIMER3, musicUpdate);

	consoleDemoInit();
	// Initialize the sound engine (in directsound.c)
	dirSndInit(SND_FREQ_11469);

	// ansi escape sequence to set print co-ordinates
	// /x1b[line;columnH
	while (1) {
		VBlankIntrWait();

		scanKeys();

		keys_pressed = keysDown();
		keys_released = keysUp();

		    if (keys_pressed & KEY_L) {
		      // make sure the channel is disabled first
		      sndChannel[1].sampleData = 0;

		      // set up channel vars

		      // Start at the start
		      sndChannel[1].samplePos = 0;

		      // Calculate the increment. The piano sample was
		      // originally 8363Hz, so I've hardcoded it in here.
		      // Later we'll want to make a table of info on samples
		      // so it can be done automatically.
		      // also, it is 12-bit fixed-point, so shift up before the divide
		      sndChannel[1].sampleInc			= (10000<<12)/streamBitRate;

		      // Set the volume to maximum
		      sndChannel[1].sampleVol = 64;

		      // The length of the original sample (also 12-bit fixed-point)
		      // This will go into our sample info table too
		      sndChannel[1].sampleLength = 11396 << 12;
		      // Set the loop length to the special no-loop marker value
		      sndChannel[1].sampleLoopLength = 0;

		      // Set the data. This will actually start the channel up so
		      // it will be processed next time StepDirectSound() is called
		      sndChannel[1].sampleData = (s8*) bigKick;
		    }
		/*
		if (keys_pressed & KEY_START) {
				sndChannel[3].data = 0;
				sndChannel[3].pos = 0;
				sndChannel[3].inc = (12000<<12)/streamBitRate;
				sndChannel[3].vol = 64;
				sndChannel[3].length = 720000 << 12;//0xF0000000;//1089420 << 12;   0x427E30   983040  0x7FFFFFFF    0xF0000000
				sndChannel[3].loopLength = -1;
				sndChannel[3].data = (s8*) musMonchi;
		}


    if (keys_pressed & KEY_R) {
      // make sure the channel is disabled first
      sndChannel[1].data = 0;

      // set up channel vars
      sndChannel[1].pos = 0;

      // Play at 2* frequency. This is one octave above normal
      sndChannel[1].inc = (sampleBitRate*2<<12)/streamBitRate;

      // Same
      sndChannel[1].vol = 64;

      // Same
      sndChannel[1].length = 10516<<12;

      // This one will actually loop. Here, the loop start
      // position (which I checked in the MOD I ripped the
      // sample from) is 9176. So, we take length-loopStart
      // and convert to 12-bit fixed-point like everything else.
      sndChannel[1].loopLength = (10514-9176)<<12;

      // Same
      sndChannel[1].data = (s8*) smpPiano;
    }
		if (keys_released & KEY_R) {
			sndChannel[1].data = 0;
		}

    if (keys_pressed & KEY_B) {
      for (i = 0; i < SND_MAX_CHANNELS; i++) {
        // Disable all the channels by setting the special disable value
        sndChannel[i].data = 0;
      }
    }
*/
		for (i = 0; i < SND_MAX_DS_CHANNELS; i++) {
			// Disable all the channels by setting the special disable value
			if (sndChannel[i].mus_note_on)
				strMsg[i] = 'X';
			else
				strMsg[i] = 'x';
		}
		iprintf("\x1b[0;0H mmlgba Sound Test v1.0\n   by potatoTeto\n\n\n github.com/potatoTeto/mmlgba\n\n\n\n\n                   %s\n%x", strMsg,sndChannel[0].output_freq);

		StepDirectSound();
	}
}
