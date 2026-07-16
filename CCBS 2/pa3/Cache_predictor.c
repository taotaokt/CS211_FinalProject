#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * CS211 Final Project
 * Final Cache Predictor
 *
 * Group Number:
 * 7
 *
 * Group Members:
 * Zhitong Guo
 * Catherine Liu
 *
 * Prediction strategies:
 * 1. Repeating-pattern detection
 * 2. Stable-stride detection
 * 3. Address-transition prediction
 * 4. Recent-frequency prediction
 *
 * Predicted addresses are ranked, deduplicated, and written
 * one address per line.
 */

#define INITIAL_CAPACITY 1024
#define MAX_CANDIDATES 256
#define MAX_PREDICTIONS 6
#define MAX_PERIOD 16
#define STRIDE_WINDOW 8
#define FREQUENCY_WINDOW 64

typedef struct
{
    unsigned long long address;
    int score;
} Candidate;

/*
 * Add a candidate prediction.
 *
 * If the address already exists, retain the higher score.
 */
static void add_candidate(
    Candidate candidates[],
    size_t *candidate_count,
    unsigned long long address,
    int score)
{
    size_t index;

    for (index = 0; index < *candidate_count; index++)
    {
        if (candidates[index].address == address)
        {
            if (score > candidates[index].score)
            {
                candidates[index].score = score;
            }

            return;
        }
    }

    if (*candidate_count < MAX_CANDIDATES)
    {
        candidates[*candidate_count].address = address;
        candidates[*candidate_count].score = score;
        (*candidate_count)++;
    }
}

/*
 * Sort candidates from highest score to lowest score.
 */
static int compare_candidates(const void *left, const void *right)
{
    const Candidate *candidate_left = left;
    const Candidate *candidate_right = right;

    if (candidate_left->score < candidate_right->score)
    {
        return 1;
    }

    if (candidate_left->score > candidate_right->score)
    {
        return -1;
    }

    if (candidate_left->address > candidate_right->address)
    {
        return 1;
    }

    if (candidate_left->address < candidate_right->address)
    {
        return -1;
    }

    return 0;
}

/*
 * Detect whether the final two blocks contain the same pattern.
 *
 * Example:
 * 10 20 30 10 20 30
 *
 * The next likely addresses are:
 * 10 20 30
 */
static void detect_repeating_pattern(
    const unsigned long long addresses[],
    size_t address_count,
    Candidate candidates[],
    size_t *candidate_count)
{
    size_t period;

    for (period = 1;
         period <= MAX_PERIOD && address_count >= period * 2;
         period++)
    {
        size_t offset;
        int matches = 1;

        for (offset = 0; offset < period; offset++)
        {
            size_t first_index =
                address_count - (period * 2) + offset;

            size_t second_index =
                address_count - period + offset;

            if (addresses[first_index] != addresses[second_index])
            {
                matches = 0;
                break;
            }
        }

        if (matches)
        {
            for (offset = 0; offset < period; offset++)
            {
                int score = 120 - (int)offset;

                add_candidate(
                    candidates,
                    candidate_count,
                    addresses[address_count - period + offset],
                    score);
            }

            /*
             * Prefer the shortest repeating pattern.
             */
            break;
        }
    }
}

/*
 * Find the most common stride among the most recent accesses.
 */
static void detect_stable_stride(
    const unsigned long long addresses[],
    size_t address_count,
    Candidate candidates[],
    size_t *candidate_count)
{
    long long differences[STRIDE_WINDOW];
    size_t difference_count;
    size_t start_index;
    size_t index;

    long long best_stride = 0;
    size_t best_frequency = 0;

    if (address_count < 2)
    {
        return;
    }

    difference_count = address_count - 1;

    if (difference_count > STRIDE_WINDOW)
    {
        difference_count = STRIDE_WINDOW;
    }

    start_index = address_count - difference_count;

    for (index = 0; index < difference_count; index++)
    {
        differences[index] =
            (long long)addresses[start_index + index] -
            (long long)addresses[start_index + index - 1];
    }

    for (index = 0; index < difference_count; index++)
    {
        size_t compare_index;
        size_t frequency = 0;

        for (compare_index = 0;
             compare_index < difference_count;
             compare_index++)
        {
            if (differences[index] == differences[compare_index])
            {
                frequency++;
            }
        }

        if (frequency > best_frequency)
        {
            best_frequency = frequency;
            best_stride = differences[index];
        }
    }

    /*
     * A stride repeated at least twice is treated as stable.
     */
    if (best_frequency >= 2 && best_stride != 0)
    {
        int prediction_number;

        for (prediction_number = 1;
             prediction_number <= 4;
             prediction_number++)
        {
            long long predicted_address =
                (long long)addresses[address_count - 1] +
                best_stride * prediction_number;

            if (predicted_address >= 0)
            {
                add_candidate(
                    candidates,
                    candidate_count,
                    (unsigned long long)predicted_address,
                    100 +
                        (int)best_frequency -
                        prediction_number);
            }
        }
    }
    else
    {
        /*
         * Fall back to the most recent stride with lower confidence.
         */
        long long recent_stride =
            (long long)addresses[address_count - 1] -
            (long long)addresses[address_count - 2];

        long long predicted_address =
            (long long)addresses[address_count - 1] +
            recent_stride;

        if (recent_stride != 0 && predicted_address >= 0)
        {
            add_candidate(
                candidates,
                candidate_count,
                (unsigned long long)predicted_address,
                60);
        }
    }
}

