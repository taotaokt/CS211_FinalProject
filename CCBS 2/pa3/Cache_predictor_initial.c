#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

// CS211 Final Project
// Group Number: 7
// Group Members: Zhitong Guo and Catherine Liu
// Initial Cache Predictor

// The default addresses are from 0 to 4095.
// The default cache block size is 4 bytes.
#define MAX_ADDRESS 4096
#define BLOCK_SIZE 4
#define INITIAL_CAPACITY 1024

int main(int argc, char *argv[])
{
    FILE *input_file;
    FILE *output_file;
    unsigned long long *addresses;
    size_t address_count = 0;
    size_t capacity = INITIAL_CAPACITY;
    long stride;
    long prediction;
    char line[128];

    // Check that the user gives an input file and an output file.
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }

    input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        fprintf(stderr, "Could not open input file: %s\n", argv[1]);
        return 1;
    }

    addresses = malloc(capacity * sizeof(*addresses));
    if (addresses == NULL) {
        fclose(input_file);
        return 1;
    }

    // Read all addresses from the input file
    while (fgets(line, sizeof(line), input_file) != NULL) {
        char *end_pointer;
        unsigned long long address;

        errno = 0;
        address = strtoull(line, &end_pointer, 0);

        if (end_pointer == line || errno != 0) {
            continue;
        }

        if (address_count == capacity) {
            unsigned long long *resized_addresses;
            capacity *= 2;
            resized_addresses = realloc(addresses, capacity * sizeof(*addresses));
            if (resized_addresses == NULL) {
                free(addresses);
                fclose(input_file);
                return 1;
            }
            addresses = resized_addresses;
        }

        addresses[address_count] = address;
        address_count++;
    }

    if (ferror(input_file)) {
        fprintf(stderr, "An error occurred while reading the input file.\n");
        free(addresses);
        fclose(input_file);
        return 1;
    }

    fclose(input_file);

    if (address_count == 0) {
        free(addresses);
        fprintf(stderr, "The input file does not contain an address.\n");
        return 1;
    }

    output_file = fopen(argv[2], "w");
    if (output_file == NULL) {
        free(addresses);
        fprintf(stderr, "Could not open output file: %s\n", argv[2]);
        return 1;
    }

    // Generate one prediction per input address.
    // Use simple stride-based prediction.
    for (size_t i = 0; i < address_count; i++) {
        if (i == 0) {
            // First address: predict next block
            prediction = (long)addresses[0] + BLOCK_SIZE;
        } else {
            // Compute stride and predict next
            stride = (long)addresses[i] - (long)addresses[i - 1];
            prediction = (long)addresses[i] + stride;
        }

        // Keep prediction within valid range
        prediction = prediction % MAX_ADDRESS;
        if (prediction < 0) {
            prediction = prediction + MAX_ADDRESS;
        }

        fprintf(output_file, "%ld\n", prediction);
    }

    if (fclose(output_file) != 0) {
        fprintf(stderr, "Could not finish writing the output file.\n");
        free(addresses);
        return 1;
    }

    free(addresses);
    return 0;
}
