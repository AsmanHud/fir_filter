#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "fir_filter_cli.h"
#include "fir_filter.h"


void print_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s create <filter_type> <window_type> <cutoff_freq> <kernel_length> <sample_rate> <output_file>\n", prog_name);
    printf("  %s apply <input_file> <filter_file> <output_file>\n", prog_name);
    printf("  %s destroy <filter_file>\n", prog_name);
    printf("\n");
    printf("Commands:\n");
    printf("  create    Create a FIR filter and save it to a file\n");
    printf("  apply     Apply a FIR filter to an input signal\n");
    printf("  destroy   Destroy a FIR filter (delete the filter file)\n");
    printf("\n");
    printf("Options:\n");
    printf("  <filter_type>   : lowpass or highpass\n");
    printf("  <window_type>   : rect, hanning, hamming, blackman, kaiser_b6, kaiser_b8, kaiser_b10\n");
    printf("  <cutoff_freq>   : Cutoff frequency in Hz\n");
    printf("  <kernel_length> : Kernel length (odd integer)\n");
    printf("  <sample_rate>   : Sample rate in Hz\n");
    printf("  <input_file>    : Path to input signal file (text file with one float per line)\n");
    printf("  <output_file>   : Path to output signal file (text file with one float per line)\n");
    printf("  <filter_file>   : Path to filter file (binary file to save/load the filter)\n");
}

// Below are CLI argument parser functions

static FilterType parse_filter_type(const char *arg) {
    if (strcmp(arg, "lowpass") == 0) return LOW_PASS;
    if (strcmp(arg, "highpass") == 0) return HIGH_PASS;
    fprintf(stderr, "Invalid filter type: %s\n", arg);
    exit(EXIT_FAILURE);
}

static WindowType parse_window_type(const char *arg) {
    if (strcmp(arg, "rect") == 0) return RECT;
    if (strcmp(arg, "hanning") == 0) return HANNING;
    if (strcmp(arg, "hamming") == 0) return HAMMING;
    if (strcmp(arg, "blackman") == 0) return BLACKMAN;
    if (strcmp(arg, "kaiser_b6") == 0) return KAISER_B6;
    if (strcmp(arg, "kaiser_b8") == 0) return KAISER_B8;
    if (strcmp(arg, "kaiser_b10") == 0) return KAISER_B10;
    fprintf(stderr, "Invalid window type: %s\n", arg);
    exit(EXIT_FAILURE);
}

static float parse_cutoff_freq(const char *arg) {
    char *endptr;
    errno = 0;

    float cutoff_freq = strtof(arg, &endptr);
    if (errno != 0 || endptr == arg) {
        fprintf(stderr, "Invalid cutoff frequency: %s\n", arg);
        exit(EXIT_FAILURE);
    } else {
        return cutoff_freq;
    }
}

static int parse_kernel_length(const char *arg) {
    char *endptr;
    errno = 0;

    int kernel_length = (int) strtol(arg, &endptr, 10);
    if (errno != 0 || endptr == arg) {
        fprintf(stderr, "Invalid kernel length: %s\n", arg);
        exit(EXIT_FAILURE);
    } else {
        return kernel_length;
    }
}

static float parse_sample_rate(const char *arg) {
    char *endptr;
    errno = 0;

    float sample_rate = strtof(arg, &endptr);
    if (errno != 0 || endptr == arg) {
        fprintf(stderr, "Invalid sample rate: %s\n", arg);
        exit(EXIT_FAILURE);
    } else {
        return sample_rate;
    }
}

// Save the filter data into a binary file to be reused later
static void save_filter_to_file(const char *filename, FIRFilter *filter) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open filter file for writing: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fwrite(&filter->type, sizeof(FilterType), 1, file);
    fwrite(&filter->window, sizeof(WindowType), 1, file);
    fwrite(&filter->cutoff_freq, sizeof(float), 1, file);
    fwrite(&filter->kernel_length, sizeof(int), 1, file);
    fwrite(&filter->sample_rate, sizeof(float), 1, file);
    fwrite(filter->coefficients, sizeof(float), filter->kernel_length, file);

    fclose(file);
}

