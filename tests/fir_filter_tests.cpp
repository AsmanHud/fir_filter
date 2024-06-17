#include <limits>
#include <iostream>
#include "gtest/gtest.h"

extern "C" {
#include "fir_filter.h"
}

// Helper function to print filter coefficients
void print_float_array(const float *array, int arr_size) {
    for (int i = 0; i < arr_size; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}

// Helper function to compare two float arrays
void compare_arrays(const float *arr1, const float *arr2, int length, float tolerance = 1e-5) {
    for (int i = 0; i < length; ++i) {
        ASSERT_NEAR(arr1[i], arr2[i], tolerance);
    }
}


// =================================
// = UNIT TESTS: create_fir_filter =
// =================================

// Base test: create and destroy a filter
TEST(FIRFilterCreateTest, CreateDestroy) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, HANNING, 1000.0f, 11, 8000.0f);
    ASSERT_NE(filter, nullptr);
    ASSERT_NE(filter->coefficients, nullptr);
    destroy_fir_filter(filter);
}

// First (series of) test(s): check if the calculated coefficients match
// with the expected ones

struct TestCase {
    FilterType filterType;
    WindowType windowType;
    float cutoffFreq;
    int kernelLength;
    float sampleRate;
    std::vector<float> expectedCoefficients;
};

// Expected coefficients were calculated using firwin from scipy.signal
std::vector<TestCase> testCases = {
        // Low-pass filters
        {LOW_PASS,  RECT,       1000.0f, 11, 8000.0f, {-0.04501582, 0.00000000,  0.07502636,  0.15915494,  0.22507908,  0.25000000, 0.22507908,  0.15915494,  0.07502636,  0.00000000,  -0.04501582}},
        {LOW_PASS,  HANNING,    1000.0f, 11, 8000.0f, {-0.00000000, 0.00000000,  0.02592097,  0.10416826,  0.20358594,  0.25000000, 0.20358594,  0.10416826,  0.02592097,  0.00000000,  -0.00000000}},
        {LOW_PASS,  HAMMING,    1000.0f, 11, 8000.0f, {-0.00360127, 0.00000000,  0.02984940,  0.10856720,  0.20530539,  0.25000000, 0.20530539,  0.10856720,  0.02984940,  0.00000000,  -0.00360127}},
        {LOW_PASS,  BLACKMAN,   1000.0f, 11, 8000.0f, {0.00000000,  0.00000000,  0.01506305,  0.08113514,  0.19114387,  0.25000000, 0.19114387,  0.08113514,  0.01506305,  0.00000000,  0.00000000}},
        {LOW_PASS,  KAISER_B6,  1000.0f, 11, 8000.0f, {-0.00066954, 0.00000000,  0.02543529,  0.10098226,  0.20153585,  0.25000000, 0.20153585,  0.10098226,  0.02543529,  0.00000000,  -0.00066954}},
        {LOW_PASS,  KAISER_B8,  1000.0f, 11, 8000.0f, {-0.00010528, 0.00000000,  0.01701424,  0.08539195,  0.19352346,  0.25000000, 0.19352346,  0.08539195,  0.01701424,  0.00000000,  -0.00010528}},
        {LOW_PASS,  KAISER_B10, 1000.0f, 11, 8000.0f, {-0.00001599, 0.00000000,  0.01139269,  0.07223330,  0.18584359,  0.25000000, 0.18584359,  0.07223330,  0.01139269,  0.00000000,  -0.00001599}},

        // High-pass filters
        {HIGH_PASS, RECT,       1000.0f, 11, 8000.0f, {0.04501582,  -0.00000000, -0.07502636, -0.15915494, -0.22507908, 0.75000000, -0.22507908, -0.15915494, -0.07502636, -0.00000000, 0.04501582}},
        {HIGH_PASS, HANNING,    1000.0f, 11, 8000.0f, {0.00000000,  -0.00000000, -0.02592097, -0.10416826, -0.20358594, 0.75000000, -0.20358594, -0.10416826, -0.02592097, -0.00000000, 0.00000000}},
        {HIGH_PASS, HAMMING,    1000.0f, 11, 8000.0f, {0.00360127,  -0.00000000, -0.02984940, -0.10856720, -0.20530539, 0.75000000, -0.20530539, -0.10856720, -0.02984940, -0.00000000, 0.00360127}},
        {HIGH_PASS, BLACKMAN,   1000.0f, 11, 8000.0f, {-0.00000000, -0.00000000, -0.01506305, -0.08113514, -0.19114387, 0.75000000, -0.19114387, -0.08113514, -0.01506305, -0.00000000, -0.00000000}},
        {HIGH_PASS, KAISER_B6,  1000.0f, 11, 8000.0f, {0.00066954,  -0.00000000, -0.02543529, -0.10098226, -0.20153585, 0.75000000, -0.20153585, -0.10098226, -0.02543529, -0.00000000, 0.00066954}},
        {HIGH_PASS, KAISER_B8,  1000.0f, 11, 8000.0f, {0.00010528,  -0.00000000, -0.01701424, -0.08539195, -0.19352346, 0.75000000, -0.19352346, -0.08539195, -0.01701424, -0.00000000, 0.00010528}},
        {HIGH_PASS, KAISER_B10, 1000.0f, 11, 8000.0f, {0.00001599,  -0.00000000, -0.01139269, -0.07223330, -0.18584359, 0.75000000, -0.18584359, -0.07223330, -0.01139269, -0.00000000, 0.00001599}}
};

