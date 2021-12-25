#include "wave.h"
#include "io.h"
#include <stdio.h>

typedef struct {
  uint16_t waveform;
  float   angle;
  int16_t adsr;
  float gain;

} Instrument;

int main(int argc, char* argv[]) {

  // Error check the command lines
  if (argc != 3) {
    fatal_error("Not enough input arguments");
  }

  Instrument instruments[16];

  // Initialize all instruments as a default instrument
  for (int i = 0; i < 16; i++) {
    instruments[i].waveform = 0; 
    instruments[i].angle = 0.0f; 
    instruments[i].adsr = 0; 
    instruments[i].gain = 0.2; 
  }

  // Reading file
  int num_samples;
  FILE* fpr = fopen(argv[1], "r");

  // Error check file opening
  if (!fpr) {
    fatal_error("error opening file");
  }

  // Reading the number of stereo sample pairs in .wav file
  fscanf(fpr, " %d", &num_samples);
  int16_t* stereo_buf = calloc(num_samples * 2, sizeof(int16_t)); 

  // file reading variables
  // f = related to file | n = related to N directive 
  char   f_directive;
  unsigned int    f_waveform;
  float  f_angle;
  int    f_adsr;
  float  f_gain;
  int    f_instrument;
  int    n_start;
  int    n_end;
  int    n_note;
  float  n_gain;
  
  int num_c;

  do {
    num_c = fscanf(fpr, " %c", &f_directive);
    if (num_c == EOF) { // end of file reached
      break;
    }

    switch (f_directive) {
      case 'N':
        if (fscanf(fpr, " %d %d %d %d %f", &f_instrument, &n_start, &n_end, &n_note, &n_gain) != 5) {
          fatal_error("Missing data for N directive");
        }

        if (n_start < 0 || n_end < 0 || n_note < 0 || n_gain < 0) {
          fatal_error("invalid value in directive information.");
        }

        // Dynamically allocating each stereo mono buffer array and initializing to 0
        int16_t* left_monobuf  = calloc(n_end - n_start + 1, sizeof(int16_t));
        int16_t* right_monobuf = calloc(n_end - n_start + 1, sizeof(int16_t));

        // Converting MIDI to frequency
        float freq = 440 * pow(2, (n_note - 69) / 12.0f);

        // Generating waveform
        switch (instruments[f_instrument].waveform) {
          case 0:
            generate_sine_wave(left_monobuf, n_end - n_start + 1, freq);
            generate_sine_wave(right_monobuf, n_end - n_start + 1, freq);
            break;
          case 1:
            generate_square_wave(left_monobuf, n_end - n_start + 1, freq);
            generate_square_wave(right_monobuf, n_end - n_start + 1, freq);
            break;
          case 2:
            generate_saw_wave(left_monobuf, n_end - n_start + 1, freq);
            generate_saw_wave(right_monobuf, n_end - n_start + 1, freq);
            break;
          default:
            fatal_error("Invalid waveform value in song file");
        }

        // Apply gain
        apply_gain(left_monobuf, n_end - n_start + 1, n_gain);
        apply_gain(right_monobuf, n_end - n_start + 1, n_gain);
        apply_gain(left_monobuf, n_end - n_start + 1, instruments[f_instrument].gain);
        apply_gain(right_monobuf, n_end - n_start + 1, instruments[f_instrument].gain);

        // Apply asdr, if applicable
        if (instruments[f_instrument].adsr == 1) {
          apply_adsr_envelope(left_monobuf, n_end - n_start + 1);
          apply_adsr_envelope(right_monobuf, n_end - n_start + 1);
        }

        // Find what the left+right channel gains would be 
        float channel_gain[] = {0.0f, 0.0f};
        compute_pan(instruments[f_instrument].angle, channel_gain);

        // Apply gain from channel_gain for each left and right
        apply_gain(left_monobuf, n_end - n_start + 1, channel_gain[0]);
        apply_gain(right_monobuf, n_end - n_start + 1, channel_gain[1]); 

        // Mix these together into stereo_buf
        mix_in(stereo_buf + (2 * n_start), 0, left_monobuf, n_end - n_start + 1);
        mix_in(stereo_buf + (2 * n_start), 1, right_monobuf, n_end - n_start + 1);

        // Freeing allocated memory for left_monobuf and right_monobuf
        free(left_monobuf);
        free(right_monobuf);

        break;

      case 'W':
        if (fscanf(fpr, " %d %u", &f_instrument, &f_waveform) != 2) {
          fatal_error("Missing data for W directive");
        }
        instruments[f_instrument].waveform = f_waveform;
        break;
 
      case 'P':
        if (fscanf(fpr, " %d %f", &f_instrument, &f_angle) != 2) {
          fatal_error("Missing data for P directive");
        }
        instruments[f_instrument].angle = f_angle;
        break;

      case 'E':
        if (fscanf(fpr, " %d %d", &f_instrument, &f_adsr) != 2) {
          fatal_error("Missing data for E directive");
        }
        instruments[f_instrument].adsr = f_adsr;
        break;

      case 'G':
        if (fscanf(fpr, " %d %f", &f_instrument, &f_gain) != 2) {
          fatal_error("Missing data for G directive");
        }
        instruments[f_instrument].gain = f_gain;
        break;

      default:
        fatal_error("Invalid directive in song file");
    }
  } while(num_c == 1);

  // Open filewriter for output .wav file
  FILE* fpr_w = fopen(argv[2], "wb");
  write_wave_header(fpr_w, num_samples); 

  if (!fpr_w) {
    fatal_error("error opening writing file");
  }

  fwrite(stereo_buf, sizeof(stereo_buf[0]), 2*num_samples, fpr_w);

  // Close all the files and free all dynamically allocated memory
  fclose(fpr);
  fclose(fpr_w);
  free(stereo_buf);

	return 0; 
}
