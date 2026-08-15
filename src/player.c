#include <stdio.h>

#include "player.h"
#include "tfmxsong.h"
#include "machine/endian.h"
#include "SDL.h"

#define NOTSUPPORTED fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum)

int notevals[] = {0x6AE,0x64E,0x5F4,0x59E,0x54D,0x501,
0x4B9,0x475,0x435,0x3F9,0x3C0,0x38C,0x358,0x32A,0x2FC,0x2D0,0x2A8,0x282,
0x25E,0x23B,0x21B,0x1FD,0x1E0,0x1C6,0x1AC,0x194,0x17D,0x168,0x154,0x140,
0x12F,0x11E,0x10E,0x0FE,0x0F0,0x0E3,0x0D6,0x0CA,0x0BF,0x0B4,0x0AA,0x0A0,
0x097,0x08F,0x087,0x07F,0x078,0x071,0x0D6,0x0CA,0x0BF,0x0B4,0x0AA,0x0A0,
0x097,0x08F,0x087,0x07F,0x078,0x071,0x0D6,0x0CA,0x0BF,0x0B4 };

struct Audio audioData[8];
struct TrackManager trackManager;
struct Channel channelData[16];
struct PatternBlock patternBlockData;
struct Idb idb;

extern struct Header hdr;

extern int startPat,gemx,loops,dangerFreakHack,oopsUpHack,monkeyHack;

S8 tempVol;

int jiffies=0;

int multimode=0;
U32 eClocks=14318;

void NotePort(U32 i);
int LoopOff(struct Audio *audio);
int LoopOn(struct Audio *audio);
void RunMacro(struct Channel *channel, U32 nChannel);
void DoEffects(struct Channel *channel);
void DoMacro(int cc);
void DoAllMacros(void);
void ChannelOff(int i);
void DoFade(int sp,int dv);
void GetTrackStep(void);
int DoTrack(struct Pattern *p/* ,int pp  */);
void DoTracks(void);
void tfmxIrqIn(void);
void AllOff(void);
void TfmxInit(void);
void StartSong(int song, int mode);

void NotePort(U32 i)
{
	//printf("*** NotePort: Note Port\n");
	UNI x;
	struct Channel *channel;
	x.l=i;
	channel=&channelData[x.b.b2&(multimode?7:3)];
	if (x.b.b0==0xFC)
	{ /* lock */
		channel->SfxFlag=x.b.b1;
		channel->SfxLockTime=x.b.b3;
		return;
	}
	if (channel->SfxFlag) return;
	if (x.b.b0<0xC0)
	{
		if (!dangerFreakHack)
			channel->Finetune=(int)x.b.b3;
		else
			channel->Finetune=0;

		channel->Velocity=(x.b.b2>>4)&0xF;
		channel->PrevNote=channel->CurrNote;
		channel->CurrNote=x.b.b0;
		channel->ReallyWait=1;
		channel->NewStyleMacro=0xFF;
		channel->MacroPtr=macros[channel->MacroNum=x.b.b1];
		
		channel->MacroStep=channel->EfxRun=channel->MacroWait=0;
		
		channel->KeyUp=1;
		channel->Loop=-1;
		channel->MacroRun=-1;
	}
	else if (x.b.b0<0xF0)
	{
		channel->PortaReset=x.b.b1;
		channel->PortaTime=1;
		if (!channel->PortaRate) channel->PortaPer=channel->DestPeriod;
		channel->PortaRate=x.b.b3;
		channel->DestPeriod=(notevals[channel->CurrNote=(x.b.b0&0x3F)]);
	}
	else switch (x.b.b0)
	{
	case 0xF7: /* enve */
		channel->EnvRate=x.b.b1;
		channel->EnvReset=channel->EnvTime=(x.b.b2>>4)+1;
		channel->EnvEndvol=x.b.b3;
		break;
	case 0xF6: /* vibr */
		channel->VibTime=(channel->VibReset=(x.b.b1&0xFE))>>1;
		channel->VibWidth=x.b.b3;
		channel->VibFlag=1; /* ?! */
		channel->VibOffset=0;
		break;
	case 0xF5: /* kup^ */
		channel->KeyUp=0;
		break;
	}
}

#define MAYBEWAIT if (channel->NewStyleMacro==0x0) {\
		channel->NewStyleMacro=0xFF;\
		break;\
	} else {\
		return;\
	}

int LoopOff(struct Audio *audio)
{
	(void)audio;
	return 1;
}

int LoopOn(struct Audio *audio)
{
	if (!audio->channel) return 1;
	if (audio->channel->WaitDMACount--) return 1;
	audio->loop=&LoopOff;
	audio->channel->MacroRun=0xFF;
	return 1;
}

