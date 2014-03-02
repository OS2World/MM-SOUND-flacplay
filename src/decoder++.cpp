#define INCL_DOS
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <memory.h>

#include <unistd.h>
#include <glob.h>
#include <sys/time.h>

#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stdint.h>

#include "decoder.h"

Decoder::Decoder(): FLAC::Decoder::File() {
        // empty structure
        memset((DECODER_PARAMS*)this,0,sizeof(DECODER_PARAMS));
        DosCreateEventSem(NULL,&play,0,FALSE);
        DosCreateEventSem(NULL,&ok,0,FALSE);
        format.size=sizeof(FORMAT_INFO);
        format.format=WAVE_FORMAT_PCM;
        buffer=NULL;
        decodertid=0;
        status=stop=flac_ok=filepos=0;
        total_samples=0;
        sample_rate=channels=bps=0;
        //(debug) fpraw=fopen("flacplay.raw","wb");

        tech_info=NULL;
        title	 =NULL;
        artist=NULL;
        album=NULL;
        year=NULL;
        comment=NULL;
        genre=NULL;
        track=NULL;
        copyright=NULL;
        REPLAYGAIN_REFERENCE_LOUDNESS=NULL;
        REPLAYGAIN_TRACK_GAIN=NULL;
        REPLAYGAIN_TRACK_PEAK=NULL;
        REPLAYGAIN_ALBUM_GAIN=NULL;
        REPLAYGAIN_ALBUM_PEAK=NULL;
        }

Decoder::~Decoder() {
   DosCloseEventSem(play);
   DosCloseEventSem(ok);
   //(debug) fclose(fpraw);
   }

void Decoder::decoder_thread() {

   ULONG resetcount;
//   while(1)
   jumpto = -1;
   rew = ffwd = 0;

   do
   {
      debug("decoder_thread circle started!\n");

      DosWaitEventSem(play, (ULONG)-1);
      DosResetEventSem(play,&resetcount);
      debug("decoder_thread semaphore reset!\n");

      status = DECODER_STARTING;

      last_length = -1;

//      DosResetEventSem(playsem,&resetcount);
      DosPostEventSem(ok);

//      if(open(filename,formatinfo.samplerate,
//            formatinfo.channels,formatinfo.bits,formatinfo.format))
      if(flac_open(filename))
      {
		 debug("open %s failed \n",filename);
         WinPostMsg(hwnd,WM_PLAYERROR,0,0);
         status = DECODER_STOPPED;
//         DosPostEventSem(playsem);
         continue;
      }

      if (flac_init())
      {
         debug("init %s failed \n",filename);
         WinPostMsg(hwnd,WM_PLAYERROR,0,0);
         status = DECODER_STOPPED;
//         DosPostEventSem(playsem);
         continue;
      }

      stop = 0;

      status = DECODER_PLAYING;

      if(jumpto >= 0)
      {
         flac_jumpto(jumpto);
         jumpto = -1;
         WinPostMsg(hwnd,WM_SEEKSTOP,0,0);
         }

      while (process_single() && !stop && get_state()!=FLAC__STREAM_DECODER_END_OF_STREAM ) {
         if(jumpto >= 0)
         {
            flac_jumpto(jumpto);
            jumpto = -1;
            WinPostMsg(hwnd,WM_SEEKSTOP,0,0);
      		DosResetEventSem(play,&resetcount);
         } else
         if(ffwd)
         {
            debug("thread ffwd: %d, %d/%d\n",ffwd,flac_filepos(),flac_filelength());
            //flac_skip(1000);
            //skip_single_frame();
            if (flac_filepos()+1000>flac_filelength())
            	break;
            flac_jumpto(flac_filepos()+1000);
         } else
         if(rew)
         {
            debug("thread rew: %d, %d/%d\n",rew,flac_filepos(),flac_filelength());
            if (flac_filepos()-2000<0)
                break;
            flac_jumpto(flac_filepos()-2000);
         }
      }

	  if (get_state()==FLAC__STREAM_DECODER_END_OF_STREAM && bufused) {
         int written = output_play_samples(a, &format, buffer, bufused, filepos=filepos+(bufused*1000/format.channels/((format.bits-1)/8+1)/format.samplerate) );
         //(debug)fwrite(buffer,1,bufused,fpraw);
         debug("decoder_thread write: %d bytes (written %d) / %d blocks (%d buf / %d audiobuf)\n",bufused,written,0,0,audio_buffersize);
         if (written < bufused) {
            WinPostMsg(hwnd,WM_PLAYERROR,0,0);
            debug("ERROR: write error\n");
            }
    	 }

	  debug("decoder_thread circle end!\n");
      status = DECODER_STOPPED;
      flac_close();

//      DosPostEventSem(playsem);
      WinPostMsg(hwnd,WM_PLAYSTOP,0,0);

      DosPostEventSem(ok);
      debug("decoder_thread circle end 2!\n");
   }
   while (1);
   debug("decoder_thread finish!\n");
}


