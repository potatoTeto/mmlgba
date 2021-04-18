#ifndef  MMLGBA_H
#define  MMLGBA_H

typedef unsigned char UBYTE, UINT8;
typedef unsigned short UWORD;

#define VU8	volatile UBYTE *
#define VU16 volatile UWORD *

#define	GBA_REG_BASE	0x04000000

//---------------------------------------------------------------------------------
// pin8 compatible sound macros
//---------------------------------------------------------------------------------
#define SND_STAT (*(VU16)0x04000084)
#define SNDSTAT_SQR1   0x0001
#define SNDSTAT_SQR2   0x0002
#define SNDSTAT_TRI    0x0004
#define SNDSTAT_NOISE  0x0008
#define SNDSTAT_ENABLE 0x0080

#define DMGSNDCTRL (*(VU16)0x04000080)
#define DMGSNDCTRL_LVOL(x) (x)
#define DMGSNDCTRL_RVOL(x) ((x) << 4)
#define DMGSNDCTRL_LSQR1   0x0100
#define DMGSNDCTRL_LSQR2   0x0200
#define DMGSNDCTRL_LTRI    0x0400
#define DMGSNDCTRL_LNOISE  0x0800
#define DMGSNDCTRL_RSQR1   0x1000
#define DMGSNDCTRL_RSQR2   0x2000
#define DMGSNDCTRL_RTRI    0x4000
#define DMGSNDCTRL_RNOISE  0x8000

#define TRICTRL         (*(VU16)0x04000070)
#define TRICTRL_2X32    0x0000
#define TRICTRL_1X64    0x0020
#define TRICTRL_BANK(x) ((x) << 6)
#define TRICTRL_ENABLE  0x0080

// Square 1
#define NR10_REG  (*(VU8) (GBA_REG_BASE + 0x060)) //    FF10 - PPP NSSS Sweep period, negate, shift
#define NR11_REG  (*(VU8) (GBA_REG_BASE + 0x062)) //    FF11 DDLL LLLL Duty, Length load(64 - L)
#define NR12_REG  (*(VU8) (GBA_REG_BASE + 0x063)) //    FF12 VVVV APPP Starting volume, Envelope add mode, period
#define NR13_REG  (*(VU8) (GBA_REG_BASE + 0x064)) //    FF13 FFFF FFFF Frequency LSB
#define NR14_REG  (*(VU8) (GBA_REG_BASE + 0x065)) //    FF14 TL-- - FFF Trigger, Length enable, Frequency MSB

// Square 2
// FF15---- ----Not used
#define NR21_REG  (*(VU8) (GBA_REG_BASE + 0x068)) //    FF16 DDLL LLLL Duty, Length load(64 - L)
#define NR22_REG  (*(VU8) (GBA_REG_BASE + 0x069)) //    FF17 VVVV APPP Starting volume, Envelope add mode, period
//											 0x006A - Not used
#define NR23_REG  (*(VU8) (GBA_REG_BASE + 0x06C)) //    FF18 FFFF FFFF Frequency LSB
#define NR24_REG  (*(VU8) (GBA_REG_BASE + 0x06D)) //    FF19 TL-- - FFF Trigger, Length enable, Frequency MSB

//Wave
#define NR30_REG  (*(VU8) (GBA_REG_BASE + 0x070)) //    FF1A E-- - ----DAC power
#define NR31_REG  (*(VU8) (GBA_REG_BASE + 0x072)) //    FF1B LLLL LLLL Length load(256 - L)
#define NR32_REG  (*(VU8) (GBA_REG_BASE + 0x073)) //    FF1C - VV - ----Volume code(00 = 0 %, 01 = 100 %, 10 = 50 %, 11 = 25 %)
#define NR33_REG  (*(VU8) (GBA_REG_BASE + 0x074)) //    FF1D FFFF FFFF Frequency LSB
#define NR34_REG  (*(VU8) (GBA_REG_BASE + 0x075)) //    FF1E TL-- - FFF Trigger, Length enable, Frequency MSB

//Noise
//FF1F---- ----Not used
#define NR41_REG  (*(VU8) (GBA_REG_BASE + 0x078)) //    FF20 --LL LLLL Length load(64 - L)
#define NR42_REG  (*(VU8) (GBA_REG_BASE + 0x079)) //    FF21 VVVV APPP Starting volume, Envelope add mode, period
#define NR43_REG  (*(VU8) (GBA_REG_BASE + 0x07C)) //    FF22 SSSS WDDD Clock shift, Width mode of LFSR, Divisor code
#define NR44_REG  (*(VU8) (GBA_REG_BASE + 0x07D)) //    FF23 TL-- ----Trigger, Length enable

//Control / Status
#define NR50_REG  (*(VU8) (GBA_REG_BASE + 0x080)) //    FF24 ALLL BRRR Vin L enable, Left vol, Vin R enable, Right vol
#define NR51_REG  (*(VU8) (GBA_REG_BASE + 0x081)) //    FF25 NW21 NW21 Left enables, Right enables
#define NR52_REG  (*(VU8) (GBA_REG_BASE + 0x082)) //    FF26 P-- - NW21 Power control / status, Channel length statuses

#define TMA_REG	   (*(VU16) (GBA_REG_BASE + 0x100))  // Timer 0 modulo
//#define TAC_REG	   (*(VU8) (GBA_REG_BASE + 0x102))	// Timer 0 control 

//Wave Table
// FF30 0000 1111 Samples 0 and 1
//....
// FF3F 0000 1111 Samples 30 and 31
#define	WAVE_RAM	((VU8)	(GBA_REG_BASE+ 0x090))

#endif // !MMLGBA_H
