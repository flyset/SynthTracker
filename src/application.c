#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "SDL.h"
#include "application.h"
#include "tfmxsong.h"
#include "player.h"
#include "audio.h"

void open_sndfile();
void open_snddev();
void TfmxInit();
void StartSong(int song, int mode);
void play_it();
void TfmxTakedown();
int load_tfmx(char *mfn, char *sfn);
void do_debug(void);

extern int singleFile, dosExt, toOutFile, printinfo, songnum, gubed, export;
extern int startPat, gemx, loops, dangerFreakHack, force8, blend, filt, over;
extern int monkeyHack;
extern char outf[PATHNAME_LENGTH], act[8];
extern U32 outRate;
extern struct Header hdr;
extern struct Audio audioData[8];
extern S8 *smplbuf;
extern int num_ts, num_pat, num_mac;
extern int LoopOff(struct Audio *audio);

static void usage(char *x)
{
    fprintf(stderr,
        "SynthTracker v1.1.7/SDL by Jon Pickard <marxmarv@antigates.com>,\n"
        "Neochrome <neko@netcologne.de> and others.\n"
        "Copyright 1996-2004, see accompanying README for details.\n\n"
        "Usage: %s [options] mdat-file [smpl-file]\n"
        "where options is one or more of:\n"
        "-b mode\t\tset stereo mode (0=mono, default 1=headphone, 2=stereo)\n"
        "-8\t\tgenerate 8-bit output\n-p num\t\tsubsong to play (default 0)\n"
        "-f freq\t\tsuggest playback rate in samples/sec (default 44100)\n"
        "-o file\t\twrite audio output to file\n-i\t\tprint info about the module (text, subsong, etc.)\n"
        "-w num\t\tset low-pass filter frequency (0=none, 3=lowest, default 0)\n"
        "-l num\t\tset loop mode (0=no repeat, default 1=infinite)\n"
        "-v              disable oversampling (=linear interpolation)\n"
        "-D              force hack for Danger Freak title tune\n"
        "-G              force old hack for GemX title tune (still incomplete)\n"
        "-x              export to XRNS XML\n-~              debug mode (commands pp and pm)\n", x);
}

int application_run(int argc, char **argv)
{
    char *tfxloc=0, *channel=0;
    int x;
    char mfn[PATHNAME_LENGTH], sfn[PATHNAME_LENGTH];

    over=-1;
    filt=0;
    while ((x=getopt(argc,argv,"~xGDivSb:8o:f:P:V:p:w:l:"))!=-1) {
        switch (x) {
        case '?': case ':': usage(argv[0]); return 2;
        case 'o': strncpy(outf,optarg,PATHNAME_LENGTH-1); outf[PATHNAME_LENGTH-1]='\0'; toOutFile=1; break;
        case 'P': startPat=strtol(optarg,NULL,0); break;
        case 'f': outRate=strtol(optarg,NULL,0); break;
        case 'b': blend=strtol(optarg,NULL,0); break;
        case 'p': songnum=strtol(optarg,NULL,0); break;
        case 'w': filt=strtol(optarg,NULL,0); break;
        case 'l': loops=strtol(optarg,NULL,0); break;
        case 'v': over=0; break;
        case 'G': gemx=1; break;
        case 'D': dangerFreakHack=1; break;
        case 'S': break;
        case 'i': printinfo=1; break;
        case 'V': channel=optarg; for(;*channel;act[(*channel++)&7]=0) {} break;
        case '8': force8=1; break;
        case 'x': export=1; break;
        case '~': gubed=1; break;
        default: fprintf(stderr,"getopt: got code 0x%x\n",x);
        }
    }
    if (optind<argc) {
        strncpy(mfn,argv[optind++],PATHNAME_LENGTH-1); mfn[PATHNAME_LENGTH-1]='\0';
        strncpy(sfn,"\0",1);
        if (optind<argc) { strncpy(sfn,argv[optind++],PATHNAME_LENGTH-1); sfn[PATHNAME_LENGTH-1]='\0'; printf("IF\n"); }
        else {
            strncpy(sfn,mfn,PATHNAME_LENGTH-1); sfn[PATHNAME_LENGTH-1]='\0';
            if (!(channel=strrchr(sfn,'/'))) channel=sfn; else channel++;
            tfxloc=strchr(channel,'\0');
            if ((tfxloc-4)>channel) { tfxloc-=4; if (!strncasecmp(tfxloc,".tfx",4)) dosExt=1; }
            if (dosExt!=1) {
                if (strncasecmp(channel,"mdat.",5)) {
                    if (strncasecmp(channel,"tfmx.",5)) puts("'mdat'/'tfmx' prefix missing\n");
                    else { singleFile=1; sfn[0]='\0'; }
                }
                if (!singleFile) { (*channel++)^='m'^'s'; (*channel++)^='d'^'m'; (*channel++)^='a'^'p'; (*channel++)^='t'^'l'; channel-=4; }
            } else { tfxloc++; (*tfxloc++)^='t'^'s'; (*tfxloc++)^='f'^'a'; (*tfxloc++)^='x'^'m'; tfxloc-=4; }
        }
    } else { usage(argv[0]); return 2; }

    if ((x=load_tfmx(mfn,sfn))==1) { fprintf(stderr,"%s: load_tfmx failed\n",argv[0]); exit(1); }
    else if (x==2) { fprintf(stderr,"%s: Not an MDAT/TFMX file\n",channel); exit(1); }
    if (blend) stereo=1; blend&=1;
    if (!(channel=strrchr(mfn,'/'))) channel=mfn; else channel++;
    printf("Module: %s\n",channel);
    if (printinfo) {
        for (x=0;x<6;x++) printf(">%40.40s\n",hdr.text[x]); puts("");
        printf("%d tracksteps at 0x%04x\n",num_ts,(hdr.trackstart<<2)+0x200);
        printf("%d patterns at 0x%04x\n",num_pat,(hdr.pattstart<<2)+0x200);
        printf("%d macros at 0x%04x\n",num_mac,(hdr.macrostart<<2)+0x200);
        for (x=0;x<31;x++) if (hdr.end[x]) printf("Song %2d: start %3x end %3x\n",x,ntohs(hdr.start[x]),ntohs(hdr.end[x]));
    }
    if (gubed) { do_debug(); exit(0); }
    if (export) exit(0);
    if (monkeyHack==1) printf("MONKEY ISLAND DETECTED\n");
    if (toOutFile==1) open_sndfile(); else open_snddev();
    TfmxInit(); StartSong(songnum,0);
    audioData[0]=(struct Audio){0,0x1C01,0x3200,0x15BE,&smplbuf[0x4],&smplbuf[0x4+0x1C42],0x40,3,&LoopOff,0,NULL};
    signal(SIGINT, inthand);
    play_it();
    TfmxTakedown();
    return 0;
}
