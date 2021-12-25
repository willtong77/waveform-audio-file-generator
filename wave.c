#include "io.h"
#include "wave.h"

void write_wave_header(FILE *out, unsigned num_samples) {
  //
  // See: http://soundfile.sapp.org/doc/WaveFormat/
  //

  uint32_t ChunkSize, Subchunk1Size, Subchunk2Size;
  uint16_t NumChannels = NUM_CHANNELS;
  uint32_t ByteRate = SAMPLES_PER_SECOND * NumChannels * (BITS_PER_SAMPLE/8u);
  uint16_t BlockAlign = NumChannels * (BITS_PER_SAMPLE/8u);

  // Subchunk2Size is the total amount of sample data
  Subchunk2Size = num_samples * NumChannels * (BITS_PER_SAMPLE/8u);
  Subchunk1Size = 16u;
  ChunkSize = 4u + (8u + Subchunk1Size) + (8u + Subchunk2Size);

  // Write the RIFF chunk descriptor
  write_bytes(out, "RIFF", 4u);
  write_u32(out, ChunkSize);
  write_bytes(out, "WAVE", 4u);

  // Write the "fmt " sub-chunk
  write_bytes(out, "fmt ", 4u);       // Subchunk1ID
  write_u32(out, Subchunk1Size);
  write_u16(out, 1u);                 // PCM format
  write_u16(out, NumChannels);
  write_u32(out, SAMPLES_PER_SECOND); // SampleRate
  write_u32(out, ByteRate);
  write_u16(out, BlockAlign);
  write_u16(out, BITS_PER_SAMPLE);

  // Write the beginning of the "data" sub-chunk, but not the actual data
  write_bytes(out, "data", 4);        // Subchunk2ID
  write_u32(out, Subchunk2Size);
}

void read_wave_header(FILE *in, unsigned *num_samples) {
  char label_buf[4];
  uint32_t ChunkSize, Subchunk1Size, SampleRate, ByteRate, Subchunk2Size;
  uint16_t AudioFormat, NumChannels, BlockAlign, BitsPerSample;

  read_bytes(in, label_buf, 4u);
  if (memcmp(label_buf, "RIFF", 4u) != 0) {
    fatal_error("Bad wave header (no RIFF label)");
  }

  read_u32(in, &ChunkSize); // ignore

  read_bytes(in, label_buf, 4u);
  if (memcmp(label_buf, "WAVE", 4u) != 0) {
    fatal_error("Bad wave header (no WAVE label)");
  }

  read_bytes(in, label_buf, 4u);
  if (memcmp(label_buf, "fmt ", 4u) != 0) {
    fatal_error("Bad wave header (no 'fmt ' subchunk ID)");
  }

  read_u32(in, &Subchunk1Size);
  //printf("Subchunk1Size=%u\n", Subchunk1Size);
  if (Subchunk1Size != 16u) {
    fatal_error("Bad wave header (Subchunk1Size was not 16)");
  }

  read_u16(in, &AudioFormat);
  if (AudioFormat != 1u) {
    fatal_error("Bad wave header (AudioFormat is not PCM)");
  }

  read_u16(in, &NumChannels);
  if (NumChannels != NUM_CHANNELS) {
    fatal_error("Bad wave header (NumChannels is not 2)");
  }

  read_u32(in, &SampleRate);
  if (SampleRate != SAMPLES_PER_SECOND) {
    fatal_error("Bad wave header (Unexpected sample rate)");
  }

  read_u32(in, &ByteRate); // ignore

  read_u16(in, &BlockAlign); // ignore

  read_u16(in, &BitsPerSample);
  if (BitsPerSample != BITS_PER_SAMPLE) {
    fatal_error("Bad wave header (Unexpected bits per sample)");
  }

  read_bytes(in, label_buf, 4u);
  if (memcmp(label_buf, "data", 4u) != 0) {
    fatal_error("Bad wave header (no 'data' subchunk ID)");
  }

  // finally we're at the Subchunk2Size field, from which we can
  // determine the number of samples
  read_u32(in, &Subchunk2Size);
  *num_samples = Subchunk2Size / NUM_CHANNELS / (BITS_PER_SAMPLE/8u);
}

void compute_pan(float angle, float channel_gain[]) {
  // calculate left and right gain from the equation they gave
  float L = (sqrt(2) / 2) * (cos(angle) + sin(angle));
  float R = (sqrt(2) / 2) * (cos(angle) - sin(angle));
  channel_gain[0] = L; // Channel 0 is the left channel
  channel_gain[1] = R; // Channel 1 is the right channel
}

