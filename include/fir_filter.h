#ifndef FIR_FILTER_H
#define FIR_FILTER_H


/**
 * @brief Enum for filter types.
 */
typedef enum {
    LOW_PASS,  /**< Low-pass filter */
    HIGH_PASS  /**< High-pass filter */
} FilterType;

/**
 * @brief Enum for window types.
 */
typedef enum {
    RECT,         /**< Rectangular window */
    HANNING,      /**< Hanning window */
    HAMMING,      /**< Hamming window */
    BLACKMAN,     /**< Blackman window */
    KAISER_B6,    /**< Kaiser window with beta=6 */
    KAISER_B8,    /**< Kaiser window with beta=8 */
    KAISER_B10    /**< Kaiser window with beta=10 */
} WindowType;

/**
 * @brief Struct for FIR filter configuration.
 */
typedef struct {
    FilterType type;        /**< Type of filter */
    WindowType window;      /**< Type of window */
    float cutoff_freq;      /**< Cutoff frequency in Hz */
    int kernel_length;      /**< Length of the filter kernel */
    float sample_rate;      /**< Sampling rate in Hz */
    float *coefficients;    /**< Filter coefficients */
} FIRFilter;

/**
 * @brief Creates a FIR filter.
 *
 * @param type Type of filter
 * @param window Type of window
 * @param cutoff_freq Cutoff frequency in Hz
 * @param kernel_length Length of the filter kernel
 * @param sample_rate Sampling rate in Hz
 * @return Pointer to the created FIRFilter
 */
FIRFilter *create_fir_filter(
        FilterType type,
        WindowType window,
        float cutoff_freq,
        int kernel_length,
        float sample_rate
);

/**
 * @brief Applies the FIR filter to an input signal.
 *
 * @param filter Pointer to the FIR filter
 * @param input_signal Pointer to the input signal array
 * @param output_signal Pointer to the output signal array
 * @param signal_length Length of the input signal
 */
void apply_fir_filter(
        FIRFilter *filter,
        const float *input_signal,
        float *output_signal,
        int signal_length
);

/**
 * @brief Destroys a FIR filter.
 *
 * @param filter Pointer to the FIR filter to be destroyed
 */
void destroy_fir_filter(FIRFilter *filter);


#endif // FIR_FILTER_H