void RunMacro(struct Channel *channel, U32 nChannel)
{
	//printf("*** RunMacro: Run Macro\n");
	UNI x;
	register int a=0;
	channel->MacroWait=0;
	loop:
	x.l=ntohl(editbuf[channel->MacroPtr+(channel->MacroStep++)]);
	a=x.b.b0;
	x.b.b0=0;
	DEBUG(3)
	{
		printf("\nMacro on channel[%d] %02x:%02x:%02x:%02x\n",
			nChannel,a,x.b.b1,x.b.b2,x.b.b3);
		fflush(stdout);
	}

	//printf("a value in macro: [%d][%02x]\n",a,a);

	switch (a)
	{
	case 0: /* dmaoff+reset */
		DEBUG(3) puts("DMAOff+Reset");
		channel->EnvReset=channel->VibReset=/*channel->ArpRun=channel->SIDSize=*/channel->PortaRate=
		channel->AddBeginTime=0;

		if (gemx)
		{
			if (x.b.b2)
				channel->CurVol=x.b.b3;
			else
				channel->CurVol=x.b.b3+(channel->Velocity)*3;
		}
	case 0x13: /* dmaoff */
		DEBUG(3) puts("DMAOff");
		channel->audio->loop=&LoopOff;
		if (!x.b.b1)
		{
			channel->audio->mode=0;
/* START Added by Stefan Ohlsson.
   Removes glitch in TurricanII World2 Song0, among others */
			if(channel->NewStyleMacro)
			{
				channel->audio->slen=0;
			}
/* END */
			break;
		}
		else
		{
			channel->audio->mode|=4;
			channel->NewStyleMacro=0;
			return;
		}
	case 0x1: /* dma on */
		DEBUG(3) puts("DMAOn");
		channel->EfxRun=x.b.b1;
		channel->audio->mode=1;
		if ((!channel->NewStyleMacro)||(dangerFreakHack))
		{
			channel->audio->SampleStart = &smplbuf[channel->SaveAddr];
			channel->audio->SampleLength = (channel->SaveLen) ? channel->SaveLen << 1 : 65535;
			channel->audio->sbeg=channel->audio->SampleStart;
			channel->audio->slen=channel->audio->SampleLength;
			channel->audio->pos=0;
			channel->audio->mode|=2;
			break;
		}
		else
		{
			/*printf("--- using new style macro ---\n");*/
			break;
		}
	case 0x2: /* setbegin */
		DEBUG(3) puts("SetBegin");
		channel->AddBeginTime=0;
		channel->SaveAddr=channel->CurAddr=x.l;
		break;
	case 0x11: /* addbegin */
		DEBUG(3) printf("AddBegin - time=%02x, delta=%04x",
				    x.b.b1,(int)x.w.w1);
		channel->AddBeginTime=channel->AddBeginReset=x.b.b1;
		a=channel->CurAddr+(channel->AddBegin=(S16)x.w.w1);
/*		if (channel->SIDSize)
			channel->SIDSrcSample=channel->CurAddr=a;
		else
*/			channel->SaveAddr=channel->CurAddr=a;
		break;
	case 0x3: /* setlen */
		DEBUG(3) puts("SetLen");
		channel->SaveLen=channel->CurrLength=x.w.w1;
		break;
	case 0x12: /* addlen */
		DEBUG(3) puts("AddLen");
		channel->CurrLength+=x.w.w1;
		a=channel->CurrLength;
/*		if (channel->SIDSize)
			channel->SIDSrcLength=a;
		else*/
			channel->SaveLen=a;
		break;
	case 0x4:
		DEBUG(3) puts("Wait");
		if (x.b.b1&0x01)
		{
			if (channel->ReallyWait++)
				return;
		}
		/* this fixes part of the Z-Out theme problem, but actually it is WRONG!
		bytes/words are already ordered according to byteorder in tfmxplay.h */
/*#ifdef WORDS_BIGENDIAN
		channel->MacroWait=x.w.w0;
#else*/
		channel->MacroWait=x.w.w1;
/*#endif*/
		MAYBEWAIT;
	case 0x1A:
		DEBUG(3) puts("Wait on DMA");
		channel->audio->loop=&LoopOn;
		channel->audio->channel=channel;
		channel->WaitDMACount=x.w.w1;
		channel->MacroRun=0;
/*		return;*/
		MAYBEWAIT;
	case 0x1C: /* note split */
		DEBUG(3) puts("Splitnote");
		if (channel->CurrNote>x.b.b1)
			channel->MacroStep=x.w.w1;
		break;
	case 0x1D: /* vol split */
		DEBUG(3) puts("Splitvol");
		if (channel->CurVol>x.b.b1)
			channel->MacroStep=x.w.w1;
		break;
/*
TODO: add random play/random limit (0x1e/0x1b) for Master Blazer Ingame:
(need more docs!)
*/
	case 0x1B: /* TODO: random play */
	        printf("TODO: random play (0x1B)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
		break;
	case 0x1E: /* TODO:random limit */
	        printf("TODO: random limit (0x1E)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
	        break;
	case 0x10: /* loop key up */
		DEBUG(3) puts("Loop key up");
		if (!channel->KeyUp)
			break;
	case 0x5: /* loop */
		DEBUG(3) puts("Loop");
		if (!(channel->Loop--))
			break;
		else if (channel->Loop<0)
			channel->Loop=x.b.b1-1;
		channel->MacroStep=x.w.w1;
		break;
	case 0x7: /* stop */
		DEBUG(3) puts("STOP");
		channel->MacroRun=0;
		return;
	case 0xD: /* add volume */
		DEBUG(3) puts("Addvolume");
		if (x.b.b2!=0xFE)
		{
			/* --- neofix --- */
			/*channel->CurVol=(channel->Velocity*3)+x.b.b3;*/
			tempVol=(channel->Velocity*3)+x.b.b3;
			if (tempVol>0x40)
				channel->CurVol=0x40;
			else
				channel->CurVol=tempVol;
			/* --- neofix end --- */
			break;
		}
		NOTSUPPORTED;
		break;
/*			channel->CurVol=channel->Velocity*3+x.b.b3;
		PutNote */
	case 0xE: /* set volume */
		DEBUG(3) puts("Setvolume");
		if (x.b.b2!=0xFE)
		{
			channel->CurVol=x.b.b3;
			break;
		}
		NOTSUPPORTED;
		break;
	case 0x21: /* start macro */
		DEBUG(3) puts("Play macro");
		x.b.b0=channel->CurrNote;
		x.b.b2|=channel->Velocity<<4;
		NotePort(x.l);
		break;
	case 0x1F: /* set prev note */
		DEBUG(3) puts("AddPrevNote");
		a=channel->PrevNote;
		goto SetNote;
	case 0x8:
		DEBUG(3) puts("Addnote");
		a=channel->CurrNote;
		goto SetNote;
	case 0x9:
		DEBUG(3) puts("SetNote");
		a=0;
		SetNote:
		
		/*a=(notevals[a+x.b.b1&0x3F]*(0x100+channel->Finetune+(S8)x.b.b3))>>8;*/
		a = (notevals[(a+x.b.b1) & (0x3F)] * ( 0x100 + channel->Finetune + (S8)x.b.b3 )) >> 8;
		
		channel->DestPeriod=a;
		if (!channel->PortaRate) channel->CurPeriod=a;
		MAYBEWAIT;
	case 0x17: /* setperiod */
		DEBUG(3) puts("Setperiod");
		channel->DestPeriod=x.w.w1;
		if (!channel->PortaRate) channel->CurPeriod=x.w.w1;
		break;
	case 0xB: /* portamento FIXME: for R-Type (to high) */
		DEBUG(3) puts("Portamento");
		channel->PortaReset=x.b.b1;
		channel->PortaTime=1;
		if (!channel->PortaRate) channel->PortaPer=channel->DestPeriod;
		channel->PortaRate=x.w.w1;
		break;
	case 0xC: /* vibrato FIXME: X-Out loader, Apprentice (too fast) */
		DEBUG(3) puts("Vibrato");
		channel->VibTime=(channel->VibReset=x.b.b1)>>1;
		channel->VibWidth=x.b.b3;
		channel->VibFlag=1;
		if (!channel->PortaRate)
		{
			channel->CurPeriod=channel->DestPeriod;
			channel->VibOffset=0;
		}
		break;
	case 0xF: /* envelope */
		DEBUG(3) puts("Envelope");
		channel->EnvReset=channel->EnvTime=x.b.b2;
		channel->EnvEndvol=x.b.b3;
		channel->EnvRate=x.b.b1;
		break;
	case 0xA: /* reset */
		DEBUG(3) puts("Reset efx");
		channel->EnvReset=channel->VibReset=/*channel->ArpRun=channel->SIDSize=*/channel->PortaRate=
		channel->AddBeginTime=0;
		break;
	case 0x14: /* wait key up */
		DEBUG(3) puts("Wait key up");
		if (!channel->KeyUp) channel->Loop=0;
		if (!channel->Loop)
		{
			channel->Loop=-1;
			break;
		}
		if (channel->Loop==-1)
			channel->Loop=x.b.b3-1;
		else
			channel->Loop--;
		channel->MacroStep--;
		return;
	case 0x15: /* go sub */
		DEBUG(3) puts("Gosub patt");
		channel->ReturnPtr=channel->MacroPtr;
		channel->ReturnStep=channel->MacroStep;
	case 0x6: /* cont */
		DEBUG(3) puts("Continue");
		channel->MacroPtr=(channel->MacroNum=macros[x.b.b1]);
		channel->MacroStep=x.w.w1;
		channel->Loop=0xFFFF;
		break;
	case 0x16: /* return sub */
		DEBUG(3) puts("Returnpatt");
		channel->MacroPtr=channel->ReturnPtr;
		channel->MacroStep=channel->ReturnStep;
		break;
	case 0x18: /* sampleloop */
		DEBUG(3) puts("Sampleloop");
		channel->SaveAddr+=(x.w.w1&0xFFFE);
		channel->SaveLen-=x.w.w1>>1;
		channel->CurrLength=channel->SaveLen;
		channel->CurAddr=channel->SaveAddr;
		break;
	case 0x19: /* oneshot */
		DEBUG(3) puts("One-shot");
		channel->AddBeginTime=0;
		channel->SaveAddr=channel->CurAddr=0;
		channel->SaveLen=channel->CurrLength=1;
		break;
	case 0x20: /* cue */
		DEBUG(3) puts("Cue");
		idb.Cue[x.b.b1&0x03]=x.w.w1;
		break;
/*
TODO:
About macros 22-30 (as used in GemZ Title/Credits):
Not much is known about them, JHP wrote the following
stuff in his unofficial TFMX docs (regarding 22-29):

MacrSIDSampleMsg        dc.b    'SID setbeg  xxxxxx   sample-startadress',0
MacrSIDLengthMsg        dc.b    'SID setlen  xx/xxxx  buflen/sourcelen  ',0
MacrSID2OfsMsg          dc.b    'SID op3 ofs xxxxxx   offset            ',0
MacrSID2VibMsg          dc.b    'SID op3 frq xx/xxxx  speed/amplitude   ',0
MacrSID1OfsMsg          dc.b    'SID op2 ofs xxxxxx   offset            ',0
MacrSID1VibMsg          dc.b    'SID op2 frq xx/xxxx  speed/amplitude   ',0
MacrSIDFilterMsg        dc.b    'SID op1     xx/xx/xx speed/amplitude/TC',0
MacrSIDStopMsg          dc.b    'SID stop    xx....   flag (1=clear all)',0
*/
        case 0x22:
	        printf("TODO: SIDSampleMsg (0x22)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
		/* seems to work similar to 02... (not 100% sure, though) */
		channel->AddBeginTime=0;
		channel->CurAddr=x.l;
		break;
        case 0x23:
	        printf("TODO: SIDLengthMsg (0x23)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
		break;
        case 0x24:
	        printf("TODO: SID2OfsMsg (0x24)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
	        break;
        case 0x25:
	        printf("TODO: SID2VibMsg (0x25)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
	        break;
        case 0x26:
	        printf("TODO: SID1OfsMsg (0x26)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
	        break;
        case 0x27:
	        printf("TODO: SID1VibMsg (0x27)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
	        break;
        case 0x28:
	        printf("TODO: SIDFilterMsg (0x28)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
	        break;
        case 0x29:
	        printf("TODO: SIDStopMsg (0x29)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
	        break;
        case 0x30:
	        printf("TODO: ??? (0x30)\n");
                fprintf(stderr,"Found code %08x at step %04x in macro %02x",\
				x.l,channel->MacroStep-1,channel->MacroNum);
	        break;
	case 0x31: /* turrican 3 title - we can safely ignore */
		break;
	default:
		NOTSUPPORTED;
		break;
		channel->MacroRun=0;
		return;
	}
	goto loop;
}

void DoEffects(struct Channel *channel)
{
	//printf("*** DoEffects: Doing Effects\n");
	register int a=0;
	if (channel->EfxRun<0) return;
	if (!channel->EfxRun)
	{
		channel->EfxRun=1;
		return;
	}
	if (channel->AddBeginTime)
	{
		channel->CurAddr+=channel->AddBegin;
/*		if (channel->SIDSize)
			channel->SIDSrcSample=channel->CurAddr;
		else*/
			channel->SaveAddr=channel->CurAddr;
		channel->AddBeginTime--;
		if (!channel->AddBeginTime)
		{
			channel->AddBegin=-channel->AddBegin;
			channel->AddBeginTime=channel->AddBeginReset;
		}
	}
/*
	if (channel->SIDSize) {
		fputs("SID not supported\n",stderr);
		channel->SIDSize=0;
	}
*/
	if (channel->VibReset)
	{
		a=(channel->VibOffset+=channel->VibWidth);
		a=(channel->DestPeriod*(0x800+a))>>11;
		if (!channel->PortaRate) channel->CurPeriod=a;
		if (!(--channel->VibTime))
		{
			channel->VibTime=channel->VibReset;
			channel->VibWidth=-channel->VibWidth;
		}
	}
	if ((channel->PortaRate)&&((--channel->PortaTime)==0))
	{
		channel->PortaTime=channel->PortaReset;
		if (channel->PortaPer>channel->DestPeriod)
		{
			a=(channel->PortaPer*(256-channel->PortaRate)-128)>>8;
			if (a<=channel->DestPeriod)
				channel->PortaRate=0;
		}
		else if (channel->PortaPer<channel->DestPeriod)
		{
			a=(channel->PortaPer*(256+channel->PortaRate))>>8;
			if (a>=channel->DestPeriod)
				channel->PortaRate=0;
		}
		else channel->PortaRate=0;
		if (!channel->PortaRate)
			a=channel->DestPeriod;
		channel->PortaPer=channel->CurPeriod=a;
	}
	if ((channel->EnvReset)&&(!(channel->EnvTime--)))
	{
		channel->EnvTime=channel->EnvReset;
		if (channel->CurVol > channel->EnvEndvol)
		{
			if (channel->CurVol<channel->EnvRate) channel->EnvReset=0; else
			channel->CurVol -= channel->EnvRate;
			if (channel->EnvEndvol > channel->CurVol)
				channel->EnvReset=0;
		}
		else if (channel->CurVol < channel->EnvEndvol)
		{
			channel->CurVol += channel->EnvRate;
			if (channel->EnvEndvol < channel->CurVol)
				channel->EnvReset=0;
		}
		if (!channel->EnvReset)
		{
				channel->EnvReset=channel->EnvTime=0;
				channel->CurVol=channel->EnvEndvol;
		}
	}
/*	if (channel->ArpRun) {
		fputs("Arpeggio/randomplay not supported\n",stderr);
		channel->ArpRun=0;
	}
*/
	if ((trackManager.FadeSlope)&&((--trackManager.FadeTime)==0))
	{
		trackManager.FadeTime=trackManager.FadeReset;
		trackManager.MasterVol+=trackManager.FadeSlope;
		if (trackManager.FadeDest==trackManager.MasterVol) trackManager.FadeSlope=0;
	}
}

void DoMacro(int cc)
{
	//printf("*** DoMacro: Doing Macro\n");
	struct Channel *channel=&channelData[cc];

	int a;int nRun;int nWait;
/* locking */
	if (channel->SfxLockTime>=0)
		channel->SfxLockTime--;
	else
		channel->SfxFlag=channel->SfxPriority=0;
	
	a=channel->SfxCode;
	if (a)
	{
		channel->SfxFlag=channel->SfxCode=0;
		NotePort(a);
		channel->SfxFlag=channel->SfxPriority;
	}
	DEBUG(3) {
		printf("%01x:\t",cc);
	}
	
	/*if ((channel->MacroWait)&&(!(channel->MacroWait--)))*/
	
	/* FIXME with weird Z-Out theme,
	channel->MacroRun and channel->MacroWait differ sometimes from
	the correct values, when run on Mac OS X 
		
	channel->MacroRun: S8
	channel->MacroWait: U16
	*/
	nRun=channel->MacroRun;
	nWait=channel->MacroWait;
	channel->MacroWait=channel->MacroWait-1;
	
	DEBUG(3)
	{
		printf(" run:[%d] wait:[%d] ",nRun,nWait);
	}
	
	if ((nRun)&&(!(nWait)))
	{
		RunMacro(channel,cc);
	}
	else
	{
		DEBUG(3)
		{
			puts("_macro not run_");
		}
	}
	DEBUG(3) puts("");
	DoEffects(channel);
	/* has to be here because of if(efxrun=1) */
	channel->audio->delta=(channel->CurPeriod)?(3579545<<9)/(channel->CurPeriod*outRate>>5):0;
	channel->audio->SampleStart=&smplbuf[channel->SaveAddr];
	channel->audio->SampleLength=(channel->SaveLen)?channel->SaveLen<<1:65535;
	if ((channel->audio->mode&3)==1)
	{
		channel->audio->sbeg=channel->audio->SampleStart;
		channel->audio->slen=channel->audio->SampleLength;
	}
	channel->audio->vol=(channel->CurVol*trackManager.MasterVol)>>6;
}

void DoAllMacros()
{
	//puts("");
	//printf("*** DoAllMacros: Doing all Macros\n");
	DoMacro(0);
	DoMacro(1);
	DoMacro(2);
	if (multimode)
	{
		DoMacro(4);
		DoMacro(5);
		DoMacro(6);
		DoMacro(7);
	} /* else -- DoMacro(3) should always run so fade speed is right */
	DoMacro(3);
}

void ChannelOff(int i)
{
	//printf("*** ChannelOff: Channel Off\n");
	struct Channel *channel;
	channel=&channelData[i&0xF];
	if (!channel->SfxFlag)
	{
		channel->audio->mode=0;
		
		channel->AddBeginTime=channel->AddBeginReset=channel->MacroRun=/*channel->SIDSize=channel->ArpRun=*/0;

		channel->NewStyleMacro=0xFF;
		channel->SaveAddr=channel->CurVol=channel->audio->vol=0;
		channel->SaveLen=channel->CurrLength=1;
		channel->audio->loop=&LoopOff;
		channel->audio->channel=channel;
	}
}

void DoFade(int sp,int dv)
{
	//printf("*** DoFade: Doing Fade\n");
	trackManager.FadeDest=dv;
	if (!(trackManager.FadeTime=trackManager.FadeReset=sp)||(trackManager.MasterVol==sp))
	{
		trackManager.MasterVol=dv;
		trackManager.FadeSlope=0;
		return;
	}
	trackManager.FadeSlope=(trackManager.MasterVol>trackManager.FadeDest)?-1:1;
}

void GetTrackStep()
{
	//printf("*** GetTrackStep: Get Track Step\n");
	U16 *l;
	int x,y;
	loop:
	/* Fixed by Sven Janssen 15 August 2004 */
	if ((patternBlockData.CurrPos==patternBlockData.FirstPos) && (loops<=0))
	{
		if (loops<0)
		{
			trackManager.PlayerEnable=0;
			return;
		}
		loops--;
	}

	l=(U16 *)&editbuf[hdr.trackstart+(patternBlockData.CurrPos*4)];
	
	/* TEXT Display */
	puts("");
	printf("Track %04x: ",patternBlockData.CurrPos);
	for(x=0;x<8;x++)
	{
		printf("%04x ",l[x]);
	}
	printf("tempo=%d pre=%d jif=%d speedcnt=%d",0x1B51F8/trackManager.CIASave,patternBlockData.Prescale,jiffies, trackManager.SpeedCnt);
	puts("");
	puts("-----------------------------------------------------------------------------------------");

	jiffies=0;
	if ((l[0])==0xEFFE)
	{
		switch (l[1]) {
		case 0: /* stop */
			trackManager.PlayerEnable=0;
			return;
		case 1: /* loop */
			if (loops)
			{
				if (!(--loops))
				{
					trackManager.PlayerEnable=0;
					return;
				}
			}
			if (!(trackManager.TrackLoop--))
			{
				trackManager.TrackLoop=-1;
				patternBlockData.CurrPos++;
				goto loop;
			}
			else if (trackManager.TrackLoop<0)
				trackManager.TrackLoop=l[3];
			patternBlockData.CurrPos=l[2];
			goto loop;
		case 2: /* speed */ 
			trackManager.SpeedCnt=patternBlockData.Prescale=l[2];
			if (!(l[3]&0xF200)&&(x=(l[3]&0x1FF)>0xF))
				trackManager.CIASave=eClocks=0x1B51F8/x;
			patternBlockData.CurrPos++;
			goto loop;
		case 3: /* timeshare */
			if (!((x=l[3])&0x8000))
			{
				x=((char)x)<-0x20?-0x20:(char)x;
				trackManager.CIASave=eClocks=(14318*(x+100))/100;
				multimode=1;
			} /* else multimode=0;*/
			patternBlockData.CurrPos++;
			goto loop;
		case 4: /* fade */
			DoFade(l[2]&0xFF,l[3]&0xFF);
			patternBlockData.CurrPos++;
			goto loop;
		default:
			fprintf(stderr,"EFFE %04x in trackstep\n", l[1]);
			patternBlockData.CurrPos++;
			goto loop;
		}
	}
	else
	{
		for (x=0;x<8;x++)
		{
			patternBlockData.p[x].PXpose=(int)(l[x]&0xff);
			if ((y=patternBlockData.p[x].PNum=(l[x]>>8))<0x80)
			{
				patternBlockData.p[x].PStep=0;
				patternBlockData.p[x].PWait=0;
				patternBlockData.p[x].PLoop=0xFFFF;
				patternBlockData.p[x].PAddr=patterns[y];
			}
		}
	}
}

int DoTrack(struct Pattern *pattern)
{
	//printf("*** DoTrack: ---- ");
	const char *n1="CCDDEFFGGAAB",*n2=" # #  # # # ";
	static char n[]={0,0,0,0};
	UNI x;
	int y=0, t, z, zz;

	char *pattcmds[]={
	(char *)"End --Next track  step--",
	(char *)"Loop[count     / step.w]",
	(char *)"Cont[patternno./ step.w]",
	(char *)"Wait[count 00-FF--------",
	(char *)"Stop--Stop this pattern-",
	(char *)"Kup^-Set key up/channel]",
	(char *)"Vibr[speed     / rate.b]",
	(char *)"Enve[speed /endvolume.b]",
	(char *)"GsPt[patternno./ step.w]",
	(char *)"RoPt-Return old pattern-",
	(char *)"Fade[speed /endvolume.b]",
	(char *)"PPat[patt./track+transp]",
	(char *)"Lock---------ch./time.b]",
	(char *)"----------No entry------",
	(char *)"Stop-Stop custompattern-",
	(char *)"NOP!-no operation-------"
	};

	char *macrocmds[]={
	(char *)"DMAoff+Resetxx/xx/xx flag/addset/vol   ",
	(char *)"DMAon (start sample at selected begin) ",
	(char *)"SetBegin    xxxxxx   sample-startadress",
	(char *)"SetLen      ..xxxx   sample-length     ",
	(char *)"Wait        ..xxxx   count (VBI''s)     ",
	(char *)"Loop        xx/xxxx  count/step        ",
	(char *)"Cont        xx/xxxx  macro-number/step ",
	(char *)"-------------STOP----------------------",
	(char *)"AddNote     xx/xxxx  note/detune       ",
	(char *)"SetNote     xx/xxxx  note/detune       ",
	(char *)"Reset   Vibrato-Portamento-Envelope    ",
	(char *)"Portamento  xx/../xx count/speed       ",
	(char *)"Vibrato     xx/../xx speed/intensity   ",
	(char *)"AddVolume   ....xx   volume 00-3F      ",
	(char *)"SetVolume   ....xx   volume 00-3F      ",
	(char *)"Envelope    xx/xx/xx speed/count/endvol",
	(char *)"Loop key up xx/xxxx  count/step        ",
	(char *)"AddBegin    xx/xxxx  count/add to start",
	(char *)"AddLen      ..xxxx   add to sample-len ",
	(char *)"DMAoff stop sample but no clear        ",
	(char *)"Wait key up ....xx   count (VBI''s)     ",
	(char *)"Go submacro xx/xxxx  macro-number/step ",
	(char *)"--------Return to old macro------------",
	(char *)"Setperiod   ..xxxx   DMA period        ",
	(char *)"Sampleloop  ..xxxx   relative adress   ",
	(char *)"-------Set one shot sample-------------",
	(char *)"Wait on DMA ..xxxx   count (Wavecycles)",
	(char *)"Random play xx/xx/xx macro/speed/mode  ",
	(char *)"Splitkey    xx/xxxx  key/macrostep     ",
	(char *)"Splitvolume xx/xxxx  volume/macrostep  ",
	(char *)"Addvol+note xx/fe/xx note/CONST./volume",
	(char *)"SetPrevNote xx/xxxx  note/detune       ",
	(char *)"Signal      xx/xxxx  signalnumber/value",
	(char *)"Play macro  xx/.x/xx macro/chan/detune ",
	(char *)"SID setbeg  xxxxxx   sample-startadress",
	(char *)"SID setlen  xx/xxxx  buflen/sourcelen  ",
	(char *)"SID op3 ofs xxxxxx   offset            ",
	(char *)"SID op3 frq xx/xxxx  speed/amplitude   ",
	(char *)"SID op2 ofs xxxxxx   offset            ",
	(char *)"SID op2 frq xx/xxxx  speed/amplitude   ",
	(char *)"SID op1     xx/xx/xx speed/amplitude/TC",
	(char *)"SID stop    xx....   flag (1=clear all)"
	};
	
	if (pattern->PNum==0xFE)
	{
		pattern->PNum++;
		ChannelOff(pattern->PXpose);
		//printf("%04x: --- ---- -- | ", pattern->PNum);
		return(0);
	}
	if (!pattern->PAddr) {
		//printf("%04x: --- ---- -- | ", pattern->PNum);
		return(0);
	}
	if (pattern->PNum>=0x90) {
		//printf("%04x: --- ---- -- | ", pattern->PNum);
		return(0);
	}
	if (pattern->PWait--) {
		//printf("%04x: --- ---- -- | ", pattern->PNum);
		return(0);
	}

	while(1)
	{
		loop:
		x.l=ntohl(editbuf[pattern->PAddr+pattern->PStep++]);
		t=x.b.b0;

		//a.l=ntohl(editbuf[x++]);

		/*
		if (z<0x80)
		{
			//printf("%04x: %02x %s %02x %x %x %02x ---- ", y++,x.b.b0,n,x.b.b1,x.b.b2>>4,x.b.b2&0xF,x.b.b3);
			printf("04x: %02x ---- | ", y++, x.b.b0);
		}
		else if (z<0xC0)
		{
			//printf("%04x: %02x %s %02x %x %x %02x wait ---- ",y++,x.b.b0,n,x.b.b1,x.b.b2>>4,x.b.b2&0xF,x.b.b3);
			printf("04x: %02x wait | ", y++, x.b.b0);
		}
		else if (z<0xF0)
		{
			//printf("%04x: %02x %s %02x %x %x %02x porta ---- ",y++,x.b.b0,n,x.b.b1,x.b.b2>>4,x.b.b2&0xF,x.b.b3);
			printf("04x: %02x port | ", y++, x.b.b0);
		}
		else
		{
			//printf("%04x: %02x %s %02x %x %x %02x ---- ",y++,x.b.b0,pattcmds[z-0xF0],x.b.b1,x.b.b2>>4,x.b.b2&0xF,x.b.b3);
			printf("04x: %02x ---- | ", y++, x.b.b0);
		}
		*/

		if (t<0xF0)
		{
			fflush(stdout);
			if ((t&0xC0)==0x80)
			{
				pattern->PWait=x.b.b3;
				x.b.b3=0;
			}
			x.b.b0=((t+pattern->PXpose)&0x3F);
			
			if ((t&0xC0)==0xC0) {
				x.b.b0|=0xC0;
			}

			NotePort(x.l);
			
			zz=(t&0x3F)+6;
			n[2]=48+(zz/12);
			zz%=12;
			n[0]=n1[zz];
			n[1]=n2[zz];

			if ((t&0xC0)==0x80) {
				//printf("%04x: %s ---- -- | ", pattern->PNum, n);
				return(0);
			}
			goto loop;
		}


		switch (t&0xF)
		{
		case 15: /* NOP */
			//printf("%04x: --- NOP- 15 | ", pattern->PNum);
			break;
		case 0:	/* End */
			//printf("%04x: --- END- 00 | ", pattern->PNum);
			pattern->PNum=0xFF;
			patternBlockData.CurrPos=(patternBlockData.CurrPos==patternBlockData.LastPos)?
				    patternBlockData.FirstPos:patternBlockData.CurrPos+1;
			GetTrackStep();
			return(1);
		case 1:
			if (!(pattern->PLoop))
			{
				pattern->PLoop=0xFFFF;
				//printf("%04x: --- %04x 01 | ", pattern->PNum, t);
				break;
			}
			else if (pattern->PLoop==0xFFFF) /* FF --'ed */
			{
				pattern->PLoop=x.b.b1;
			}
			pattern->PLoop--;
			pattern->PStep=x.w.w1;
			break;
		case 8: /* GsPt */
			pattern->PRoAddr=pattern->PAddr;
			pattern->PRoStep=pattern->PStep;
			/* fall through to... */
		case 2: /* Cont */
			//printf("%04x: --- %04x 02 | ", pattern->PNum, t);
			pattern->PAddr=patterns[x.b.b1];
			pattern->PStep=x.w.w1;
			break;
		case 3: /* Wait */
			//printf("%04x: --- Wait 03 | ", pattern->PNum);
			pattern->PWait=x.b.b1;
			return(0);
		case 14: /* StCu */
			trackManager.PlayPattFlag=0;
		case 4: /* Stop */
			//printf("%04x: --- Break 04\n", pattern->PNum);
			pattern->PNum=0xFF;
			return(0);
		case 5: /* Kup^ */
		case 6: /* Vibr */
		case 7: /* Enve */
		case 12: /* Lock */
			//printf("%04x: --- %04x 12 | ", pattern->PNum, t);
			NotePort(x.l);
			break;
		case 9: /* RoPt */
			//printf("%04x: --- RoPt 09 | ", pattern->PNum);
			pattern->PAddr=pattern->PRoAddr;
			pattern->PStep=pattern->PRoStep;
			break;
		case 10: /* Fade */
			//printf("%04x: --- %04x 10 | ", pattern->PNum, t);
			DoFade(x.b.b1,x.b.b3);
			break;
		case 13: /* Cue */
			//printf("%04x: --- %04x 13 | ", pattern->PNum, t);
			idb.Cue[x.b.b1&0x03]=x.w.w1;
			break;
		case 11: /* PPat */
			//printf("%04x: --- %04x 11 | ", pattern->PNum, t);
			t=x.b.b2&0x07;
			patternBlockData.p[t].PNum=x.b.b1;
			patternBlockData.p[t].PAddr=patterns[x.b.b1];
			patternBlockData.p[t].PXpose=x.b.b3;
			patternBlockData.p[t].PStep=0;
			patternBlockData.p[t].PWait=0;
			patternBlockData.p[t].PLoop=0xFFFF;
			break;
		}
	}
}

void DoTracks() {
	int x;
	jiffies++;
	
	if (!trackManager.SpeedCnt--) {
		trackManager.SpeedCnt=patternBlockData.Prescale;

		/* sortof fix Oops Up tempo */
		if (oopsUpHack) {
		        trackManager.SpeedCnt=5;
		}

		for (x=0;x<8;x++) {
			if ( DoTrack(&patternBlockData.p[x]) ) {
				x=-1;
				continue;
			}
		}
	}
}

void tfmxIrqIn() {
	if (!trackManager.PlayerEnable) {
		return;
	}

	DoAllMacros();
	if (trackManager.CurrSong>=0) {
		DoTracks();
	}
}

void AllOff()
{
	printf("*** AllOff: All Off\n");
	int x;
	struct Channel *channel;
	trackManager.PlayerEnable=0;
	for (x=0;x<8;x++) {
		channel=&channelData[x];
		channel->audio=&audioData[x];
		channel->audio->channel=channel;	/* wait on dma */
		audioData[x].mode=0;
		
		channel->MacroWait=channel->MacroRun=channel->SfxFlag=/*channel->SIDSize=channel->ArpRun=*/channel->CurVol=
			channel->SfxFlag=channel->SfxCode=channel->SaveAddr=0;

		audioData[x].vol=0;
		channel->Loop=channel->NewStyleMacro=channel->SfxLockTime=-1;
		channel->audio->sbeg=channel->audio->SampleStart=smplbuf;
		channel->audio->SampleLength=channel->audio->slen=channel->SaveLen=2;
		channel->audio->loop=&LoopOff;
	}
}

void TfmxInit()
{
	printf("*** TfmxInit: TFMX Init\n");
	int x;
	AllOff();
	for (x=0;x<8;x++) {
		audioData[x].channel=&channelData[x];
		patternBlockData.p[x].PNum=0xFF;
		patternBlockData.p[x].PAddr=0;
		ChannelOff(x);
	}
	return;
}

void StartSong(int song, int mode)
{
	printf("*** StartSong: Starting Song\n");
	int x;
	trackManager.PlayerEnable=0; /* sort of locking mechanism */
	trackManager.MasterVol=0x40;
	trackManager.FadeSlope=0;
	trackManager.TrackLoop=-1;
	trackManager.PlayPattFlag=0;
	trackManager.CIASave=eClocks=14318; /* assume 125bpm, NTSC timing */
	if (mode!=2) {
		patternBlockData.CurrPos=patternBlockData.FirstPos=hdr.start[song];
		patternBlockData.LastPos=hdr.end[song];
		if ((x=hdr.tempo[song])>=0x10)
		{
		        trackManager.CIASave=eClocks=0x1B51F8/x;
		        patternBlockData.Prescale=0;
		}
		else
		        patternBlockData.Prescale=x;
	}
	for (x=0;x<8;x++) {
		patternBlockData.p[x].PAddr=0;
		patternBlockData.p[x].PNum=0xFF;
		patternBlockData.p[x].PXpose=0;
		patternBlockData.p[x].PStep=0;
	}
	if (mode!=2) GetTrackStep();
	if (startPat!=-1) {
		patternBlockData.CurrPos=patternBlockData.FirstPos=startPat;
		GetTrackStep();
		startPat=-1;
	}
	trackManager.SpeedCnt=trackManager.EndFlag=0;
	trackManager.PlayerEnable=1;
}
