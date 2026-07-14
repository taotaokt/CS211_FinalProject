#include <stdio.h>

/*
 * The simulator uses addresses from 0 through 4095 by default.
 * The default cache block size is 4 bytes.
 */
#define MAX_ADDRESS 4096
#define BLOCK_SIZE 4

int main(int argc, char *argv[])
{
    FILE *input_file;
    FILE *output_file;
    long current_address;
    long previous_address = 0;
    long last_address = 0;
    long address_count = 0;
    long stride;
    long prediction;

    /* The simulator must provide an input file and an output file. */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }

    input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr, "Could not open input file: %s\n", argv[1]);
        return 1;
    }

    /*
     * %li accepts both decimal addresses and hexadecimal addresses that
     * begin with 0x. Only the final two addresses need to be remembered.
     */
    while (fscanf(input_file, "%li", &current_address) == 1) {
        previous_address = last_address;
        last_address = current_address;
        address_count++;
    }

    if (ferror(input_file)) {
        fprintf(stderr, "An error occurred while reading the input file.\n");
        fclose(input_file);
        return 1;
    }

    fclose(input_file);

    if (address_count == 0) {
        fprintf(stderr, "The input file does not contain an address.\n");
        return 1;
    }

    /*
     * If at least two addresses exist, continue their most recent stride.
     * Example: 16, 20 produces a stride of 4 and predicts 24.
     * With only one address, move forward by one default cache block.
     */
    if (address_count >= 2) {
        stride = last_address - previous_address;
        prediction = last_address + stride;
    } else {
        prediction = last_address + BLOCK_SIZE;
    }

    /* Keep the prediction inside the default address range 0-4095. */
    prediction = prediction % MAX_ADDRESS;
    if (prediction < 0) {
        prediction = prediction + MAX_ADDRESS;
    }

    output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
        fprintf(stderr, "Could not open output file: %s\n", argv[2]);
        return 1;
    }

    /* The output file must contain addresses only, one address per line. */
    fprintf(output_file, "%ld\n", prediction);

    if (fclose(output_file) != 0) {
        fprintf(stderr, "Could not finish writing the output file.\n");
        return 1;
    }

    return 0;
}
