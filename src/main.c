#include <stdlib.h>
#include <string.h>
#include "fir_filter_cli.h"

// Command Line Interface (CLI) entry point.
// Please refer to the README file for the usage syntax
// or just run the main.c without any parameters

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "create") == 0) {
        handle_create_fir_filter(argc, argv);
    } else if (strcmp(argv[1], "apply") == 0) {
        handle_apply_fir_filter(argc, argv);
    } else if (strcmp(argv[1], "destroy") == 0) {
        handle_destroy_fir_filter(argc, argv);
    } else {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}