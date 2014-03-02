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

::FLAC__StreamDecoderWriteStatus Decoder::write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const flacbuffer[])
{

    unsigned blocksize=frame->header.blocksize;
    format.samplerate =  frame->header.sample_rate;
    format.channels = frame->header.channels;
    format.bits = frame->header.bits_per_sample;
//    format.format=WAVE_FORMAT_PCM;
    debug("write_callback: %d bits, %d channels, %d rate...\n",format.bits,format.channels,format.samplerate);

	if (frame->header.number_type != FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER) {
	   debug("wrong assumptiong: not a sample number at %lu\n", frame->header.number.frame_number);
	   }
	if (frame->header.channel_assignment!= FLAC__CHANNEL_ASSIGNMENT_INDEPENDENT && format.channels!=2) {
       debug("wrong assumptiong: not a channel independent: %u ( %d channels)!\n", frame->header.channel_assignment,format.channels);
       return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
       }


		    for (unsigned i=0; i<blocksize; ) {
        		unsigned char* start=(unsigned char* )buffer+bufused;
    			unsigned bufsize = (audio_buffersize-bufused) / format.channels / ((format.bits-1)/8+1); //may be wrong with (bits % 8)!=0
		        unsigned limit = blocksize<(bufsize+i)?blocksize:(bufsize+i);

        		// no good - should be optimized!
		        for (; i<limit; i++) {
        		    for (int ch=0; ch<format.channels; ch++) {
        		    	//debug(":%d: %04x - ",ch,(unsigned short)(flacbuffer[ch][i]));
                		for (int bits=0; bits<format.bits; bits+=8) {
		                    *(start++)= (unsigned char)(flacbuffer[ch][i] >> bits); //may be wrong with (bits % 8)!=0
		                    //debug("%02x ",(unsigned char)(flacbuffer[ch][i] >> bits));
		                	}
		            	}
		            //debug("\n");
		        	}

				if (start-(unsigned char* )buffer>audio_buffersize- format.channels * ((format.bits-1)/8+1) ) {
        			int written = output_play_samples(a, &format, buffer, start-(unsigned char* )buffer, (filepos=(sample_rate?(frame->header.number.sample_number+limit)*1000/sample_rate:0)) );
                	//(debug) fwrite(buffer,1,start-(unsigned char* )buffer,fpraw);
                	debug("write_callback play: %d bytes (written %d) / %d blocks (%d buf / %d audiobuf), bufused=%d\n",start-(unsigned char* )buffer,written,blocksize,bufsize,audio_buffersize,bufused);
		        	if (written < start-(unsigned char* )buffer) {
        		    	WinPostMsg(hwnd,WM_PLAYERROR,0,0);
		            	debug("ERROR: write error\n");
        		    	return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		            	}
		        	//return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
		        	//DosSleep(blocksize*500/sample_rate);
		        	bufused=0;
		    		}
		    	else
		    		bufused=start-(unsigned char* )buffer;
        		}
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void Decoder::metadata_callback(const ::FLAC__StreamMetadata *metadata)
{
    debug("metadata block type=%d\n",metadata->type);

    /* print some stats */
    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        /* save for later */
        total_samples = metadata->data.stream_info.total_samples;
        format.samplerate =  sample_rate = metadata->data.stream_info.sample_rate;
        format.channels = channels = metadata->data.stream_info.channels;
        format.bits = bps = metadata->data.stream_info.bits_per_sample;
        format.format = 1;

        debug("sample rate    : %u Hz\n", sample_rate);
        debug("channels       : %u\n", channels);
        debug("bits per sample: %u\n", bps);
#ifdef _MSC_VER
        debug("total samples  : %I64u\n", total_samples);
#else
        debug("total samples  : %llu\n", total_samples);
#endif
    }
   else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
       struct info_item {
        char* name;
        char** field;
        //char* name;
         } song_infos[]={
            {"tech_info",&tech_info},
            {"title"    ,&title    },
            {"artist"   ,&artist   },
            {"album"    ,&album    },
            {"date"     ,&year     },
            {"comment"  ,&comment  },
            {"genre"    ,&genre    },
            {"tracknumber",&track  },
            {"copyright",&copyright},
            {"REPLAYGAIN_REFERENCE_LOUDNESS",&REPLAYGAIN_REFERENCE_LOUDNESS},
            {"REPLAYGAIN_TRACK_GAIN",&REPLAYGAIN_TRACK_GAIN},
            {"REPLAYGAIN_TRACK_PEAK",&REPLAYGAIN_TRACK_PEAK},
            {"REPLAYGAIN_ALBUM_GAIN",&REPLAYGAIN_ALBUM_GAIN},
            {"REPLAYGAIN_ALBUM_PEAK",&REPLAYGAIN_ALBUM_PEAK}
            };

   	for (unsigned int i=0; i<metadata->data.vorbis_comment.num_comments; i++) {
    		const char* str=(const char*)metadata->data.vorbis_comment.comments[i].entry;
        	debug("metadata: %s\n",str);
            for (unsigned int j=0; j<sizeof(song_infos)/sizeof(song_infos[0]); j++)
            	if (strnicmp(str,song_infos[j].name,strlen(song_infos[j].name))==0) {
            		const char* r=strchr(str,'=');
            		if (r) {
            			if (*song_infos[j].field) delete *song_infos[j].field;
            			*song_infos[j].field=strnewdup(r+1);
            			}
            		debug("metadata found: %s = %s (%d:%s)\n",song_infos[j].name,r?r+1:"",i,str);
            		break;
            	 	}
            }
	}
}

void Decoder::error_callback(FLAC__StreamDecoderErrorStatus status)
{
    debug("Got error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

