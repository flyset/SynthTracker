#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "SDL.h"
#include "player.h"
#include "audio.h"
#include "audio_output.h"

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

#define AUDIO_BUFFER_SIZE_SMALL 4096
#define AUDIO_BUFFER_SIZE_LARGE 8192
#define AUDIO_OUTPUT_WORKSPACE_CAPACITY 65536

#ifdef WORDS_BIGENDIAN
#define AUDIO_FORMAT_8BIT AUDIO_U8
#define AUDIO_FORMAT_16BIT AUDIO_S16MSB
#else
#define AUDIO_FORMAT_8BIT AUDIO_U8
#define AUDIO_FORMAT_16BIT AUDIO_S16LSB
#endif

#define MULTIPLIER_DEFAULT_VALUE 2

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
void mixit(int audio_samples, int buf_position);  
void mixem(U32 audio_samples,U32 buf_position);
void open_sndfile(void);
void open_snddev(void); 
int try_to_output(void);
int play_it(void);
void TfmxTakedown(void);   
int try_to_makeblock(void);

S32 calculateSamplesToProcess(void);
void processAudioData(S32* num_samples_to_process, S32* buf_position, int* buf_proc_counter, int* audio_samples);
void checkThreadSync(int loops);

// Function prototypes
void initSyncPrimitives(void);
void processAudio(void);
void finalizeAudioOutput(void);
void waitForRemainingAudioData(void);
void cleanupSyncPrimitives(void);

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

static audio_frame audio_output_workspace[AUDIO_OUTPUT_WORKSPACE_CAPACITY];
static audio_output_null_adapter audio_output_adapter;

void mix_add(struct Audio *audio, int n, S32 *b) {
    if (audio->vol > 0x40) {
        audio->vol = 0x40;
    }

    if (audio->sbeg == (S8 *)&nul || ((audio->mode & 1) == 0) || (audio->slen == 0)) {
        return;
    }

    if ((audio->mode & 3) == 1) {
        audio->sbeg = audio->SampleStart;
        audio->slen = audio->SampleLength;
        audio->pos = 0;
        audio->mode |= 2;
    }

    if (audio->vol == 0) {
        return;
    }

    register S8 *p = audio->sbeg;
    register U32 ps = audio->pos;
    U32 d = audio->delta;
    U32 l = (audio->slen << 14);

    while (n--) {
        *b++ += (p[(ps += d) >> 14] * audio->vol);
        if (ps < l) {
            continue;
        }

        ps -= l;
        p = audio->SampleStart;
        if (((l = (audio->slen = audio->SampleLength) << 14) < 0x10000) || !audio->loop(audio)) {
            audio->slen = ps = d = 0;
            p = smplbuf;
            break;
        }
    }

    audio->sbeg = p;
    audio->pos = ps;
    audio->delta = d;
    if (audio->mode & 4) {
        audio->mode = 0;
    }
}

void mix_add_ov(struct Audio *audio, int n, S32 *b) {
    if (audio->sbeg == (S8 *)&nul || ((audio->mode & 1) == 0) || (audio->slen == 0)) {
        return;
    }

    if (audio->vol > 0x40) {
        audio->vol = 0x40;
    }

    if ((audio->mode & 3) == 1) {
        audio->sbeg = audio->SampleStart;
        audio->slen = audio->SampleLength;
        audio->pos = 0;
        audio->mode |= 2;
    }

    if (audio->vol == 0) {
        return;
    }

    register S8 *p = audio->sbeg;
    register U32 ps = audio->pos;
    register U32 l = (audio->slen << 14);
    register U32 d = audio->delta;

    const U32 FRACTION_BITS = 14;
    const U32 INTEGER_MASK = (0xFFFFFFFF << FRACTION_BITS);
    const U32 FRACTION_MASK = (~ INTEGER_MASK);
    
    while (n--) {
        U32 psreal = ps >> FRACTION_BITS;
        S8 v1 = p[psreal];
        S8 v2 = (psreal + 1 < audio->slen) ? p[psreal + 1] : audio->SampleStart[0];

        S32 sample = v1 + (((signed) ((v2 - v1) * (ps & FRACTION_MASK))) >> FRACTION_BITS);
        *b++ += audio->vol * sample;

        ps += d;
        if (ps >= l) {
            ps -= l;
            p = audio->SampleStart;
            if (((l = (audio->slen = audio->SampleLength) << 14) < 0x10000) || !audio->loop(audio)) {
                audio->slen = ps = d = 0;
                p = smplbuf;
                break;
            }
        }
    }

    audio->sbeg = p;
    audio->pos = ps;
    audio->delta = d;

    if (audio->mode & 4) {
        audio->mode = 0;
    }
}
	
void (*mix)(struct Audio *,int,S32 *)=&mix_add;

void mixit(int audio_samples, int buf_position) {
    int loop_counter;
    S32 *pointer_position;

    // Process multimode audio mixing
    if (multimode) {
        for (int channel = 4; channel <= 7; channel++) {
            if (act[channel]) {
                mix(&audioData[channel], audio_samples, &tbuf[buf_position]);
            }
        }

        pointer_position = &tbuf[HALFBUFSIZE + buf_position];
        for (loop_counter = 0; loop_counter < audio_samples; loop_counter++, pointer_position++) {
            *pointer_position = (*pointer_position > 16383) ? 16383 :
                                (*pointer_position < -16383) ? -16383 : *pointer_position;
        }
    } else {
        if (act[3]) {
            mix(&audioData[3], audio_samples, &tbuf[buf_position]);
        }
    }

    // Process non-multimode audio mixing
    for (int channel = 0; channel < 3; channel++) {
        if (act[channel]) {
            mix(&audioData[channel], audio_samples, &tbuf[HALFBUFSIZE * (channel > 0) + buf_position]);
        }
    }
}

