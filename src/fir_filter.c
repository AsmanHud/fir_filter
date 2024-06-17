#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "fir_filter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BESSEL_FUNCTION_APPROXIMATION 25
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Generate a factorial list of values up to the specified number
// Precision starts to decay after 14!, but it doesn't matter for larger numbers
// because the factorials are used for division
static void generate_factorials(int up_to, float *factorial_list) {
    float factorial_term = 1.0f;
    for (int i = 1; i <= up_to; ++i) {
        factorial_term *= (float) i;
        factorial_list[i - 1] = factorial_term;
    }
}

// Function to calculate the modified zero order Bessel function of the 1st kind (i.e. I0(x))
// for the Kaiser window calculation
static float I0(float x, const float *factorial_list) {
    float result = 1.0f;
    float half_x = x / 2.0f;
    for (int j = 1; j <= BESSEL_FUNCTION_APPROXIMATION; ++j) {
        float term = powf(half_x, (float) j) / factorial_list[j - 1];
        result += powf(term, 2.0f);
    }
    return result;
}

// Function to calculate the Kaiser window terms
static float kaiser_window_function(
        float beta_param,
        float I0_beta,
        const float *factorial_list,
        int kernel_length,
        int n
) {
    // Normalized window position: (2 * n) / (N - 1)
    float normalized_win_pos = (float) (2 * n) / (float) (kernel_length - 1);
    // Calculate the term which will be plugged into the Bessel function
    float nominator_term = beta_param * sqrtf(1.0f - powf(normalized_win_pos, 2.0f));
    return I0(nominator_term, factorial_list) / I0_beta;
}

// Calculate the window function term based on the window type
static float window_function(WindowType window, int kernel_length, int n) {
    // Normalized angular position: (2 * pi * n) / (N - 1)
    float normalized_ang_pos = (float) (2 * n) * (float) M_PI / (float) (kernel_length - 1);
    // Window function definitions
    switch (window) {
        case RECT:
            return 1.0f;
        case HANNING:
            return 0.5f + 0.5f * cosf(normalized_ang_pos);
        case HAMMING:
            return 0.54f + 0.46f * cosf(normalized_ang_pos);
        case BLACKMAN:
            return 0.42f + 0.5f * cosf(normalized_ang_pos) + 0.08f * cosf(2 * normalized_ang_pos);
        default:
            return 1.0f; // default to a rectangular window if somehow window is unspecified
    }
}

// Calculate the coefficients for the filter
static void generate_sinc(FIRFilter *filter) {
    // Calculate the normalized cutoff frequency
    float normalized_cutoff_freq = 2.0f * filter->cutoff_freq / filter->sample_rate;
    // Define the range of values for n as half of the interval count
    int half_M = (filter->kernel_length - 1) / 2;
    // Additional values for the Kaiser window calculations
    int is_window_kaiser = filter->window == KAISER_B6 || filter->window == KAISER_B8 || filter->window == KAISER_B10;
    float *factorial_list = NULL;
    float beta_param = 0.0f;
    float I0_beta = 0.0f;
    if (is_window_kaiser) {
        factorial_list = (float *) malloc(BESSEL_FUNCTION_APPROXIMATION * sizeof(float));
        if (factorial_list == NULL) {
            fprintf(stderr, "Memory allocation for Kaiser window calculation failed.\n");
            free(filter->coefficients);
            free(filter);
            filter = NULL;
            return;
        }
        generate_factorials(BESSEL_FUNCTION_APPROXIMATION, factorial_list);
        if (filter->window == KAISER_B6) beta_param = 6.0f;
        if (filter->window == KAISER_B8) beta_param = 8.0f;
        if (filter->window == KAISER_B10) beta_param = 10.0f;
        I0_beta = I0(beta_param, factorial_list);
    }

    // Generate the filter coefficients according to the filter and window type
    for (int n = -half_M; n <= half_M; ++n) {
        // Use a pointer for ease of access to the filter coefficients
        // Adding half_M corresponds to time shifting in order to make the filter causal
        float *coefficient_ptr = filter->coefficients + n + half_M;
        // Calculate the float value for pi * n, to avoid multiple conversions in the calculation of sinc
        float pi_times_n = (float) M_PI * (float) n;

        // Find the pure sinc function coefficients
        if (n == 0) {
            *coefficient_ptr = normalized_cutoff_freq;
        } else {
            *coefficient_ptr = sinf(normalized_cutoff_freq * pi_times_n) / pi_times_n;
        }

        // Apply the window function to the pure sinc function, to get the impulse response of the filter
        // h[n] = h[n] * w[n]
        if (is_window_kaiser) {
            *coefficient_ptr *= kaiser_window_function(beta_param, I0_beta, factorial_list, filter->kernel_length, n);
        } else {
            *coefficient_ptr *= window_function(filter->window, filter->kernel_length, n);
        }

        // To obtain HP filter from the LP, we need to perform spectral inversion of the IR.
        // The spectral inversion of a filter h[n] is defined as follows:
        // 1) Change the sign of each value in h[n]
        // 2) Add one to the value in the center.
        if (filter->type == HIGH_PASS) {
            *coefficient_ptr *= -1;
            if (n == 0) *coefficient_ptr += 1;
        }
    }

    // Free the factorial list, if it was used (i.e. if the window type was Kaiser)
    if (factorial_list != NULL) {
        free(factorial_list);
    }
}

