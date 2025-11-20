#ifndef RLE_H
#define RLE_H

#include <stdio.h>

int rle_open_compression_file(const char *in_file_name, FILE **out_file_handle);
int rle_compress_raw_bytes(FILE *in_file, FILE *out_file);

int rle_check_extension(const char *file_name);
int rle_open_decompression_file(const char *in_file_name, FILE **out_file_handle);
int rle_decompress(FILE *in_file, FILE *out_file);

#endif
