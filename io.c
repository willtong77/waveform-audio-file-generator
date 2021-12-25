#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "io.h"

void fatal_error(const char *message) {
  fprintf(stderr, "Error: %s\n", message);
  exit(1);
}

void write_byte(FILE *out, char val) {
  int write = fwrite(&val, sizeof(char), 1, out);
  if (write != 1) { fatal_error("Could not write to file with write_byte."); }
}

void write_bytes(FILE *out, const char data[], unsigned n) {
  long write = fwrite(data, sizeof(char), n, out);
  if (write != (long) n) { fatal_error("Could not write to file with write_bytes."); }
}

void write_u16(FILE *out, uint16_t value) {
  int write = fwrite(&value, sizeof(uint16_t), 1, out);
  if (write != 1) { fatal_error("Could not write to file with write_u16."); }
}

void write_u32(FILE *out, uint32_t value) {
  int write = fwrite(&value, sizeof(uint32_t), 1, out);
  if (write != 1) { fatal_error("Could not write to file with  write_u32."); }
}

void write_s16(FILE *out, int16_t value) {
  int write = fwrite(&value, sizeof(int16_t), 1, out);
  if (write != 1) { fatal_error("Could not write to file with write_s16."); }
}

void write_s16_buf(FILE *out, const int16_t buf[], unsigned n) {
  for (int i = 0; i < (int) n; i++) {
    write_s16(out, buf[i]);
  }
}

void read_byte(FILE *in, char *val) {
  if (feof(in)) { fatal_error("End of file reached, unable to read value with read_byte."); }
  int read = fread(val, sizeof(char), 1, in);
  if (read != 1) { fatal_error("Could not read file with read_byte."); }
}

void read_bytes(FILE *in, char data[], unsigned n) {
  if (feof(in)) { fatal_error("End of file reached, unable to read more values with read_bytes."); }
  long read = fread(data, sizeof(char), n, in);
  if (read != (long) n) { fatal_error("Could not read file with read_bytes."); }
}

void read_u16(FILE *in, uint16_t *val) {
  if (feof(in)) { fatal_error("End of file reached, unable to read value with read_u16."); }
  int read = fread(val, sizeof(uint16_t), 1, in);
  if (read != 1) { fatal_error("Could not read file with read_u16."); }
}

void read_u32(FILE *in, uint32_t *val) {
  if (feof(in)) { fatal_error("End of file reached, unable to read value with read_u32."); }
  int read = fread(val, sizeof(uint32_t), 1, in);
  if (read != 1) { fatal_error("Could not read file with read_u32."); }
}

void read_s16(FILE *in, int16_t *val) {
  if (feof(in)) { fatal_error("End of file reached, unable to read value with read_s16."); }
  int read = fread(val, sizeof(int16_t), 1, in);
  if (read != 1) { fatal_error("Could not read file with read_s16"); }
}

void read_s16_buf(FILE *in, int16_t buf[], unsigned n) {
  for (int i = 0; i < (int) n; i++) {
    if (feof(in)) { fatal_error("End of file reached, unable to read more values with read_s16_buf."); }
    read_s16(in, &buf[i]);
  }
}

// TESTING FUNCTIONS

/*
int main(void) {
  FILE *fpw1 = fopen("io-test-1.dat", "wb");
  FILE *fpw2 = fopen("io-test-2.dat", "wb");
  FILE *fpw3 = fopen("io-test-3.dat", "wb");
  FILE *fpw4 = fopen("io-test-4.dat", "wb");
  FILE *fpw5 = fopen("io-test-5.dat", "wb");
  FILE *fpw6 = fopen("io-test-6.dat", "wb"); 

  char val_1 = 'x';
  const char data_1[] = {'a', 'b', 'c'};
  uint16_t uint16_1   = (uint16_t) 32765;
  uint32_t uint32_1   = (uint32_t) 2147483645;
  int16_t  int16_1    = (int16_t) -32765;
  int16_t int16_arr_1[] = {-32759, 32760, -32761, 32762, -32763, 32764};

  write_byte(fpw1, val_1);
  write_bytes(fpw2, data_1, 3);
  write_u16(fpw3, uint16_1);
  write_u32(fpw4, uint32_1);
  write_s16(fpw5, int16_1);
  write_s16_buf(fpw6, int16_arr_1, 6);

  fclose(fpw1);
  fclose(fpw2);
  fclose(fpw3);
  fclose(fpw4);
  fclose(fpw5);
  fclose(fpw6);

  FILE *fpr1 = fopen("io-test-1.dat", "rb");
  FILE *fpr2 = fopen("io-test-2.dat", "rb");
  FILE *fpr3 = fopen("io-test-3.dat", "rb");
  FILE *fpr4 = fopen("io-test-4.dat", "rb");
  FILE *fpr5 = fopen("io-test-5.dat", "rb");
  FILE *fpr6 = fopen("io-test-6.dat", "rb");

  char* val_2 = malloc(sizeof(char));
  char data_2[3];
  uint16_t* uint16_2 = malloc(sizeof(uint16_t));
  uint32_t* uint32_2 = malloc(sizeof(uint32_t));
  int16_t* int16_2   = malloc(sizeof(int16_t));
  int16_t int16_arr_2[6];

  read_byte(fpr1, val_2);
  printf("val_2: %c (expected: %c)\n", *val_2, val_1);

  read_bytes(fpr2, data_2, 3);
  for (int i = 0; i < 3; i++) { printf("data_2[%d]: %c (expected: %c)\n", i, data_2[i], data_1[i]); }

  read_u16(fpr3, uint16_2);
  printf("uint16_2: %u (expected: %u)\n", *uint16_2, uint16_1);

  read_u32(fpr4, uint32_2);
  printf("uint32_2: %u (expected: %u)\n", *uint32_2, uint32_1);

  read_s16(fpr5, int16_2);
  printf("int16_2: %d (expected: %d)\n", *int16_2, int16_1);

  read_s16_buf(fpr6, int16_arr_2, 6);
  for (int i = 0; i < 6; i++) { printf("int16_arr_2[%d]: %d (expected: %d)\n", i, int16_arr_2[i], int16_arr_1[i]); }

  fclose(fpr1);
  fclose(fpr2);
  fclose(fpr3);
  fclose(fpr4);
  fclose(fpr5);
  fclose(fpr6);

  free(val_2);
  free(uint16_2);
  free(uint32_2);
  free(int16_2);

  return 0;
}
*/