// Load the filter from a binary filter file
static FIRFilter *load_filter_from_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open filter file for reading: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    FIRFilter *filter = (FIRFilter *) malloc(sizeof(FIRFilter));
    if (filter == NULL) {
        fprintf(stderr, "Memory allocation failed for FIRFilter\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fread(&filter->type, sizeof(FilterType), 1, file);
    fread(&filter->window, sizeof(WindowType), 1, file);
    fread(&filter->cutoff_freq, sizeof(float), 1, file);
    fread(&filter->kernel_length, sizeof(int), 1, file);
    fread(&filter->sample_rate, sizeof(float), 1, file);

    filter->coefficients = (float *) malloc(filter->kernel_length * sizeof(float));
    if (filter->coefficients == NULL) {
        fprintf(stderr, "Memory allocation failed for coefficients\n");
        free(filter);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fread(filter->coefficients, sizeof(float), filter->kernel_length, file);

    fclose(file);
    return filter;
}

// Read the input signal from the text input file
static float *read_signal_from_file(const char *filename, int *length) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open input file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    float *signal = NULL;
    int count = 0;
    char line[256];

    // Loop to read each line from the file
    while (fgets(line, sizeof(line), file)) {
        char *endptr;
        errno = 0;

        // Convert the line to a float
        float value = strtof(line, &endptr);

        // Check for conversion errors
        if (errno != 0 || endptr == line || (*endptr != '\n' && *endptr != '\0' && *endptr != '\r')) {
            fprintf(stderr, "Invalid float value in input file: %s", line);
            free(signal);
            fclose(file);
            exit(EXIT_FAILURE);
        }

        // Reallocate memory to store the new float value
        float *temp = realloc(signal, (count + 1) * sizeof(float));
        if (temp == NULL) {
            fprintf(stderr, "Memory allocation failed while reading input signal\n");
            free(signal);
            fclose(file);
            exit(EXIT_FAILURE);
        }
        signal = temp;
        signal[count++] = value;
    }

    fclose(file);
    *length = count;
    return signal;
}

// Write the calculated signal to the text output file
static void write_signal_to_file(const char *filename, const float *signal, int length) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open output file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < length; ++i) {
        fprintf(file, "%f\n", signal[i]);
    }

    fclose(file);
}


void handle_create_fir_filter(int argc, char *argv[]) {
    if (argc != 8) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    FilterType filter_type = parse_filter_type(argv[2]);
    WindowType window_type = parse_window_type(argv[3]);
    float cutoff_freq = parse_cutoff_freq(argv[4]);
    int kernel_length = parse_kernel_length(argv[5]);
    float sample_rate = parse_sample_rate(argv[6]);
    const char *output_file = argv[7];

    FIRFilter *filter = create_fir_filter(filter_type, window_type, cutoff_freq, kernel_length, sample_rate);
    if (filter == NULL) {
        fprintf(stderr, "Failed to create FIR filter\n");
        exit(EXIT_FAILURE);
    }

    save_filter_to_file(output_file, filter);
    destroy_fir_filter(filter);
}


void handle_apply_fir_filter(int argc, char *argv[]) {
    if (argc != 5) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *input_file = argv[2];
    const char *filter_file = argv[3];
    const char *output_file = argv[4];

    int signal_length;
    float *input_signal = read_signal_from_file(input_file, &signal_length);
    float *output_signal = (float *) malloc(signal_length * sizeof(float));
    if (!output_signal) {
        fprintf(stderr, "Memory allocation failed for output signal\n");
        free(input_signal);
        exit(EXIT_FAILURE);
    }

    FIRFilter *filter = load_filter_from_file(filter_file);

    apply_fir_filter(filter, input_signal, output_signal, signal_length);
    write_signal_to_file(output_file, output_signal, signal_length);

    // Free all the memory held up by the dynamically allocated memory
    free(input_signal);
    free(output_signal);
    destroy_fir_filter(filter);
}


void handle_destroy_fir_filter(int argc, char *argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filter_file = argv[2];
    if (remove(filter_file) != 0) {
        fprintf(stderr, "Failed to delete filter file: %s\n", filter_file);
    } else {
        printf("Successfully deleted filter file: %s\n", filter_file);
    }
}