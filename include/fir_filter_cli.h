#ifndef FIR_FILTER_CLI_H
#define FIR_FILTER_CLI_H


/**
 * @brief Prints the usage information for the CLI.
 *
 * This function prints the usage instructions for the command-line interface,
 * detailing the available commands and their required arguments.
 *
 * @param prog_name The name of the executable program.
 */
void print_usage(const char *prog_name);

/**
 * @brief Handles the creation of a FIR filter.
 *
 * This function parses the command-line arguments to create a FIR filter
 * with the specified parameters. It then saves the filter to a binary file.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 */
void handle_create_fir_filter(int argc, char *argv[]);

/**
 * @brief Handles the application of a FIR filter to an input signal.
 *
 * This function reads an input signal from a file, applies a previously
 * created FIR filter loaded from a binary file, and writes the filtered
 * output signal to another file.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 */
void handle_apply_fir_filter(int argc, char *argv[]);

/**
 * @brief Handles the destruction of a FIR filter.
 *
 * This function deletes the specified binary file containing the FIR filter.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 */
void handle_destroy_fir_filter(int argc, char *argv[]);


#endif //FIR_FILTER_CLI_H
