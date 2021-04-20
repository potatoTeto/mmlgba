#ifndef DIRECTSOUND_H
#define DIRECTSOUND_H

#include <gba_console.h>
#include <gba_dma.h>

#define REG_DMA1CNT	*(vu32*)(REG_BASE + 0x0c4)
#define REG_DMA1CNT_H   	(*(volatile u16*)0x40000C6)
#define REG_DMA3CNT_L   	(*(volatile u16*)0x40000DC)
#define REG_DMA3CNT_H   	(*(volatile u16*)0x40000DE)


#define IN_IWRAM      __attribute__ ((section (".iwram")))
#define IN_EWRAM      __attribute__ ((section (".ewram")))

#define DMA_MEMSET32		(BIT(8) | BIT(10) | BIT(15))


#define SND_MAX_DS_CHANNELS		6

typedef enum _SND_FREQ
{
  SND_FREQ_5735,
  SND_FREQ_9079,
  SND_FREQ_10513,
  SND_FREQ_11469,
  SND_FREQ_13380,
  SND_FREQ_15769,
  SND_FREQ_18158,
  SND_FREQ_21025,
  SND_FREQ_26760,
  SND_FREQ_31537,
  SND_FREQ_36316,
  SND_FREQ_40138,
  SND_FREQ_42049,
	SND_FREQ_NUM
} SND_FREQ;

/*
typedef enum _SND_FREQ
{
  SND_FREQ_5734,
	SND_FREQ_10512,
	SND_FREQ_13379,
	SND_FREQ_18157,
	SND_FREQ_21024,
	SND_FREQ_26758,
	SND_FREQ_31536,
	SND_FREQ_36314,
	SND_FREQ_40137,
	SND_FREQ_42048,
	SND_FREQ_43959,

	SND_FREQ_NUM

} SND_FREQ;
*/


typedef struct _SOUND_CHANNEL
{
	s8		*sampleData;
	u32		samplePos;
	u32		sampleInc;
	u32		sampleVol;
	u32		sampleLength;
	u32		sampleLoopLength;

  u8 mus_enabled;
  u8 mus_done;
  u16 mus_freq;
  u8 mus_octave;
  u8 mus_length;
  u8 mus_volume;
  u8 mus_env;
  u8 mus_pan;
  u8 mus_wait;
  u16 mus_target;
  u8 mus_slide;
  u8 mus_vib_speed;
  u8* mus_vib_table;
  u8 mus_vib_pos;
  u8 mus_vib_delay;
  u8 mus_po; //pitch offset
  u8 mus_macro;
  u8* ds_mus_rep[4];
  u8 mus_repeats[4];
  u8 mus_rep_depth;

} SOUND_CHANNEL;

typedef struct _SOUND_VARS
{
	s8		*mixBufferBase;
	s8		*curMixBuffer;
	u32		mixBufferSize;
	u8		activeBuffer;

} SOUND_VARS;



extern SOUND_CHANNEL	sndChannel[SND_MAX_DS_CHANNELS];
extern SOUND_VARS		sndVars;
extern int sampleBitRate, streamBitRate;


inline void Dma3(void *dest, const void *src, u32 count, u16 flags)
{
	REG_DMA3SAD = (u32)src;
	REG_DMA3DAD = (u32)dest;
	REG_DMA3CNT_L = count; // low byte
	REG_DMA3CNT_H = flags; // high byte
}


extern void dirSndInit(SND_FREQ freq);
extern void DSSwapAudioBuffers() IN_IWRAM;	// in main.c
extern void StepDirectSound();
//void directSoundUpdate(void);

#endif // !DIRECTSOUND_H