/*
 * Predict addresses that historically followed the current
 * final address.
 *
 * This acts as a simple first-order transition predictor.
 */
static void detect_transitions(
    const unsigned long long addresses[],
    size_t address_count,
    Candidate candidates[],
    size_t *candidate_count)
{
    unsigned long long current_address;
    size_t index;

    if (address_count < 2)
    {
        return;
    }

    current_address = addresses[address_count - 1];

    for (index = 0; index + 1 < address_count; index++)
    {
        if (addresses[index] == current_address)
        {
            unsigned long long next_address = addresses[index + 1];
            size_t compare_index;
            size_t frequency = 0;

            for (compare_index = 0;
                 compare_index + 1 < address_count;
                 compare_index++)
            {
                if (addresses[compare_index] == current_address &&
                    addresses[compare_index + 1] == next_address)
                {
                    frequency++;
                }
            }

            add_candidate(
                candidates,
                candidate_count,
                next_address,
                80 + (int)frequency * 5);
        }
    }
}

/*
 * Add addresses that occur repeatedly in the recent history.
 */
static void detect_recent_frequency(
    const unsigned long long addresses[],
    size_t address_count,
    Candidate candidates[],
    size_t *candidate_count)
{
    size_t window_start;
    size_t index;

    if (address_count > FREQUENCY_WINDOW)
    {
        window_start = address_count - FREQUENCY_WINDOW;
    }
    else
    {
        window_start = 0;
    }

    for (index = window_start; index < address_count; index++)
    {
        size_t compare_index;
        size_t frequency = 0;

        for (compare_index = window_start;
             compare_index < address_count;
             compare_index++)
        {
            if (addresses[index] == addresses[compare_index])
            {
                frequency++;
            }
        }

        /*
         * Only repeated addresses are considered useful
         * frequency-based candidates.
         */
        if (frequency >= 2)
        {
            add_candidate(
                candidates,
                candidate_count,
                addresses[index],
                40 + (int)frequency);
        }
    }
}

int main(int argc, char **argv)
{
    FILE *input;
    FILE *output;

    unsigned long long *addresses;
    size_t address_count = 0;
    size_t capacity = INITIAL_CAPACITY;

    Candidate candidates[MAX_CANDIDATES];
    size_t candidate_count = 0;

    char line[128];

    if (argc != 3)
    {
        return EXIT_FAILURE;
    }

    input = fopen(argv[1], "r");

    if (input == NULL)
    {
        return EXIT_FAILURE;
    }

    output = fopen(argv[2], "w");

    if (output == NULL)
    {
        fclose(input);
        return EXIT_FAILURE;
    }

    addresses = malloc(capacity * sizeof(*addresses));

    if (addresses == NULL)
    {
        fclose(input);
        fclose(output);
        return EXIT_FAILURE;
    }

    while (fgets(line, sizeof(line), input) != NULL)
    {
        char *end_pointer;
        unsigned long long address;

        errno = 0;
        address = strtoull(line, &end_pointer, 0);

        if (end_pointer == line || errno != 0)
        {
            continue;
        }

        if (address_count == capacity)
        {
            unsigned long long *resized_addresses;

            capacity *= 2;

            resized_addresses =
                realloc(addresses, capacity * sizeof(*addresses));

            if (resized_addresses == NULL)
            {
                free(addresses);
                fclose(input);
                fclose(output);
                return EXIT_FAILURE;
            }

            addresses = resized_addresses;
        }

        addresses[address_count] = address;
        address_count++;
    }

    if (address_count == 0)
    {
        free(addresses);
        fclose(input);
        fclose(output);
        return EXIT_SUCCESS;
    }

    /*
     * Generate one prediction per input address.
     * For each position i in the history, predict what comes next
     * based on the patterns up to position i.
     */
    for (size_t i = 0; i < address_count; i++)
    {
        candidate_count = 0;

        if (i > 0)
        {
            detect_repeating_pattern(
                addresses,
                i,
                candidates,
                &candidate_count);

            detect_stable_stride(
                addresses,
                i,
                candidates,
                &candidate_count);

            detect_transitions(
                addresses,
                i,
                candidates,
                &candidate_count);

            detect_recent_frequency(
                addresses,
                i,
                candidates,
                &candidate_count);

            add_candidate(
                candidates,
                &candidate_count,
                addresses[i - 1],
                25);

            if (i >= 2)
            {
                add_candidate(
                    candidates,
                    &candidate_count,
                    addresses[i - 2],
                    20);
            }
        }
        else
        {
            add_candidate(
                candidates,
                &candidate_count,
                addresses[0] + 4,
                10);
        }

        if (candidate_count == 0)
        {
            add_candidate(
                candidates,
                &candidate_count,
                addresses[i] + 4,
                10);
        }

        qsort(
            candidates,
            candidate_count,
            sizeof(candidates[0]),
            compare_candidates);

        fprintf(output, "%llu\n", candidates[0].address);
    }

    free(addresses);
    fclose(input);
    fclose(output);

    return EXIT_SUCCESS;
}