void runTestCase(const TestCase &testCase) {
    FIRFilter *filter = create_fir_filter(testCase.filterType, testCase.windowType, testCase.cutoffFreq,
                                          testCase.kernelLength, testCase.sampleRate);
    ASSERT_NE(filter, nullptr);
    ASSERT_NE(filter->coefficients, nullptr);
    for (int i = 0; i < testCase.kernelLength; ++i) {
        ASSERT_NEAR(filter->coefficients[i], testCase.expectedCoefficients[i], 1e-5);
    }
    destroy_fir_filter(filter);
}

// Individual tests for low-pass filters
TEST(FIRFilterCreateTest, LowPassRectangular) { runTestCase(testCases[0]); }

TEST(FIRFilterCreateTest, LowPassHanning) { runTestCase(testCases[1]); }

TEST(FIRFilterCreateTest, LowPassHamming) { runTestCase(testCases[2]); }

TEST(FIRFilterCreateTest, LowPassBlackman) { runTestCase(testCases[3]); }

TEST(FIRFilterCreateTest, LowPassKaiserB6) { runTestCase(testCases[4]); }

TEST(FIRFilterCreateTest, LowPassKaiserB8) { runTestCase(testCases[5]); }

TEST(FIRFilterCreateTest, LowPassKaiserB10) { runTestCase(testCases[6]); }

// Individual tests for high-pass filters
TEST(FIRFilterCreateTest, HighPassRectangular) { runTestCase(testCases[7]); }

TEST(FIRFilterCreateTest, HighPassHanning) { runTestCase(testCases[8]); }

TEST(FIRFilterCreateTest, HighPassHamming) { runTestCase(testCases[9]); }

TEST(FIRFilterCreateTest, HighPassBlackman) { runTestCase(testCases[10]); }

TEST(FIRFilterCreateTest, HighPassKaiserB6) { runTestCase(testCases[11]); }

TEST(FIRFilterCreateTest, HighPassKaiserB8) { runTestCase(testCases[12]); }

TEST(FIRFilterCreateTest, HighPassKaiserB10) { runTestCase(testCases[13]); }

