#include <dc_application/command_line.h>
#include <dc_application/config.h>
#include <dc_application/defaults.h>
#include <dc_application/environment.h>
#include <dc_application/options.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int run(const struct dc_posix_env *env, struct dc_error *err);
static void error_reporter(const struct dc_error *err);
static void trace_reporter(const struct dc_posix_env *env,
                           const char *file_name,
                           const char *function_name,
                           size_t line_number);


int main(int argc, char *argv[])
{
    dc_posix_tracer tracer;
    dc_error_reporter reporter;
    struct dc_posix_env env;
    struct dc_error err;
    struct dc_application_info *info;
    int ret_val;

    reporter = error_reporter;
    tracer = trace_reporter;
    tracer = NULL;
    dc_error_init(&err, reporter);
    dc_posix_env_init(&env, tracer);
    info = dc_application_info_create(&env, &err, "Settings Application");
    ret_val = run(&env, &err);
    dc_application_info_destroy(&env, &info);
    dc_error_reset(&err);

    return ret_val;
}


void pushValue(const struct dc_posix_env *env, struct dc_error *err, const uint8_t value, uint8_t *currentPos, uint8_t *writeOutChar) {
    switch (*currentPos) {
        case 0:
            *writeOutChar |= value * 64;
            *currentPos = 1;
            break;
        case 1:
            *writeOutChar |= value * 16;
            *currentPos = 2;
            break;
        case 2:
            *writeOutChar |= value * 4;
            *currentPos = 3;
            break;
        case 3:
            *writeOutChar |= value;
            *currentPos = 0;
            dc_write(env, err, STDOUT_FILENO, writeOutChar, 1);
            *writeOutChar = 0;
            break;
    }
}

static int run(const struct dc_posix_env *env, struct dc_error *err)
{
    DC_TRACE(env);

    char readInChar;
    uint8_t writeOutChar = 0;
    uint8_t currentChar = 0; // 0 1 2 3
    const uint8_t dit = 0b10;
    const uint8_t dah = 0b01;
    const uint8_t space = 0b11;
    const uint8_t eoc = 0b00;

    ssize_t count;
    while ((count = dc_read(env, err, STDIN_FILENO, &readInChar, 1) > 0)) {
        if (readInChar <= 'Z' && readInChar >= 'A'){
            readInChar += 'a' - 'A';
        }
        if (readInChar == ' ') {
            pushValue(env, err, space, &currentChar, &writeOutChar);
        } else {
            switch (readInChar) {
                case 'a': // .-
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'b': // -...
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'c': // -.-.
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'd': // -..
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'e': // .
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'f': // ..-.
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'g': // --.
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'h': // ....
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'i': // ..
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'j': // .---
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'k': // -.-
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'l': // .-..
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'm': // --
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'n': // -.
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'o': // ---
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'p': // .--.
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 'q': // --.-
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'r': // .-.
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 's': // ...
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case 't': // -
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'u': // ..-
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'v': // ...-
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'w': // .--
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'x': // -..-
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'y': // -.--
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case 'z': // --..
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case '&': // .-...
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case '\'': // .----.
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case '@': // .--.-.
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case '(': // -.--.-
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case ')': // -.--.
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case ':': // ---...
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    break;
                case ',': // --..--
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case '=': // -...-
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case '!': // -.-.--
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case '.': // .-.-.-
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case '-': // -....-
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);

                    break;
                case '%': // ------..-.-----
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    break;
                case '+': // .-.-.
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
                case '"': // .-..-.
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
                case '?': // ..--..
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
                case '/': // -..-.
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
                case '\n': // .-.-
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);

                    break;
                case '0': // -----
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);

                    break;
                case '1': // .----
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);

                    break;
                case '2': // ..---
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);

                    break;
                case '3': // ...--
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);

                    break;
                case '4': // ....-
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);

                    break;
                case '5': // .....
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
                case '6': // -....
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
                case '7': // --...
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
                case '8': // ---..
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
                case '9': // ----.
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dah, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
                default: // error -> .......
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);
                    pushValue(env, err, dit, &currentChar, &writeOutChar);

                    break;
            }
            pushValue(env, err, eoc, &currentChar, &writeOutChar);
        }
    }
    pushValue(env, err, eoc, &currentChar, &writeOutChar);
    pushValue(env, err, eoc, &currentChar, &writeOutChar);
    switch (currentChar) {
        case 1:
            pushValue(env, err, eoc, &currentChar, &writeOutChar);
            pushValue(env, err, eoc, &currentChar, &writeOutChar);
            pushValue(env, err, eoc, &currentChar, &writeOutChar);
            break;
        case 2:
            pushValue(env, err, eoc, &currentChar, &writeOutChar);
            pushValue(env, err, eoc, &currentChar, &writeOutChar);
            break;
        case 3:
            pushValue(env, err, eoc, &currentChar, &writeOutChar);
            break;
        default:
            break;
    }
    return EXIT_SUCCESS;
}




static void error_reporter(const struct dc_error *err)
{
    fprintf(stderr, "ERROR: %s : %s : @ %zu : %d\n", err->file_name, err->function_name, err->line_number, 0);
    fprintf(stderr, "ERROR: %s\n", err->message);
}

static void trace_reporter(__attribute__((unused)) const struct dc_posix_env *env,
                           const char *file_name,
                           const char *function_name,
                           size_t line_number)
{
    fprintf(stdout, "TRACE: %s : %s : @ %zu\n", file_name, function_name, line_number);
}
