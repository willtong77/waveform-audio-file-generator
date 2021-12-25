#include "wave.h"
#include "io.h"
#include <stdio.h>

int main(int argc, char* argv[]) {

	// error check the command lines
	if (argc != 6) {
		fatal_error("not enough input arguments");
	}

	// get the command line args and load them into variables
	// need to do C-string conversion to float/int for certain variables
	char* waveform_str = argv[1];
	int waveform;
	sscanf(waveform_str, "%d", &waveform);

	char* freq_str = argv[2]; 
	float freq; 
	sscanf(freq_str, "%f", &freq);

	char* amp_str = argv[3]; 
	float amp; 
	sscanf(amp_str, "%f", &amp);

	char* numsamples_str = argv[4]; 
	int numsamples;
	sscanf(numsamples_str, "%d", &numsamples);

	char *wavfileout = argv[5]; 

	// error checking of command line args
	if (freq < 0 || amp < 0 || numsamples < 0) {
		fatal_error("invalid command line arguments");
	}

	// this is the array that will be written all the waveform values
	int16_t* waveform_vals = calloc(numsamples, sizeof(int16_t));

	// generate the waveform accordingly
	switch (waveform) {
		case 0: 
			generate_sine_wave(waveform_vals, numsamples, freq);
			break;
		case 1: 
			generate_square_wave(waveform_vals, numsamples, freq);
			break;
		case 2: 
			generate_saw_wave(waveform_vals, numsamples, freq);
			break;
		default: 
			fatal_error("invalid waveform option");
	}

	// call apply_gain
	apply_gain(waveform_vals, numsamples, amp);

	// initialize the stereo buffer and mix into left and right channels
	int16_t* stereo_buf = calloc(numsamples * 2, sizeof(int16_t));
	
	mix_in(stereo_buf, 0, waveform_vals, numsamples);
	mix_in(stereo_buf, 1, waveform_vals, numsamples);

	// now that all the values are loaded in, we can write it to wavfileout
	// first write to header, then write raw data in
	FILE* fp = fopen(wavfileout, "wb");
	write_wave_header(fp, numsamples);

	// error check and fwrite into the file
	if (!fp) {
		fatal_error("error opening file.");
	}

	fwrite(stereo_buf, sizeof(waveform_vals[0]), 2*numsamples, fp); 

	// close file pointer and free dynamically allocated memory
	fclose(fp); 
	free(waveform_vals);
	free(stereo_buf);

	return 0; 
}