// API endpoint to create a FIR filter
FIRFilter *create_fir_filter(
        FilterType type,
        WindowType window,
        float cutoff_freq,
        int kernel_length,
        float sample_rate
) {
    // Validate the input parameters
    if (cutoff_freq <= 0 || sample_rate <= 0 || kernel_length <= 0) {
        fprintf(stderr, "create_fir_filter: One of the input parameters was zero or negative.\n"
                        "Please ensure that all the input parameters for the filter are non-zero and positive.\n");
        return NULL;
    }
    // Ensure that the kernel length is odd, this enhances the filter efficiency
    if ((kernel_length & 1) == 0) {
        kernel_length += 1;
    }

    // Allocate memory for the filter, and check if the allocation was successful
    FIRFilter *filter = (FIRFilter *) malloc(sizeof(FIRFilter));
    if (filter == NULL) {
        fprintf(stderr, "Failed to allocate memory for FIRFilter\n");
        return NULL;
    }

    filter->type = type;
    filter->window = window;
    filter->cutoff_freq = cutoff_freq;
    filter->kernel_length = kernel_length;
    filter->sample_rate = sample_rate;

    // Allocate memory for the coefficients, and check if the allocation was successful
    filter->coefficients = (float *) malloc(kernel_length * sizeof(float));
    if (filter->coefficients == NULL) {
        fprintf(stderr, "Failed to allocate memory for coefficients of the filter\n");
        free(filter);
        return NULL;
    }

    // Calculate the filter coefficients
    generate_sinc(filter);
    return filter;
}

// API endpoint for applying the filter
void apply_fir_filter(
        FIRFilter *filter,
        const float *input_signal,
        float *output_signal,
        int signal_length
) {
    // Check for valid input parameters
    if (filter == NULL || filter->coefficients == NULL || input_signal == NULL || output_signal == NULL ||
        signal_length < 0) {
        fprintf(stderr, "apply_fir_filter: Invalid input parameter(s).\n");
        return;
    }

    // For simplicity, the convolution is currently implemented using the flip-and-shift method
    // instead of the optimal method computation-wise using the Fast Fourier Transform algorithm

    // The last kernel_length-1 values of the convolution are truncated (not calculated),
    // so that the output signal length is equal to the length of the input signal

    // The convolution logic
    // Calculation of the first kernel_length-1 values of the output
    // or all of the output values if the kernel is larger than the signal
    for (int i = 0; i < MIN(filter->kernel_length - 1, signal_length); ++i) {
        output_signal[i] = 0;
        for (int j = 0; j < i + 1; ++j) {
            output_signal[i] += filter->coefficients[j] * input_signal[i - j];
        }
    }
    // Calculation of the rest of output, if the signal is larger than the kernel
    for (int i = filter->kernel_length - 1; i < signal_length; ++i) {
        output_signal[i] = 0;
        for (int j = 0; j < filter->kernel_length; ++j) {
            output_signal[i] += filter->coefficients[j] * input_signal[i - j];
        }
    }
}

// API endpoint to free the memory held by the filter
void destroy_fir_filter(FIRFilter *filter) {
    if (filter != NULL) {
        free(filter->coefficients);
        free(filter);
    }
}