// Create a bunch of filters (with proper destruction of filters)
TEST(FIRFilterCreateTest, CreateLargeAmountOfFilters) {
    FIRFilter *filter = nullptr;
    int amount_of_filters = 1000;
    for (int i = 0; i < amount_of_filters; ++i) {
        if (filter != nullptr) {
            destroy_fir_filter(filter);
        }
        filter = create_fir_filter(LOW_PASS, BLACKMAN, (float) (i + 1), 2 * i + 1, (float) (i + 1000));
        ASSERT_NE(filter, nullptr);
    }
    destroy_fir_filter(filter);
}

// Create a bunch of filters (without proper destruction of filters)
TEST(FIRFilterCreateTest, CreateLargeAmountOfFiltersWithMemoryLeak) {
    FIRFilter *filter;
    int amount_of_filters = 1000;
    for (int i = 0; i < amount_of_filters; ++i) {
        // Intentionally not deleting the previous filters and creating new ones
        // This causes a memory leak (from the Valgrind memcheck)
        filter = create_fir_filter(LOW_PASS, BLACKMAN, (float) (i + 1), 2 * i + 1, (float) (i + 1000));
        ASSERT_NE(filter, nullptr);
    }
}

// Invalid values and edge cases

// A large kernel length and how much memory it takes up
TEST(FIRFilterCreateTest, LargeKernelLength) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, HANNING, 1000.0f, 50000, 8000.0f);
    if (filter != nullptr) {
        std::cout << "Created filter with a 50000 kernel length" << std::endl;

        // Calculate the memory used by the filter
        size_t filter_size = sizeof(FIRFilter);
        size_t coefficients_size = filter->kernel_length * sizeof(float);
        size_t total_size = filter_size + coefficients_size;

        std::cout << "Memory used by the filter: " << total_size << " bytes" << std::endl;
        std::cout << "  - FIRFilter struct size: " << filter_size << " bytes" << std::endl;
        std::cout << "  - Coefficients array size: " << coefficients_size << " bytes" << std::endl;

        destroy_fir_filter(filter);
    } else {
        std::cout << "Failed to create filter with a large kernel length" << std::endl;
    }
}

TEST(FIRFilterCreateTest, NegativeValues) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, BLACKMAN, -1000.0f, 11, 8000.0f);
    ASSERT_EQ(filter, nullptr);

    filter = create_fir_filter(HIGH_PASS, KAISER_B8, 1000.0f, -11, 8000.0f);
    ASSERT_EQ(filter, nullptr);

    filter = create_fir_filter(LOW_PASS, BLACKMAN, 1000.0f, 11, -8000.0f);
    ASSERT_EQ(filter, nullptr);
}

TEST(FIRFilterCreateTest, ZeroValues) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, BLACKMAN, 0.0f, 0, 0.0f);
    ASSERT_EQ(filter, nullptr);
}

// Edge cases with maximum values for each of the input variables
TEST(FIRFilterCreateTest, EdgeCases) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, BLACKMAN, std::numeric_limits<float>::max(), 11, 8000.0f);
    if (filter != nullptr && filter->coefficients != nullptr) {
        std::cout << "Created filter with max float cutoff frequency" << std::endl;
        std::cout << "Filter coefficients: ";
        print_float_array(filter->coefficients, filter->kernel_length);
        destroy_fir_filter(filter);
    } else {
        std::cout << "Failed to create filter with max float cutoff frequency" << std::endl;
    }

    filter = create_fir_filter(LOW_PASS, BLACKMAN, 1000.0f, 11, std::numeric_limits<float>::max());
    if (filter != nullptr && filter->coefficients != nullptr) {
        std::cout << "Created filter with max float sample rate" << std::endl;
        std::cout << "Filter coefficients: ";
        print_float_array(filter->coefficients, filter->kernel_length);
        destroy_fir_filter(filter);
    } else {
        std::cout << "Failed to create filter with max float sample rate" << std::endl;
    }

    filter = create_fir_filter(LOW_PASS, BLACKMAN, std::numeric_limits<float>::max(), 100000,
                               std::numeric_limits<float>::max());
    if (filter != nullptr && filter->coefficients != nullptr) {
        std::cout << "Created filter with all max values (very large kernel length)" << std::endl;
        std::cout << "Filter coefficients (first 20): ";
        print_float_array(filter->coefficients, 20);
        destroy_fir_filter(filter);
    } else {
        std::cout << "Failed to create filter with all max values" << std::endl;
    }
}


