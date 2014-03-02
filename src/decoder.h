#ifndef __Decoder_h_
#define __Decoder_h_

#include "pm123/format.h"
#include "pm123/decoder_plug.h"
#include "pm123/plugin.h"
#include "FLAC++/decoder.h"

/* definitions for id3tag.cpp */
#ifndef FILE_T
#define FILE_T FILE*
#endif
#ifndef OFF_T
#define OFF_T signed long
#endif
typedef signed int Int;


typedef struct {
    OFF_T         FileSize;
    Int           GenreNo;
    Int           TrackNo;
    char          Genre   [128];
    char          Year    [ 20];
    char          Track   [  8];
    char          Title   [256];
    char          Artist  [256];
    char          Album   [256];
    char          Comment [512];
    } TagInfo_t;

/* */

inline char* strnewdup(const char* str) {
	return str?strcpy(new char[strlen(str)+1],str):NULL;
	}
void decoder_thread(void *arg);

#if defined(DEBUG)
 void debug(const char* str, ...) __attribute__ ((format (printf, 1, 2)));
#else
 inline void debug(const char* str, ...) {}
#endif



Int Read_APE_Tags ( FILE_T fp, TagInfo_t* tip );
Int Read_ID3V1_Tags ( FILE_T fp, TagInfo_t* tip );

class Decoder: public FLAC::Decoder::File, public DECODER_PARAMS {
protected:
    virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
    virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata);
    virtual void error_callback(::FLAC__StreamDecoderErrorStatus status);


	FORMAT_INFO format;
    HEV play,ok;
    int decodertid;
    int status;
    int stop;
	int flac_ok;
	int filepos;
	char* buffer;
	int bufused;
	FLAC__uint64 total_samples;
	unsigned sample_rate;
	unsigned channels;
	unsigned bps;
	long last_length;
//(debug)	FILE* fpraw;

   /* general technical information string */
   char  *tech_info;
   /* meta information */
   char  *title  ;
   char  *artist ;
   char  *album  ;
   char  *year   ;
   char  *comment;
   char  *genre  ;

   /* added since PM123 1.32 */
   char  *track    ;
   char  *copyright;

   char  *REPLAYGAIN_REFERENCE_LOUDNESS;
   char  *REPLAYGAIN_TRACK_GAIN;
   char  *REPLAYGAIN_TRACK_PEAK;
   char  *REPLAYGAIN_ALBUM_GAIN;
   char  *REPLAYGAIN_ALBUM_PEAK;

public:
    Decoder();
    virtual ~Decoder();

	void decoder_thread();
	ULONG decoder_length();
	ULONG decoder_command(ULONG msg, DECODER_PARAMS *params);
    int decoder_status() { return status; }

protected:
      int outstring(int err,const char* str, ...) __attribute__ ((format (printf, 3, 4)));

      int flac_open(char *filename) {
        flac_ok = true;
        set_metadata_respond(FLAC__METADATA_TYPE_VORBIS_COMMENT);
    	FLAC__StreamDecoderInitStatus init_status = init(filename);
    	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
    		debug("ERROR: initializing decoder: %s\n", FLAC__StreamDecoderInitStatusString[init_status]);
    		init_status=init_ogg(filename);
    		if  (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        		debug("ERROR: initializing decoder: %s\n", FLAC__StreamDecoderInitStatusString[init_status]);
        		flac_ok = false;
        		}
    		}
        return flac_ok?0:1;
        }
//      int flac_open(char *filename, int &samplerate,
//               int &channels, int &bits, int &format);
      int flac_init() {
        buffer = new char[audio_buffersize];
        bufused=0;
        return process_until_end_of_metadata()?0:1;
        }

	  int flac_filepos() {
	    	return filepos;
	    }
      int flac_close() {
      	if (buffer) {
      		delete buffer;
      		buffer = NULL;
      		}
      	return finish();
      	}

      int flac_jumpto(long offset) {
        	debug("flac_jumpto: %ld\n",offset);
      		offset=offset<0?0:offset;
            unsigned long sample=((unsigned long)offset)*sample_rate/1000;
      		sample=sample>total_samples?total_samples:sample;
            debug("flac_jumpto: absolute %lu/%lu\n",sample,(ULONG)total_samples);
        	seek_absolute(sample);
        	flush();
        	return 0;
        	}
      //int flac_skip(long ms);
      int flac_filelength() {
      		return sample_rate?total_samples*1000/sample_rate:0;
      		}

      long flac_vbr() {
        return 0; //long(vbr_acc?((double)vbr_bits*info.sample_freq/vbr_acc/1000/1000):0.0);
        }


friend ULONG _System decoder_fileinfo(char *filename, DECODER_INFO *info);
friend int _System decoder_init(void **W);
friend BOOL _System decoder_uninit(void *W);

};


#endif // defined __Decoder_h_


