#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "SDL.h"
#include "player.h"

void tfmxIrqIn();

char act[8]={1,1,1,1,1,1,1,1};

extern int toOutFile;
extern struct Audio audioData[8];
extern struct Channel channelData[8];
extern struct TrackManager trackManager;

/* we have to make HALFBUFSIZE really 1/2 of BUFSIZE now,
so we can use the maximum fragment size for SDL (...choppy sound under
heavy CPU load otherwise...) */
#define HALFBUFSIZE (65536 * 4)
#define BUFSIZE (131072 * 4)

union {
 S16 b16[BUFSIZE/2];
 U8 b8[BUFSIZE];
} buf;

volatile int bhead=0,btail=0;

S32 tbuf[HALFBUFSIZE*2];

extern int jiffies;
int bytes=0,bytes2=0;

U32 blocksize=0,multiplier=1,stereo=0;

int sndhdl=0;
int force8=0;
int isfile=0;
int eRem=0; /* remainder of eclocks */
int blend=1; /* default to blended mode */
int filt=1; /* light lpf */
int over=0;

pthread_mutex_t lock;
pthread_cond_t cond;

void fill_audio(void *udata, Uint8 *stream, int len);
void filter(S32 *b, int num);
void stereoblend(S32 *b,int num);
void conv_u8(S32 *b,int num); 
void conv_s16(S32 *b,int num);
void mix_add_ov(struct Audio *audio,int n,S32 *b);
void mix_add(struct Audio *audio,int n,S32 *b);
void mixit(int n,int b);  
void mixem(U32 nb,U32 bd);
void open_sndfile(void);
void open_snddev(void); 
int try_to_output(void);
int play_it(void);
void TfmxTakedown(void);   
int try_to_makeblock(void);
void tfmxIrqIn(void);

static int available_sound_data() {

    int l = bhead - btail + BUFSIZE;
    l %= BUFSIZE;

	//printf("available_sound_data: Available data = %d\n", l);
    return l;
}

/* Simple little three-position weighted-sum LPF. */

void filter(S32 *b, int num)
{
	//printf("*** filter: filter\n");
	register int x;
	static int wl=0,wr=0; /* actually backwards but who cares? */
	switch(filt)
	{
	case 3:
		for (x=0;x<num;x++)
		{
			wl=((b[HALFBUFSIZE])+wl*3)>>2; b[HALFBUFSIZE]=wl;
			wr=((*b)+wr*3)>>2; *b++=wr;
		}
		break;
	case 2:
		for (x=0;x<num;x++)
		{
			wl=((b[HALFBUFSIZE])+wl)>>1; b[HALFBUFSIZE]=wl;
			wr=((*b)+wr)>>1; *b++=wr;
		}
		break;
	case 1:
		for (x=0;x<num;x++)
		{
			wl=((b[HALFBUFSIZE])*3+wl)>>2; b[HALFBUFSIZE]=wl;
			wr=((*b)*3+wr)>>2; *b++=wr;
		}
		break;
	}
}

/* This one looks like a good candidate for high optimization... */

void stereoblend(S32 *b,int num) {
	//printf("*** stereoblend: stereoblend\n");
	if (blend) {
		int x;
		for (x=0;x<num;x++) {
			register int y;
			y=((b[HALFBUFSIZE]*11)+((*b)*5))>>4;
			b[0]=((b[HALFBUFSIZE]*5)+((*b)*11))>>4;
			b[HALFBUFSIZE]=y;
			b++;
		}
	}
}

