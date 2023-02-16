#include <stdio.h>
#include <string.h>

#define ARG_IN_TEMPLATE "--in-template"

struct Arguments {
    char *fileName;
};

int fillArguments(int argc, char *argv[], struct Arguments *args) {
    args->fileName = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ARG_IN_TEMPLATE) == 0) {
            if (++i == argc) {
                fprintf(stderr, "File name expected after " ARG_IN_TEMPLATE "\n");
                return 1;
            }

            args->fileName = argv[i];
        }
        else {
            fprintf(stderr, "Invalid argument %s\n", argv[i]);
            return 1;
        }
    }

    if (args->fileName == NULL) {
        fprintf(stderr, "Missing argument " ARG_IN_TEMPLATE "\n");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    printf("struct-doc instancer\n");

    struct Arguments args;
    const int result = fillArguments(argc, argv, &args);
    if (result != 0) {
        return result;
    }

    printf("All fine so far\n");
    return 0;
}