// ================================
// = UNIT TESTS: apply_fir_filter =
// ================================

// Basic functionality test
TEST(FIRFilterApplyTest, BasicFunctionality) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, HANNING, 1000.0f, 11, 8000.0f);
    ASSERT_NE(filter, nullptr);

    float input_signal[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    int signal_length = sizeof(input_signal) / sizeof(input_signal[0]);
    auto *output_signal = (float *)malloc(signal_length * sizeof(float));

    apply_fir_filter(filter, input_signal, output_signal, signal_length);

    // Ensure input signal is not destroyed
    for (int i = 0; i < signal_length; ++i) {
        ASSERT_EQ(input_signal[i], static_cast<float>(i + 1));
    }

    // Print out the input and output signals for visual check
    std::cout
            << "Filter data: A Hanning low-pass filter with cutoff_freq = 1000 Hz; kernel_length = 11; sample_rate = 8000 Hz"
            << std::endl;
    std::cout << "Input signal: ";
    print_float_array(input_signal, signal_length);
    std::cout << "Output signal: ";
    print_float_array(output_signal, signal_length);

    free(output_signal);
    destroy_fir_filter(filter);
}

// Output signal correctness test
TEST(FIRFilterApplyTest, IsOutputSignalCalculationCorrect) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, HANNING, 1000.0f, 11, 8000.0f);
    ASSERT_NE(filter, nullptr);

    std::vector<std::vector<float>> input_signals = {
            {1.0, 2.0, 3.0, 4.0, 5.0},
            {1.0, -1.0},
            {0.5, 1.5, 2.5, 3.5, 4.5, 10, 30, 50, 100}
    };

    // Expected values are calculated using Python's firwin and lfilter from scipy.signal
    std::vector<std::vector<float>> expected_outputs = {
            {0.00000000, 0.00000000, 0.02592097, 0.15601020, 0.48968537},
            {0.00000000, 0.00000000},
            {0.00000000, 0.00000000, 0.01296048, 0.09096559, 0.32284779, 0.78152296, 1.46699110, 2.42298071, 4.28862617}
    };

    for (int i = 0; i < input_signals.size(); ++i) {
        float *input_signal = input_signals[i].data();
        int signal_length = (int) input_signals[i].size();
        auto *output_signal = (float *)malloc(signal_length * sizeof(float));

        apply_fir_filter(filter, input_signal, output_signal, signal_length);

        // Compare output signal with expected results
        compare_arrays(output_signal, expected_outputs[i].data(), signal_length);
        free(output_signal);
    }

    destroy_fir_filter(filter);
}

// Large input test
TEST(FIRFilterApplyTest, LargeInputTest) {
    std::cout << "Kernel length: 1001, input signal length: 100000" << std::endl;
    FIRFilter *filter_lowpass = create_fir_filter(LOW_PASS, BLACKMAN, 1000.0f, 1001, 8000.0f);
    FIRFilter *filter_highpass = create_fir_filter(HIGH_PASS, KAISER_B8, 1000.0f, 1001, 8000.0f);
    ASSERT_NE(filter_lowpass, nullptr);
    ASSERT_NE(filter_highpass, nullptr);

    const int large_signal_length = 100000;
    float input_signal[large_signal_length];
    float output_signal[large_signal_length];
    for (int i = 0; i < large_signal_length; ++i) {
        input_signal[i] = (float) (i % 10) + 1; // Example signal
        output_signal[i] = 0.0f;
    }

    apply_fir_filter(filter_lowpass, input_signal, output_signal, large_signal_length);
    std::cout << "First 20 values of the large signal output (low_pass): " << std::endl;
    print_float_array(output_signal, 20);

    apply_fir_filter(filter_highpass, input_signal, output_signal, large_signal_length);
    std::cout << "First 20 values of the large signal output (high_pass): " << std::endl;
    print_float_array(output_signal, 20);

    destroy_fir_filter(filter_lowpass);
    destroy_fir_filter(filter_highpass);
}

