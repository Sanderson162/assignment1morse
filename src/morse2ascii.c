#include <dc_application/command_line.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_unistd.h>
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

static bool readNextBitPair(const struct dc_posix_env *env, struct dc_error *err, int *currentPos, uint8_t *byteBuffer, uint8_t *currentBitPair) {
    switch (*currentPos) {
        case 0:
            if (dc_read(env, err, STDIN_FILENO, byteBuffer, 1) == 0) {
                return false;
            }
            *currentPos = 1;
            *currentBitPair = *byteBuffer & 0b11000000;
            *currentBitPair >>= 6;
            break;
        case 1:
            *currentPos = 2;
            *currentBitPair = *byteBuffer & 0b00110000;
            *currentBitPair >>= 4;
            break;
        case 2:
            *currentPos = 3;
            *currentBitPair = *byteBuffer & 0b00001100;
            *currentBitPair >>= 2;
            break;
        case 3:
            *currentPos = 0;
            *currentBitPair = *byteBuffer & 0b00000011;
            break;

    }
    return true;
}

static int run(const struct dc_posix_env *env, struct dc_error *err)
{
    DC_TRACE(env);

    int currentPos = 0; // 0 1 2 3
    uint8_t currentBitPair;
    uint8_t byteBuffer;

    while (readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair)) {

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