ULONG Decoder::decoder_length() {
   if(status == DECODER_PLAYING)
      last_length = flac_filelength();

   if (last_length<0) last_length=0;
   return last_length;
}

ULONG Decoder::decoder_command(ULONG msg, DECODER_PARAMS *params) {
   ULONG resetcount;

   switch(msg)
   {

      case DECODER_JUMPTO:
         debug("decoder_command jumpto:%d!\n",params->jumpto);
         jumpto = params->jumpto;
         DosPostEventSem(play);
         break;

      case DECODER_PLAY:
   		 debug("decoder_command play!\n");
         if(status == DECODER_STOPPED)
         {
   			jumpto = -1;
   			rew = ffwd = 0;
            filename=strnewdup(params->filename);
            DosResetEventSem(ok,&resetcount);
            DosPostEventSem(play);
         	debug("decoder_command semaphore posted!\n");
            if (decodertid<0 || DosWaitEventSem(ok, 10000) == 640)
            {
               status = DECODER_STOPPED;
               if (decodertid>0) DosKillThread(decodertid);
               decodertid = _beginthread(::decoder_thread,0,64*1024,(void *) this);
               return 102;
            }
         }
         else
            return 101;
         break;

      case DECODER_STOP:
         debug("decoder_command stop!\n");
         if(status != DECODER_STOPPED)
         {
            DosResetEventSem(ok,&resetcount);
            stop = TRUE;
            jumpto = -1;
            rew = ffwd = 0;
            if(DosWaitEventSem(ok, 10000) == 640)
            {
               status = DECODER_STOPPED;
               if (decodertid>0) DosKillThread(decodertid);
               decodertid = _beginthread(::decoder_thread,0,64*1024,(void *) this);
               return 102;
            }
         }
         else
            return 101;
         break;

      case DECODER_FFWD:
         debug("decoder_command ffwd:%d!\n",params->ffwd);
         ffwd = params->ffwd;
         break;

      case DECODER_REW:
         debug("decoder_command rew:%d!\n",params->rew);
         rew = params->rew;
         break; // seek is too costly

      case DECODER_EQ:
         debug("decoder_command eq!\n");
         return 1;

      case DECODER_SETUP:
         debug("decoder_command setup!\n");
         output_play_samples = params->output_play_samples;
         a = params->a;
         audio_buffersize = params->audio_buffersize;
         error_display = params->error_display;
         info_display = params->info_display;
         hwnd = params->hwnd;
//         playsem = params->playsem;
//         DosPostEventSem(playsem);
         break;
      default:
         debug("decoder_command unknown!\n");
   }
   return 0;
}

int Decoder::outstring(int err,const char* str, ...) {
	const int bufsize=1024;
	static char* buf=new char[bufsize+1];

    int r;
    va_list arg_ptr;
    va_start(arg_ptr, str);
    r=vsnprintf(buf,bufsize,str,arg_ptr);
    va_end(arg_ptr);

    if (err) {
    	if (error_display) error_display(buf);
    	}
    else
    	if (info_display) info_display(buf);
    debug("%s",buf);
    return r;

	}

#if defined(DEBUG)

int debug(const char* str, ...) {
    const int bufsize=1024;
    static char* buf=new char[bufsize+1];
    static FILE* fp=NULL;
    if (fp==NULL) {
    	fp=fopen("flacplay.dbg","wt");
    	if (fp) {
    		}
    	}
    if (fp) {
    va_list arg_ptr;
    va_start(arg_ptr, str);
    vsnprintf(buf,bufsize,str,arg_ptr);
    va_end(arg_ptr);
    fprintf(fp,"%s",buf);
    fflush(fp);
	}
	return 0;

    }
#endif

