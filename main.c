#include "rle.h"
#include <stdio.h>
#include <string.h>

void print_usage(char *prog_name);

int main(int argc, char *argv[])
{
	if (argc != 3) {
		print_usage(argv[0]);
		return 1;
	}

	char *mode = argv[1];
	char *file_path = argv[2];

	FILE *input_file = NULL;
	FILE *output_file = NULL;

	if (strcmp(mode, "-c") == 0) {
		input_file = fopen(file_path, "rb");
		if (!input_file) {
			fprintf(stderr, "Could not open input file\n");
			return 1;
		}
		if (rle_open_compression_file(file_path, &output_file) != 0) {
			fprintf(stderr, "Could not create %s.rle file\n", file_path);
			fclose(input_file);
			return 1;
		}
		if (rle_compress_raw_bytes(input_file, output_file) != 0) {
			fprintf(stderr, "Something wrong happened during the compression\n");
			fclose(input_file);
			fclose(output_file);
			return 1;
		}

		fprintf(stderr, "%s was compressed to %s.rle\n", file_path, file_path);
		fclose(input_file);
		fclose(output_file);
	} else if (strcmp(mode, "-d") == 0) {
		if (rle_check_extension(file_path) == 0) {
			fprintf(stderr, "Received a non rle file, so decompression can not be done\n");
			return 1;
		}
		input_file = fopen(file_path, "rb");
		if (!input_file) {
			fprintf(stderr, "Could not open input file\n");
			return 1;
		}
		if (rle_open_decompression_file(file_path, &output_file) != 0) {
			fprintf(stderr, "Could not create the decompressed file\n");
			fclose(input_file);
			return 1;
		}
		if (rle_decompress(input_file, output_file) != 0) {
			fprintf(stderr, "Something wrong happened during the decompression\n");
			fclose(input_file);
			fclose(output_file);
			return 1;
		}

		fprintf(stderr, "%s was successfully decompressed to its original format\n", file_path);
		fclose(input_file);
		fclose(output_file);
	} else {
		fprintf(stderr, "Unknown mode: %s\n", mode);
		print_usage(argv[0]);

		return 1;
	}

	return 0;
}

void print_usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s <mode> <file_path>\n", prog_name);
	fprintf(stderr, "Modes:\n");
	fprintf(stderr, "\t-c\tCompress file\n");
	fprintf(stderr, "\t-d\tDecompress file (must have .rle extension)\n");
}

