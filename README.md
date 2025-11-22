# RLE Codec

An implementation of a Run-Length Encoding (RLE) Codec in C, capable of compressing and decompressing binary files via the CLI.

## About the RLE Algorithm

**Run-Length Encoding (RLE)** is a simple and often inefficient compression algorithm based on eliminating redundant bytes placed sequentially.

### How It Works

The core idea is to track repeated bytes. Instead of writing a repeated byte multiple times, the algorithm writes a packet that declares: _"This byte was repeated X times"_.

**Example:**

If a file contains 1,000 'A's followed by 1,000 'B's, this algorithm can reduce those ~2KB of raw data into just a few bytes using these packets.

### Technical Implementation

In this implementation, the file begins with a **File Header** (containing Signature and Version) to ensure integrity. Following the header, the data is encoded in packets, where each packet stores an **8-bit count variable**.

- **Constraint:** Since the counter is an unsigned 8-bit integer (`uint8_t`), the maximum count is **255**.
- **Overflow Handling:** If a sequence of repeated bytes exceeds 255, a new packet is automatically created to continue the sequence.

### Drawbacks

The simplicity of this algorithm is also its main weakness. In the **worst-case scenario** (data with high entropy and no repetitions), the file size could potentially **double**, as every single byte would require its own 1-byte counter prefix.

## Compilation

To compile the project, run the following command:

```bash
gcc main.c rle.c -o rle
```

## Usage

To use the tool, run the executable with the following syntax:

```bash
./rle <mode> <file_path>
```

**Arguments:**

- `<mode>`: Use `-c` for compression or `-d` for decompression.
- `<file_path>`: The path for the target file.

**Example:**

```bash
./rle -c sample.bmp
```

## Testing (Best Case Scenario)

To verify the maximum efficiency of the algorithm (compressing highly repetitive files), you can generate a test file using Python directly in your terminal.

```bash
# 1. Generate a file with 2000 bytes (1000 'A's followed by 1000 'B's)
python3 -c "print('A'*1000 + 'B'*1000, end='')" > best_case.txt

# 2. Compress the file
./rle -c best_case.txt

# 3. Verify the results (Size should drop from 2.0K to just a few bytes)
ls -lh best_case.txt*
```