void conv_u8(S32 *b,int num)
{
	//printf("*** conv_u8: conv_u8\n");
	int x;
	S32 *channel=b;
	U8 *a=(U8 *)&buf.b8[bhead];

    // there should always be enough space for conversion since buffer is only
    // filled half so abort in this case. We could wait here instead.
    if ( available_sound_data() + ( num * multiplier ) >= BUFSIZE ) {
        abort();
    }
    
	filter(b,num);
	stereoblend(b,num);

	if (stereo)
	{
		for (x=0;x<num;x++)
		{
                        *a++ = ((b[HALFBUFSIZE])>>8) ^ 0x80;
                        *a++ = ((*b++)>>8) ^ 0x80;
		}
	}
	else
	{
		for (x=0;x<num;x++)
		{
/* reverted, the new version probably broke something */
			// *a++ = (( b[HALFBUFSIZE] + *b++ )>>9) ^ 0x80;
			*a++ = b[HALFBUFSIZE];
			*a = *a + *b++;
			*a = *a >>9;
			*a = *a ^ 0x80;
		}
	}
	bytes2+=num;
	for(x=0;x<num;x++)
	{
		channel[HALFBUFSIZE]=0;
		*channel++=0;
	}

	bhead = ( bhead + ( num * multiplier ) ) % BUFSIZE;
}

void conv_s16(S32 *b,int num)
{
	//printf("*** conv_s16: conv_s16\n");
	int x;
	S32 *channel=b;
	S16 *a=(S16 *)&buf.b8[bhead];

    // there should always be enough space for conversion since buffer is only
    // filled half so abort in this case. We could wait here instead.
    if ( available_sound_data() + ( num * multiplier ) >= BUFSIZE ) {
        abort();
    }
    
	filter(b,num);
	stereoblend(b,num);

	if (stereo)
	{
		for (x=0;x<num;x++)
		{
                        *a++=(b[HALFBUFSIZE]);
                        *a++=(*b++);
		}
	}
	else
	{
		for (x=0;x<num;x++)
		{
/* reverted, the new version broke -b0 */
			// *a++=(b[HALFBUFSIZE]+*b++)>>1;
			*a++ = b[HALFBUFSIZE];
			*a = *a + *b++;
			*a = *a >> 1;
		}
	}
	bytes2+=num;
	for(x=0;x<num;x++)
	{
		channel[HALFBUFSIZE]=0;
		*channel++=0;
	}

	bhead = ( bhead + ( num * multiplier ) ) % BUFSIZE;
}

void (*conv)(S32 *,int)=&conv_s16;

static int nul=0;
void (*mix)(struct Audio *, int, S32 *);

void mix_add(struct Audio *audio, int n, S32 *b) {
	//printf("*** mix_add: mix_add\n");
	register S8 * p = audio->sbeg;
	register U32 ps=audio->pos;
	int v=audio->vol;
	U32 d=audio->delta;
	U32 l=(audio->slen<<14);

	if (v>0x40)v=0x40;

/* This used to have (p==&smplbuf).  Broke with GrandMonsterSlam */
	if ((p==(S8 *)&nul)||( ((audio->mode)&1)==0 )||(l<0x10000))
		return;
	if ((audio->mode&3)==1)
	{
		p=audio->sbeg=audio->SampleStart;
		l=(audio->slen=audio->SampleLength)<<14;
		ps=0;
		audio->mode|=2;
/*		audio->loop(&audio);*/
	}
	if (!v)
	{
#if 0		/* Will be supported someday... */
		while(n--){
			(*b++)+=(p[(ps+=d)>>14]*v);
			if (ps<l) continue;
			ps-=l;
			p=audio->SampleStart;
			if (((l=audio->SampleLength<<14)<=0x10000) ||
			    (!audio->loop(audio)) )
					{
				ps=l=d=0;
				p=smplbuf;
				break;
			}
		}
		return;
#endif
	}
	while(n--){
		(*b++)+=(p[(ps+=d)>>14]*v);
		if (ps<l) continue;
		ps-=l;
		p=audio->SampleStart;
		if ( ((l=((audio->slen=audio->SampleLength)<<14))<0x10000) ||
		     (!audio->loop(audio)) )
				 {
			audio->slen=ps=d=0;
			p=smplbuf;
			break;
		}
	}
	audio->sbeg=p;
	audio->pos=ps;
	audio->delta=d;
	if (audio->mode&4) (audio->mode=0);
}