void generate_sine_wave(int16_t mono_buf[], unsigned num_samples, float freq_hz) {
  float time; 
  for (unsigned int i = 0; i < num_samples; i++) {
    // calculate time variable since sample/(samples/sec) = time
    time = (float)i / (float)SAMPLES_PER_SECOND;
    mono_buf[i] = INT16_MAX * sin(2 * PI * freq_hz * time);
  }
}

void generate_square_wave(int16_t mono_buf[], unsigned num_samples, float freq_hz) {
  int16_t temp; 
  float time; 
  // generate a sine wave and assign to mono_buf either max or min depending
  // on the sign of the sine
  for (unsigned int i = 0; i < num_samples; i++) {
    time = (float)i/(float)(SAMPLES_PER_SECOND);
    temp = INT16_MAX * sin(2*PI*freq_hz*time);
    if (temp > 0) {
      mono_buf[i] = INT16_MAX; 
    } else {
      mono_buf[i] = INT16_MIN; 
    }
  }
}

void generate_saw_wave(int16_t mono_buf[], unsigned num_samples, float freq_hz) {
  double time;
  double temp;
  // T = 1/f
  float period = 1 / freq_hz;
  for (unsigned int i = 0; i < num_samples; i++) {
    time = (1.0 / SAMPLES_PER_SECOND) * i;
    // get time-normalized value
    temp = (time / period) - floor(time / period);
    if (temp == 0.5) {
      mono_buf[i] = 0;
    } else {
      // if we're not in the middle, just assign it the line value
      mono_buf[i] = temp * 65535.0 - 32767.0;
    }
  }
}

void apply_gain(int16_t mono_buf[], unsigned num_samples, float gain) {
  int32_t temp;
  for (unsigned int i = 0; i < num_samples; i++) {
    temp = (int32_t)( (float)mono_buf[i] * gain ) ; // casting sample value to float for computation and casting back to int32_t when updating mono_buf
    if (temp > INT16_MAX) { // clipping max
      temp = INT16_MAX; 
    } else if (temp < INT16_MIN) { // clipping min
      temp = INT16_MIN;
    }
    mono_buf[i] = (int16_t)temp;
  }
}


void apply_adsr_envelope(int16_t mono_buf[], unsigned num_samples) {
  // Special Case - number of samples is less than required of Attack, Decay, and Release
  if (num_samples < ATTACK_NUM_SAMPLES + DECAY_NUM_SAMPLES + RELEASE_NUM_SAMPLES) {
    float slope = 1.0f / (num_samples / 2);
    for (unsigned int i = 0; i < num_samples; i++) {
      if (i < (num_samples / 2) ) { // Rise
        mono_buf[i] = (slope*i)*mono_buf[i]; 
      }
      else {			    // Fall
	      mono_buf[i] = (-2.0f/num_samples) * (i - num_samples); 
      }
    }
  }

  // Normal ADSR Envelope
  else { 
    float slope_A = 1.2f / ATTACK_NUM_SAMPLES;
    float slope_D = -0.2f / DECAY_NUM_SAMPLES;
    float slope_R = -1.0f / RELEASE_NUM_SAMPLES;
    int32_t sustain = num_samples - (ATTACK_NUM_SAMPLES + DECAY_NUM_SAMPLES + RELEASE_NUM_SAMPLES);
    int bound;
    int i = 0;

    // Attack
    bound = ATTACK_NUM_SAMPLES;
    for (; i < bound; i++) {
      mono_buf[i] *= (slope_A * i); 
    }
    // Decay
    bound += DECAY_NUM_SAMPLES;
    for (; i < bound; i++) {
      mono_buf[i] *= (1.2f + (slope_D * (i - ATTACK_NUM_SAMPLES)));
    }
    // Release
    bound = num_samples;
    i += sustain; 
    for (; i < bound; i++) {
      mono_buf[i] *= (1.0f + (slope_R * (i - (ATTACK_NUM_SAMPLES + DECAY_NUM_SAMPLES + sustain))));
    }
  }
}

void mix_in(int16_t stereo_buf[], unsigned channel, const int16_t mono_buf[], unsigned num_samples) {
	int32_t temp; 
  // loop through every coupel of values
  for (unsigned int i = 0; i < (2*num_samples)-1; i += 2) {
    // add it to the existing value
    temp = stereo_buf[i + channel] + mono_buf[i/2]; 
    // clipping
    if (temp > INT16_MAX) {
      temp = INT16_MAX; 
    } else if (temp < INT16_MIN) {
      temp = INT16_MIN;
    }
    // assign to stereo_buf
    stereo_buf[i + channel] = temp; 
  }
}
