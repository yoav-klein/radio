
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mad.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#include <unistd.h>

#include "http-client.h" // http connection

pa_simple *device = NULL;
int ret = 1;
int error;
struct mad_stream mad_stream;
struct mad_frame mad_frame;
struct mad_synth mad_synth;

void output(struct mad_header const *header, struct mad_pcm *pcm);

int main(int argc, char **argv) {
    // Parse command-line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s URL\n", argv[0]);
        return 0;
    }
	
	// TODO: set the rate value dynamically, according to the stream
    // Set up PulseAudio 16-bit 44.1kHz stereo output
    static const pa_sample_spec ss = { .format = PA_SAMPLE_S16LE, .rate = 48000, .channels = 2 };
    if (!(device = pa_simple_new(NULL, "MP3 player", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        printf("pa_simple_new() failed\n");
        return 255;
    }

    // Initialize MAD library
    mad_stream_init(&mad_stream);
    mad_synth_init(&mad_synth);
    mad_frame_init(&mad_frame);

		
	struct http_handle *handle = init_connection(argv[1], NULL);
	struct chunk chunk = { 0 };
	
	unsigned char *last = NULL;
    // Decode frame and synthesize loop
    chunk = read_chunk(handle->sock);
    mad_stream_buffer(&mad_stream, chunk.data, chunk.size);
    
    
    // TODO: set frame size dynamically
    while (1) {
		#define FRAME_SIZE (384)
		
		size_t unused = mad_stream.bufend - mad_stream.next_frame;
		
		// if unused < FRAME_SIZE
		if(unused <= (FRAME_SIZE))
		{
			#ifdef DEBUG
			fprintf(stderr, "unused < FRAME_SIZE, unused: %lu, reading again\n", unused);
			#endif 
			
			unsigned char tmp[FRAME_SIZE] = { 0 };
			if(unused > 0)
			{
				memcpy(tmp, mad_stream.next_frame, unused);	
			}
			
			free(chunk.data);
			chunk = read_chunk(handle->sock);
			
			// in case unused > 0, allocate space for both unused and new data	
			chunk.data = realloc(chunk.data, chunk.size + unused);
			
			if(NULL == chunk.data)
			{
				perror("realloc");
				
				exit(1);
			}
			
			if(unused > 0)
			{
				memmove(chunk.data + unused, chunk.data, chunk.size);
				memcpy(chunk.data, tmp, unused);
			}
			mad_stream_buffer(&mad_stream, chunk.data, chunk.size + unused);
			
		}
		
        int written_bytes = 0;
        
        #ifdef PRINT_STREAM
        while(written_bytes < FRAME_SIZE)
        {
        	written_bytes = write(1, mad_stream.this_frame + written_bytes, FRAME_SIZE - written_bytes);
        }
        #endif
        
        if (mad_frame_decode(&mad_frame, &mad_stream)) {
        	  fprintf(stderr, "%s\n", mad_stream_errorstr(&mad_stream));
            if (MAD_RECOVERABLE(mad_stream.error)) {
            	
                continue;
            } else if (mad_stream.error == MAD_ERROR_BUFLEN) {
            	//printf("MAD_ERROR_BUFLEN\n");
                continue;
            } else {
                break;
            }
        }
        // Synthesize PCM data of frame
        mad_synth_frame(&mad_synth, &mad_frame);
        output(&mad_frame.header, &mad_synth.pcm);
    }

    // Close
   // fclose(fp);

    // Free MAD structs
    mad_synth_finish(&mad_synth);
    mad_frame_finish(&mad_frame);
    mad_stream_finish(&mad_stream);

    // Close PulseAudio output
    if (device)
        pa_simple_free(device);

    return EXIT_SUCCESS;
}

// Some helper functions, to be cleaned up in the future
int scale(mad_fixed_t sample) {
     /* round */
     sample += (1L << (MAD_F_FRACBITS - 16));
     /* clip */
     if (sample >= MAD_F_ONE)
         sample = MAD_F_ONE - 1;
     else if (sample < -MAD_F_ONE)
         sample = -MAD_F_ONE;
     /* quantize */
     return sample >> (MAD_F_FRACBITS + 1 - 16);
}
void output(struct mad_header const *header, struct mad_pcm *pcm) {
    register int nsamples = pcm->length;
    mad_fixed_t const *left_ch = pcm->samples[0], *right_ch = pcm->samples[1];
    static char stream[1152*4];
    if (pcm->channels == 2) {
        while (nsamples--) {
            signed int sample;
            sample = scale(*left_ch++);
            stream[(pcm->length-nsamples)*4 ] = ((sample >> 0) & 0xff);
            stream[(pcm->length-nsamples)*4 +1] = ((sample >> 8) & 0xff);
            sample = scale(*right_ch++);
            stream[(pcm->length-nsamples)*4+2 ] = ((sample >> 0) & 0xff);
            stream[(pcm->length-nsamples)*4 +3] = ((sample >> 8) & 0xff);
        }
        if (pa_simple_write(device, stream, (size_t)1152*4, &error) < 0) {
            fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
            return;
        }
    } else {
        printf("Mono not supported!");
    }
}