// Null tests
TEST(FIRFilterApplyTest, NullTests) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, HANNING, 1000.0f, 11, 8000.0f);
    ASSERT_NE(filter, nullptr);

    float input_signal[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    int signal_length = sizeof(input_signal) / sizeof(input_signal[0]);
    auto *output_signal = (float *)malloc(signal_length * sizeof(float));
    for (int i = 0; i < signal_length; ++i) { output_signal[i] = 0; }

    apply_fir_filter(nullptr, input_signal, output_signal, signal_length);
    // Expect no change in output signal
    for (int i = 0; i < signal_length; ++i) {
        ASSERT_EQ(output_signal[i], 0.0);
    }

    apply_fir_filter(filter, nullptr, output_signal, signal_length);
    // Expect no change in output signal
    for (int i = 0; i < signal_length; ++i) {
        ASSERT_EQ(output_signal[i], 0.0);
    }

    apply_fir_filter(filter, input_signal, nullptr, signal_length);
    // Expect no crash

    free(filter->coefficients);
    filter->coefficients = nullptr;
    apply_fir_filter(filter, input_signal, output_signal, signal_length);
    // Expect no crash

    free(output_signal);
    destroy_fir_filter(filter);
}

// Edge tests
TEST(FIRFilterApplyTest, EdgeTests) {
    FIRFilter* filter = create_fir_filter(LOW_PASS, BLACKMAN, std::numeric_limits<float>::max(), 11, std::numeric_limits<float>::max());
    float input_signal[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    int signal_length = sizeof(input_signal) / sizeof(input_signal[0]);
    auto *output_signal = (float *)malloc(signal_length * sizeof(float));
    for (int i = 0; i < signal_length; ++i) { output_signal[i] = 0; }

    apply_fir_filter(filter, input_signal, output_signal, signal_length);

    std::cout << "Created a filter with maximum possible cutoff frequency and sample rate." << std::endl;
    std::cout << "Input signal:" << std::endl;
    print_float_array(input_signal, signal_length);
    std::cout << "Output signal:" << std::endl;
    print_float_array(output_signal, signal_length);

    free(output_signal);
    destroy_fir_filter(filter);
}


// ==================================
// = UNIT TESTS: destroy_fir_filter =
// ==================================

// Test destroying a filter with valid memory allocation
TEST(FIRFilterDestroyTest, DestroyValidFilter) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, HANNING, 1000.0f, 11, 8000.0f);
    ASSERT_NE(filter, nullptr);
    ASSERT_NE(filter->coefficients, nullptr);
    destroy_fir_filter(filter);
    // Expect no crashes and memory is freed (Valgrind check)
}

// Test destroying a null filter pointer
TEST(FIRFilterDestroyTest, DestroyNullFilter) {
    FIRFilter *filter = nullptr;
    destroy_fir_filter(filter); // Should handle null pointer gracefully
    // No assertion needed, just ensuring no crashes
}

// Test destroying a filter with null coefficients
TEST(FIRFilterDestroyTest, DestroyFilterWithNullCoefficients) {
    FIRFilter *filter = create_fir_filter(LOW_PASS, HANNING, 1000.0f, 11, 8000.0f);
    ASSERT_NE(filter, nullptr);
    ASSERT_NE(filter->coefficients, nullptr);

    // Manually set coefficients to null to simulate this edge case
    free(filter->coefficients);
    filter->coefficients = nullptr;

    destroy_fir_filter(filter);
    // Just ensuring no crashes and memory is freed
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
