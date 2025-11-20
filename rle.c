#include "rle.h"
#include <stdint.h>
#include <string.h>

#define RLE_SIGNATURE "RLE"
#define RLE_VERSION 0b00010000
#define RLE_EXTENSION ".rle"
#define RLE_MAX_FILENAME_LENGTH 256

struct rle_packet {
	uint8_t count;
	uint8_t data;
};
typedef struct rle_packet RLE_PACKET;

struct rle_header {
	uint8_t signature[4];
	uint8_t version;
};
typedef struct rle_header RLE_HEADER;

static RLE_HEADER rle_create_header(void);
static int rle_write_header(FILE *file, RLE_HEADER header);
static int rle_write_packet(FILE *file, RLE_PACKET pkt);
static int rle_read_header(FILE *file, RLE_HEADER *header);
static int rle_read_packet(FILE *file, RLE_PACKET *pkt);
static int rle_decompress_raw_bytes(FILE *in_file, FILE *out_file);
static int rle_check_header(RLE_HEADER header);

int rle_open_compression_file(const char *in_file_name, FILE **out_file_handle)
{
	RLE_HEADER header = rle_create_header();

	char out_file_name[RLE_MAX_FILENAME_LENGTH];
	snprintf(out_file_name, RLE_MAX_FILENAME_LENGTH, "%.*s.rle", RLE_MAX_FILENAME_LENGTH - 4 - 1, in_file_name);

	FILE *out_file = fopen(out_file_name, "wb");
	if (!out_file) {
		return -1;
	}

	if (rle_write_header(out_file, header) != 0) {
		fclose(out_file);
		return -2;
	}

	*out_file_handle = out_file;

	return 0;
}

int rle_open_decompression_file(const char *in_file_name, FILE **out_file_handle)
{
	if (rle_check_extension(in_file_name) == 0) {
		return -1;
	}

	char out_file_name[RLE_MAX_FILENAME_LENGTH];
	size_t in_file_length = strlen(in_file_name);
	snprintf(out_file_name, RLE_MAX_FILENAME_LENGTH, "%.*s", (int)(in_file_length - strlen(".rle")), in_file_name);

	FILE *out_file = fopen(out_file_name, "wb");
	if (!out_file) {
		return -2;
	}

	*out_file_handle = out_file;

	return 0;
}

int rle_check_extension(const char *file_name)
{
	size_t file_length = strlen(file_name);

	if (file_length <= strlen(RLE_EXTENSION)) {
		return 0;
	}

	if (strcmp(file_name + file_length - strlen(RLE_EXTENSION), RLE_EXTENSION) == 0) {
		return 1;
	}

	return 0;
}

int rle_compress_raw_bytes(FILE *in_file, FILE *out_file)
{
	uint8_t curr_byte;
	uint8_t next_byte;

	if (fread(&curr_byte, sizeof(uint8_t), 1, in_file) == 0) {
		return -1;
	}

	uint8_t count = 1;

	while (fread(&next_byte, sizeof(uint8_t), 1, in_file)) {
		if (next_byte == curr_byte && count < UINT8_MAX) {
			count++;
		} else {
			RLE_PACKET pkt = { .count = count, .data = curr_byte };
			if (rle_write_packet(out_file, pkt) != 0) {
				return -2;
			}
			curr_byte = next_byte;
			count = 1;
		}
	}

	if (ferror(in_file)) {
		return -1;
	}

	RLE_PACKET last_pkt = { .count = count, .data = curr_byte };
	if (rle_write_packet(out_file, last_pkt) != 0) {
		return -2;
	}

	return 0;
}

int rle_decompress(FILE *in_file, FILE *out_file)
{
	RLE_HEADER header = { 0 };

	if (rle_read_header(in_file, &header) != 0) {
		return -1;
	}

	if (rle_check_header(header) != 0) {
		return -2;
	}
	
	if (rle_decompress_raw_bytes(in_file, out_file) != 0) {
		return -3;
	}

	return 0;
}

static int rle_check_header(RLE_HEADER header)
{
	if (memcmp(header.signature, RLE_SIGNATURE, 3) == 0 && header.version == RLE_VERSION) {
		return 0;
	}

	return -1;
}

static int rle_decompress_raw_bytes(FILE *in_file, FILE *out_file)
{
	RLE_PACKET pkt = { 0 };
	uint8_t buffer[UINT8_MAX];

	while (1) {
		int status = rle_read_packet(in_file, &pkt);
		if (status == 0) {
			memset(buffer, pkt.data, pkt.count);
			if (fwrite(buffer, sizeof(uint8_t), pkt.count, out_file) != pkt.count) {
				return -1;
			}
		} else if (status == -1) {
			break;
		} else {
			return -1;
		}
	}
	
	return 0;
}

static RLE_HEADER rle_create_header(void)
{
	RLE_HEADER header = {
		.signature = RLE_SIGNATURE,
		.version = RLE_VERSION
	};

	return header;
}

static int rle_write_header(FILE *file, RLE_HEADER header)
{
	if (fwrite(&(header.signature), sizeof(uint8_t), 3, file) != 3) {
		return -1;
	}

	if (fwrite(&(header.version), sizeof(uint8_t), 1, file) != 1) {
		return -1;
	}

	return 0;
}

static int rle_write_packet(FILE *file, RLE_PACKET pkt)
{
	if (fwrite(&(pkt.count), sizeof(uint8_t), 1, file) != 1) {
		return -1;
	}

	if (fwrite(&(pkt.data), sizeof(uint8_t), 1, file) != 1) {
		return -1;
	}

	return 0;
}

static int rle_read_header(FILE *file, RLE_HEADER *header)
{
	if (fread(&(header->signature), sizeof(uint8_t), 3, file) != 3) {
		if (feof(file)) {
			return -1;
		}
		if (ferror(file)) {
			return -2;
		}
		return -2;
	}
	header->signature[3] = '\0';

	if (fread(&(header->version), sizeof(uint8_t), 1, file) != 1) {
		if (ferror(file)) {
			return -2;
		}
		return -2;
	}

	return 0;
}

static int rle_read_packet(FILE *file, RLE_PACKET *pkt)
{
	if (fread(&(pkt->count), sizeof(uint8_t), 1, file) != 1) {
		if (feof(file)) {
			return -1;
		}
		if (ferror(file)) {
			return -2;
		}
		return -2;
	}

	if (fread(&(pkt->data), sizeof(uint8_t), 1, file) != 1) {
		if (ferror(file)) {
			return -2;
		}
		return -2;
	}

	return 0;
}