void mixem(U32 audio_samples, U32 buf_position)
{
	if (over==-1) {
		mix=&mix_add_ov;
	 } else {
		mix=&mix_add;
	 }
	mixit(audio_samples, buf_position);
}

void open_snddev() {
	// Configure the audio conversion method
    if (force8) {
        conv = &conv_u8;
    }

    multiplier = 4;
    blocksize = 32768;
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


void TfmxTakedown() {
	if (toOutFile==1) {
		close(sndhdl);
	}

	free(smplbuf);
}

int try_to_makeblock() {
    static const int threshold = BUFSIZE / 2;
    static S32 num_samples_to_process = 0, buf_position = 0;
    int audio_samples, buf_proc_counter = 0;
    int loops = 0;

    while (available_sound_data() < threshold && trackManager.PlayerEnable) {
        loops++;
        tfmxIrqIn();

        // Calculate the number of samples to process
        num_samples_to_process = calculateSamplesToProcess();

        // Process audio data in blocks
        processAudioData(&num_samples_to_process, &buf_position, &buf_proc_counter, &audio_samples);

        checkThreadSync(loops);
    }

    return (trackManager.PlayerEnable ? buf_proc_counter : -1);
}

// Helper function to calculate the number of samples to process
S32 calculateSamplesToProcess() {
    S32 processSamples = (eClocks * (outRate >> 1));
    eRem += (processSamples % 357955);
    processSamples /= 357955;

    if (eRem > 357955) {
        processSamples++;
        eRem -= 357955;
    }

    return processSamples;
}

// Helper function to process the audio data
void processAudioData(S32* num_samples_to_process, S32* buf_position, int* buf_proc_counter, int* audio_samples) {

    while (*num_samples_to_process > 0) {
        *audio_samples = blocksize - *buf_position;
        if (*audio_samples > *num_samples_to_process) {
            *audio_samples = *num_samples_to_process;
        }

        mixem(*audio_samples, *buf_position);

        if (toOutFile == 0) {
            if (*audio_samples > AUDIO_OUTPUT_WORKSPACE_CAPACITY) {
                abort();
            }

            for (int index = 0; index < *audio_samples; index++) {
                audio_output_workspace[index] = (audio_frame){
                    .left = tbuf[HALFBUFSIZE + *buf_position + index],
                    .right = tbuf[*buf_position + index],
                };
            }

            const audio_frame_block block = {
                .frame_count = (size_t)*audio_samples,
                .frames = audio_output_workspace,
            };
            if (audio_output_null_adapter_submit(&audio_output_adapter, &block) !=
                AUDIO_OUTPUT_SUBMIT_ACCEPTED) {
                abort();
            }

            for (int index = 0; index < *audio_samples; index++) {
                tbuf[*buf_position + index] = 0;
                tbuf[HALFBUFSIZE + *buf_position + index] = 0;
            }
        }

        bytes += *audio_samples;
        *buf_position += *audio_samples;
        *num_samples_to_process -= *audio_samples;

        if (((unsigned int)*buf_position) == blocksize || !trackManager.PlayerEnable) {
            if (toOutFile != 0) {
                conv(&tbuf[0], *buf_position);
            }
            *buf_position = 0;
            (*buf_proc_counter)++;
        }
    }
}

// Helper function for thread synchronization
void checkThreadSync(int loops) {
    (void)loops;
}

// SDL Callback function to fill the audio buffer for playback
void fill_audio(void *udata, Uint8 *stream, int len) {

    // Variables for buffer management
    int avail = available_sound_data(); // Amount of available sound data
    int total_len = len;                // Total length to write
    int written = 0;                    // Amount of data written so far

    // Fill with silence if available data is less than requested length
    if (avail < len) {
        SDL_memset(stream + avail, 0, len - avail); // Fill remainder with zeros (silence)
        len = avail; // Adjust the length to the available data
    }

    // Handle audio data writing, considering the ring buffer boundary
    while (total_len > 0) {
        // Adjust len to fit the buffer if adding len to btail exceeds BUFSIZE
        if (btail + len > BUFSIZE) {
            len = BUFSIZE - btail;
        }

        // Mix audio data from the buffer into the stream
        SDL_MixAudio(stream + written, &buf.b8[btail], len, SDL_MIX_MAXVOLUME);

        // Update buffer tail position, wrapping around at the buffer boundary
        btail = (btail + len) % BUFSIZE;
        written += len; // Update the number of bytes written
        total_len -= len; // Decrease the remaining length to be written
        len = total_len; // Update len for the next iteration
    }

    // Signal a condition variable, used for synchronization with other threads
    pthread_cond_signal(&cond);
}

int write_output() {
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
    // Initialize synchronization primitives
    initSyncPrimitives();

    // Main audio processing loop
    processAudio();

    if (toOutFile != 0) {
        // Finalize and write any remaining output
        finalizeAudioOutput();

        // Wait for any remaining audio data to be processed
        waitForRemainingAudioData();
    }

    // Clean up synchronization primitives
    cleanupSyncPrimitives();

    return 0;
}

void initSyncPrimitives() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
}

volatile sig_atomic_t stop = 0;

void inthand(int signum) {
    printf("BREAK\n");
    stop = 1;
}

void processAudio() {
    while (!stop && try_to_makeblock() >= 0) {
        write_output();
    }
}

void finalizeAudioOutput() {
    write_output();
}

void waitForRemainingAudioData() {
    // Only wait if not outputting to a file
    if (toOutFile == 0) {
        
        // Wait until there's no more available sound data
        while (available_sound_data() > 0) {
            SDL_Delay(25); // Delay to reduce CPU usage
        }
    }
}

void cleanupSyncPrimitives() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
}
