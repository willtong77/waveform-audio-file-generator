#include "wave.h"
#include "io.h"
#include <stdio.h>

int main(int argc, char* argv[]) {

	// Error check input
	if (argc != 5) {
		fatal_error("invalid user input");
	}
	char* wavfilein = argv[1];
	char* wavfileout = argv[2]; 
	char* delay_str = argv[3]; 
	char* amp_str = argv[4]; 

	// Convert delay and amp into primitive
	int delay; 
	sscanf(delay_str, "%d", &delay);
  if (delay < 0) {
    fatal_error("Invalid delay value from command line");
  }
	delay *= 2; 

	float amp; 
	sscanf(amp_str, "%f", &amp);
  if (amp < 0) {
    fatal_error("Invalid amplitude value from command line");
  }

	// Allocate num_samples
	unsigned* num_samples_stereo = (unsigned*)malloc(sizeof(unsigned)); 

	// Open file and call read_wave_header to get num_samples
	FILE* fp_r = fopen(wavfilein, "rb");
	if (!fp_r) {
		fatal_error("Unable to open file");
	}

	read_wave_header(fp_r, num_samples_stereo);
	*num_samples_stereo *= 2; 

	// Initialize values in stereo_buf using fread
	int16_t* stereo_buf = (int16_t*)calloc((*num_samples_stereo), sizeof(int16_t)); 

	int num_vals = fread(stereo_buf, sizeof(stereo_buf[0]), *num_samples_stereo, fp_r);
	if ((unsigned int)num_vals != *num_samples_stereo) {
    	fatal_error("Number of values and stereo samples are unequal in .wav file");
	}

  	// Defining and dynamically allocating output array with echo
	int16_t* echo_result = (int16_t*)calloc((*num_samples_stereo + delay), sizeof(int16_t)); 

	// First put in the original stereo_buffer
	for (unsigned int i = 0; i < *num_samples_stereo; i++) {
		echo_result[i] = stereo_buf[i]; 
	}

	// Next apply the gain to stereo_buf
	apply_gain(stereo_buf, *num_samples_stereo, amp); 

	// Add the echo into the result
	for (unsigned int i = delay; i < *num_samples_stereo + delay; i++) {
		echo_result[i] = echo_result[i] + stereo_buf[i-delay]; 
	}

	// Write into new wav file
	FILE* fp_w = fopen(wavfileout, "wb"); 
	if (!fp_w) {
		fatal_error("issue opening wavfileout.");
	}
	write_wave_header(fp_w, (*num_samples_stereo + delay) / 2); 
	fwrite(echo_result, sizeof(echo_result[0]), *num_samples_stereo + delay, fp_w); 

	// Free all dynamically allocated variables, close filepointers, and return 0
	fclose(fp_w); 
	fclose(fp_r); 
	free(num_samples_stereo); 
	free(stereo_buf); 
	free(echo_result); 
	return 0; 
}