void mix_add_ov(struct Audio *audio,int n,S32 *b)
{
	//printf("*** mix_add_ov: mix_add_ov\n");
	register S8 * p = audio->sbeg;
	register U32 ps=audio->pos;
	register U32 psreal;
	int v=audio->vol;
	U32 d=audio->delta;
	U32 l=(audio->slen<<14);

	int v1;
	int v2;

	if (v>0x40)v=0x40;

/* This used to have (p==&smplbuf).  Broke with GrandMonsterSlam */
	if ((p==(S8 *)&nul)||( ((audio->mode)&1)==0 )||(l<0x10000))
		return;
	if ((audio->mode&3)==1)
	{
		p=audio->sbeg=audio->SampleStart;
		l=(audio->slen=audio->SampleLength)<<14;
		ps=0;
		audio->mode|=2;
	/*	audio->loop(&audio); */
	}
	if (!v)
	{
#if 0		/* Will be supported someday... */
		while(n--){
			(*b++)+=(p[(ps+=d)>>14]*v);
			if (ps<l) continue;
			ps-=l;
			p=audio->SampleStart;
			if (((l=audio->SampleLength<<14)<=0x10000) ||
			    (!audio->loop(audio)) )
					{
				ps=l=d=0;
				p=smplbuf;
				break;
			}
		}
		return;
#endif
	}
/*
#   define RESAMPLATION \
      v1=src[ofs>>FRACTION_BITS];\
      v2=src[(ofs>>FRACTION_BITS)+1];\
      *dest++ = v1 + (((v2-v1) * (ofs & FRACTION_MASK)) >> FRACTION_BITS);

*/
#define FRACTION_BITS 14
#define INTEGER_MASK (0xFFFFFFFF << FRACTION_BITS)
#define FRACTION_MASK (~ INTEGER_MASK)

	while(n--){
		/*
		   register short oo=(ps&0x3FFF);
		   q=((p[(ps >> 14)+1])*(16384-oo));
		   (*b++)+=((p[((ps+=d)>>14)])*oo+q)*v>>14; 
		   */

		/*
		(*b++)+=(p[ps>>14]*v);
	        */
		psreal = ps>>FRACTION_BITS;
		v1 = p[psreal];
		if (psreal+1 < audio->slen)
		{
			v2 = p[psreal+1];
		}
		else
		{
			v2 = audio->SampleStart[0];
			/* fprintf(stderr, "H"); */
			/* (*b++) += v*v1; */
		}
		(*b++) += v*((v1 +
			      (((signed) ((v2-v1) * (ps & FRACTION_MASK)))
			       >> FRACTION_BITS)));
		ps += d;

		if (ps<l) continue;
		ps-=l;
		p=audio->SampleStart;
		if ( ((l=((audio->slen=audio->SampleLength)<<14))<0x10000) ||
		     (!audio->loop(audio)) )
				 {
			audio->slen=ps=d=0;
			p=smplbuf;
			break;
		}
	}
	audio->sbeg=p;
	audio->pos=ps;
	audio->delta=d;

	if (audio->mode&4) {
		(audio->mode=0);
	}
}
	
void (*mix)(struct Audio *,int,S32 *)=&mix_add;

void mixit(int n,int b)
{
	int x;
	S32 *y;
	if (multimode) {
		if(act[4])mix(&audioData[4],n,&tbuf[b]);
		if(act[5])mix(&audioData[5],n,&tbuf[b]);
		if(act[6])mix(&audioData[6],n,&tbuf[b]);
		if(act[7])mix(&audioData[7],n,&tbuf[b]);
		y=&tbuf[HALFBUFSIZE+b];
		for (x=0;x<n;x++,y++)
			*y=(*y>16383)?16383:
			   (*y<-16383)?-16383:*y;
	} else {
		if(act[3])mix(&audioData[3],n,&tbuf[b]);
	}
	if(act[0])mix(&audioData[0],n,&tbuf[b]);
	if(act[1])mix(&audioData[1],n,&tbuf[HALFBUFSIZE+b]);
	if(act[2])mix(&audioData[2],n,&tbuf[HALFBUFSIZE+b]);
}

void mixem(U32 nb,U32 bd)
{
	if (over==-1) {
		mix=&mix_add_ov;
	 } else {
		mix=&mix_add;
	 }
	mixit(nb,bd);
}

