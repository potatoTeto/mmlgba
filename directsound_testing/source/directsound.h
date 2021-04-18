#ifndef DIRECTSOUND_H
#define DIRECTSOUND_H

#define REG_DMA1CNT_H   	(*(volatile u16*)0x40000C6)
#define REG_DMA3CNT_L   	(*(volatile u16*)0x40000DC)
#define REG_DMA3CNT_H   	(*(volatile u16*)0x40000DE)


#define IN_IWRAM      __attribute__ ((section (".iwram")))
#define IN_EWRAM      __attribute__ ((section (".ewram")))

#define DMA_MEMSET32		(BIT(8) | BIT(10) | BIT(15))


#define SND_MAX_CHANNELS		4

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
	s8		*data;
	u32		pos;
	u32		inc;
	u32		vol;
	u32		length;
	u32		loopLength;

} SOUND_CHANNEL;

typedef struct _SOUND_VARS
{
	s8		*mixBufferBase;
	s8		*curMixBuffer;
	u32		mixBufferSize;
	u8		activeBuffer;

} SOUND_VARS;



extern SOUND_CHANNEL	sndChannel[SND_MAX_CHANNELS];
extern SOUND_VARS		sndVars;



inline void Dma3(void *dest, const void *src, u32 count, u16 flags)
{
	REG_DMA3SAD = (u32)src;
	REG_DMA3DAD = (u32)dest;
	REG_DMA3CNT_L = count; // low byte
	REG_DMA3CNT_H = flags; // high byte
}


extern void SndInit(SND_FREQ freq);
extern void DSSwapAudioBuffers() IN_IWRAM;	// in main.c
extern void StepDirectSound();

#endif
