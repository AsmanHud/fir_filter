# FIR Filter Project

## Overview
This project implements a Finite Impulse Response (FIR) filter in C. It includes a command-line interface (CLI) for creating, applying, and destroying FIR filters. Apart from the given CLI, the FIR filter library can be used on its own in custom code, to create, apply, and destroy filters. The implementation is accompanied by a comprehensive set of unit tests.

## Features
- Create low-pass and high-pass FIR filters with various window functions.
- Apply FIR filters to input signals.
- Destroy FIR filters, freeing associated resources.
- Comprehensive unit tests using Google Test.

## Getting Started

### Prerequisites
- CMake 3.22.1 or higher
- A C compiler (GCC, Clang, etc.)
- A C++ compiler for running tests
- Google Test for unit tests

### Building the Project
1. Clone the repository:
    ```sh
    git clone <repository-url>
    cd FIR_Filter_Project
    ```

2. Create a build directory and run CMake:
    ```sh
    mkdir build
    cd build
    cmake ..
    ```

3. Build the project:
    ```sh
    make
    ```

### Running the CLI
The CLI provides three main commands: `create`, `apply`, and `destroy`.

#### Creating a Filter
```sh
./fir_filter create <filter_type> <window_type> <cutoff_freq> <kernel_length> <sample_rate> <output_file>
```
- `<filter_type>`: `lowpass` or `highpass`
- `<window_type>`: `rect`, `hanning`, `hamming`, `blackman`, `kaiser_b6`, `kaiser_b8`, `kaiser_b10`
- `<cutoff_freq>`: Cutoff frequency in Hz
- `<kernel_length>`: Kernel length (odd integer)
- `<sample_rate>`: Sample rate in Hz
- `<output_file>`: Path to save the filter (binary file)

#### Applying a Filter
```sh
./fir_filter apply <input_file> <filter_file> <output_file>
```
- `<input_file>`: Path to input signal file (text file with one float per line)
- `<filter_file>`: Path to filter file (binary file)
- `<output_file>`: Path to output signal file (text file)

#### Destroying a Filter
```sh
./fir_filter destroy <filter_file>
```
- `<filter_file>`: Path to filter file (binary file)

### Running Unit Tests
To run the unit tests, execute:
```sh
./runTests
```

## Code Structure
- `src/fir_filter.c` / `src/fir_filter.h`: FIR filter implementation.
- `src/fir_filter_cli.c` / `src/fir_filter_cli.h`: CLI implementation.
- `src/main.c`: Entry point for the CLI.
- `tests/fir_filter_tests.cpp`: Unit tests for the FIR filter.
- `CMakeLists.txt`: CMake build configuration.

## Additional information

### Window Type to Stop band attenuation [dB]
I have chosen the current available window types so that they cover a decent range of stop band attenuation specifications.
```
- Rectangular = 21
- Hanning     = 44
- Hamming     = 55
- Blackman    = 75
- Kaiser_b6   = 64
- Kaiser_b8   = 81
- Kaiser_b10  = 100
```

### Improvement ideas
- The biggest improvement yet to make is to change the convolution implementation from flip-and-shift (O(N\*M)) to FFT algorithm (O(N\*log(M))). It is possible to leave the current implementation as is and add a new function apply_fir_filter_fft().
- The Kaiser window input could be more generic, allowing for custom input of beta parameter. Current implementation has three most logical values for the beta (see window type to stop band attenuation above).
- Custom window function could be allowed as an input when creating a filter.
- The Bessel function approximation constant could be implemented as an input parameter.

### Considerations
- The number of terms for calculation of the Bessel function for the Kaiser window is theoretically infinite, but in the implementation set to 25 (aka the Bessel function approximation). Usually it is taken from 20 to 25 terms.
- The test file could be modularized further into separate test files for separate units (create, apply, destroy), but I feel like it is okay as it is.
- The CLI has not been tested enough yet.

## License
This project is licensed under the MIT License.

## Acknowledgements
- Google Test for the unit testing framework.
- My DSP Professor, for his slides on FIR filter design.