void open_snddev()
{
	printf("*** open_snddev: open_snddev\n");

    SDL_AudioSpec wanted;
	
	multiplier=2;

	if (force8) {
		conv=&conv_u8;
	}

	blocksize=HALFBUFSIZE;

	/* SDL open device here */

	/* Set the audio format */
	wanted.freq = outRate;

#ifdef WORDS_BIGENDIAN
	wanted.format = (force8?AUDIO_U8:AUDIO_S16MSB);
#else
	wanted.format = (force8?AUDIO_U8:AUDIO_S16LSB);
#endif
	wanted.channels = (stereo?2:1);
	wanted.samples = (force8?4096:8192); /* as big as it gets */
	wanted.callback = fill_audio;
	wanted.userdata = NULL;

	if ( SDL_OpenAudio(&wanted, NULL) < 0 )
	{
	    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	    /* */
		_exit(-1);
	}
	SDL_PauseAudio(0);

	multiplier*=(stereo?2:1);
	multiplier/=(force8?2:1);

	if (stereo) {
		blocksize=blocksize/multiplier/2;
	} else {
		blocksize=blocksize/multiplier/4;
	}

	if (blocksize>HALFBUFSIZE) {
		fprintf(stderr,"Block size %d not supported",blocksize);
		_exit(1);
	}
	return;
}

void open_sndfile()
{
	//printf("*** open_sndfile: open_sndfile\n");
/*	int x=0;*/
	multiplier=2;

	if (force8) conv=&conv_u8;
	
/* FIXME: I hope this is correct*/
#ifdef WORDS_BIGENDIAN
	outRate*=(force8?1:2);
#endif

	blocksize=HALFBUFSIZE;

	if ((sndhdl=open(outf,O_WRONLY|O_CREAT|O_TRUNC,0644))<0)
	{		
		perror("open");
		_exit(1);
	}

	multiplier*=(stereo?2:1);
	multiplier/=(force8?2:1);

        if (stereo)
		blocksize=blocksize/multiplier/2;
        else
	        blocksize=blocksize/multiplier/4;

	if (blocksize>HALFBUFSIZE)
	{
		fprintf(stderr,"Block size %d not supported",blocksize);
		close(sndhdl);
		_exit(1);
	}
	return;
}


void TfmxTakedown()
{
	//printf("*** TfmxTakedown: TfmxTakedown\n");
	if (toOutFile==1)
	{
		close(sndhdl);
	}
	free(smplbuf);

	if (toOutFile==0)
	{
		SDL_CloseAudio();
		SDL_Quit();
	}
}


int try_to_makeblock()
{
	printf("*** try_to_makeblock: try_to_makeblock\n");
	static S32 nb = 0, bd = 0; /* num bytes, bytes done */
	int n, r = 0;
    int loops = 0;

	// Buffer Threshold
	static int threshold = BUFSIZE / 2;

    while ( available_sound_data() < threshold && trackManager.PlayerEnable ) {

		//printf("While: Available sound data = %d (threshold: %d), Player Enable Status = %d\n", available_sound_data(), threshold, trackManager.PlayerEnable);

        // Increment the loop counter
		loops++;

		// Call a function to handle an interrupt or periodic update in the context of TFMX playback
		tfmxIrqIn();

		// Calculate the number of eClocks for half the output rate
		nb = (eClocks * (outRate >> 1));

		// Add the remainder of nb divided by 357955 to eRem
		eRem += (nb % 357955);

		// Divide nb by 357955 to get a normalized value
		nb /= 357955;

		// Check if eRem exceeded 357955, adjust nb and eRem accordingly
		if (eRem > 357955) {
			nb++;
			eRem -= 357955;
		}

		// Process blocks of audio data until all nb blocks are processed
		while (nb > 0) {
			// Calculate the number of blocks to process in this iteration
			n = blocksize - bd;
			if (n > nb) {
				n = nb;
			}

			// Mix the specified number of audio blocks
			mixem(n, bd);
			printf("mixem: Mixed %d bytes at position %d\n", n, bd);

			// Update the total number of bytes processed and the buffer position
			bytes += n;
			bd += n;
			nb -= n;

			// Check if the buffer is full or the player is disabled
			if (((unsigned int)bd) == blocksize || !trackManager.PlayerEnable) {

				// Convert the processed audio data for output
				conv(&tbuf[0], bd);
				printf("Converted audio data of %u bytes\n", bd);

				// Reset the buffer position for the next batch of audio data
				bd = 0;

				// Increment the counter for the number of times the buffer has been processed
				r++;
			}
		}
	}

    if ( ! loops && toOutFile == 0 ) {

		printf("makeblock toOutFile\n");

        pthread_mutex_lock( &lock );
        if ( available_sound_data() >= BUFSIZE / 2 ) {
            pthread_cond_wait( &cond, &lock );
        }
        pthread_mutex_unlock( &lock );
    }
    
	return((trackManager.PlayerEnable)?r:-1);
}

// Callback function to fill the audio buffer for playback
void fill_audio(void *udata, Uint8 *stream, int len) {
    // printf("*** fill_audio: fill_audio\n");  // Logging statement for debugging

    // Get the amount of available sound data
    int avail = available_sound_data();

    // If available data is less than requested length, fill the remainder with silence
    if (avail < len) {
        SDL_memset(stream + avail, 0, len - avail);  // Fill with zeros (silence)
        len = avail;  // Adjust the length to the available data
    }

    // Initialize variables to track the total length to write and the amount written
    int total_len = len;
    int written = 0;

    // Loop to handle audio data writing, especially when reaching the ring buffer boundary
    while (total_len > 0) {
        // If adding len to btail exceeds the buffer size, adjust len to fit the buffer
        if (btail + len > BUFSIZE) {
            len = BUFSIZE - btail;
        }

        // Mix the audio data from the buffer into the stream
        SDL_MixAudio(stream + written, &buf.b8[btail], len, SDL_MIX_MAXVOLUME);
		printf("fill_audio: Mixed %d bytes from buffer position %d\n", len, btail);

        // Update the buffer tail position, wrapping around if necessary
        btail = (btail + len) % BUFSIZE;
        // Update the number of bytes written
        written += len;

        // Decrease the remaining length to be written
        total_len -= len;

        // Set len to the remaining length for the next iteration
        len = total_len;
    }
    
    // Note: udata is not used in this function, but it's required by SDL's callback interface

    // Signal a condition variable (used for synchronization with other threads)
    pthread_cond_signal(&cond);
}

int write_output()
{
	//printf("*** write_output: write_output\n");
	int x;
	//int n=blocksize*multiplier;

	if (toOutFile==0) {
        return 0;
    }

    int total_len = available_sound_data();
    int len;
    
    while ( total_len > 0 ) {
        len = total_len;
        if ( btail + len > BUFSIZE ) {
            len = BUFSIZE - btail;
        }

		x = write( sndhdl, &buf.b8[btail], len );

        if ( x <= 0 ) {
            perror("write");
            close(sndhdl);
            _exit(1);
        }

        btail = ( btail + len ) % BUFSIZE;

        total_len -= x;
	}
	
	/* did not have any return value */
	return 1;
}

int play_it() {

    // Initialize a mutex and a condition variable
    pthread_mutex_init( &lock, NULL );
    pthread_cond_init( &cond, NULL );

    // Print a message before calling the function try_to_makeblock()
    printf("BEFORE: try_to_makeblock\n");

    // Call the function try_to_makeblock()
    try_to_makeblock();

    // Print a message after calling the function try_to_makeblock()
    printf("AFTER: try_to_makeblock\n");

    // This section of code is commented out, so it won't execute:
    // while (try_to_makeblock());
    // while (try_to_makeblock()>=0)
    // {
    //     write_output();
    // }

    // If the variable toOutFile is equal to 0, execute the following block:
    if (toOutFile == 0) {
        // While there is available sound data, delay the program by 25 milliseconds
        while (available_sound_data() > 0) {
            SDL_Delay(25);
        }
    }

    // Destroy the previously initialized mutex and condition variable
    pthread_mutex_destroy( &lock );
    pthread_cond_destroy( &cond );

    // Return 0 to indicate successful execution
    return (0);
}

