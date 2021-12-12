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
    const uint8_t dit = 0b10;
    const uint8_t dah = 0b01;
    const uint8_t space = 0b11;
    const uint8_t eoc = 0b00;

    while (readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair)) {
        dc_write(env, err, STDOUT_FILENO, &currentBitPair, 1);

        // huffman tree if else loop - this is so incredibly disgusting
        if (currentBitPair == dit) { // .
            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
            if (currentBitPair == dit) { // ..
                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                if (currentBitPair == dit) { // ...
                    readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                    if (currentBitPair == dit) { // ....
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // .....
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // ......
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // .....-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // 5
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "5", 1);
                            }
                        } else if (currentBitPair == dah){ // ....-
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // ....-.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // ....--
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // 4
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "4", 1);
                            }
                        } else { // H
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "H", 1);
                        }
                    } else if (currentBitPair == dah){ // ...-
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // ...-.
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            // N/A
                        } else if (currentBitPair == dah){ // ...--
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // ...--.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // ...---
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // 3
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "3", 1);
                            }
                        } else { // V
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "V", 1);
                        }
                    } else { // S
                        // either eoc char or something not specified. assume eoc
                        dc_write(env, err, STDOUT_FILENO, "S", 1);
                    }
                } else if (currentBitPair == dah){ // ..-
                    readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                    if (currentBitPair == dit) { // ..-.
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // ..-..
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            // N/A
                        } else if (currentBitPair == dah){ // ..-.-
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            // N/A
                        } else { // F
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "F", 1);
                        }
                    } else if (currentBitPair == dah){ // ..--
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // ..--.
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // ..--..
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, "?", 1);
                                }
                            } else if (currentBitPair == dah){ // ..--.-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // N/A
                                // either eoc char or something not specified. assume eoc
                                //dc_write(env, err, STDOUT_FILENO, '', 1);
                            }
                        } else if (currentBitPair == dah){ // ..---
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // ..---.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // ..----
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // 2
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "2", 1);
                            }
                        } else { // N/A
                            // either eoc char or something not specified. assume eoc
                            //dc_write(env, err, STDOUT_FILENO, '', 1);
                        }
                    } else { // U
                        // either eoc char or something not specified. assume eoc
                        dc_write(env, err, STDOUT_FILENO, "U", 1);
                    }
                } else { // I
                    // either eoc char or something not specified. assume eoc
                    dc_write(env, err, STDOUT_FILENO, "I", 1);
                }
            } else if (currentBitPair == dah){ // .-
                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                if (currentBitPair == dit) { // .-.
                    readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                    if (currentBitPair == dit) { // .-..
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // .-...
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // .-....
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // .-...-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // &
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "&", 1);
                            }
                        } else if (currentBitPair == dah){ // .-..-
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // .-..-.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, '"', 1);
                                }
                            } else if (currentBitPair == dah){ // .-..--
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else { // N/A
                                // either eoc char or something not specified. assume eoc
                                //dc_write(env, err, STDOUT_FILENO, '', 1);
                            }
                        } else { // L
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "L", 1);
                        }
                    } else if (currentBitPair == dah){ // .-.-
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // .-.-.
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // .-.-..
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else if (currentBitPair == dah){ // .-.-.-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, ".", 1);
                                }
                            } else { // +
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "+", 1);
                            }
                        } else if (currentBitPair == dah){ // .-.--
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                        } else { // \n
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "\n", 1);
                        }
                    } else { // R
                        // either eoc char or something not specified. assume eoc
                        dc_write(env, err, STDOUT_FILENO, "R", 1);
                    }
                } else if (currentBitPair == dah){ // .--
                    readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                    if (currentBitPair == dit) { // .--.
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // .--..
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // .--...
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // .--..-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { //
                                // either eoc char or something not specified. assume eoc
                                //dc_write(env, err, STDOUT_FILENO, '', 1);
                            }
                        } else if (currentBitPair == dah){ // .--.-
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // .--.-.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, "@", 1);
                                }
                            } else if (currentBitPair == dah){ // .--.--
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else { // N/A
                                // either eoc char or something not specified. assume eoc
                                //dc_write(env, err, STDOUT_FILENO, '', 1);
                            }
                        } else { // L
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "P", 1);
                        }
                    } else if (currentBitPair == dah){ // .---
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // .---.
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // .---..
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else if (currentBitPair == dah){ // .---.-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            } else { // N/A
                                // either eoc char or something not specified. assume eoc
                                //dc_write(env, err, STDOUT_FILENO, "", 1);
                            }
                        } else if (currentBitPair == dah){ // .----
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // .----.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, "`", 1);
                                }
                            } else if (currentBitPair == dah){ // .-----
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else { // 1
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "1", 1);
                            }
                        } else { // J
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "J", 1);
                        }
                    } else { // W
                        // either eoc char or something not specified. assume eoc
                        dc_write(env, err, STDOUT_FILENO, "W", 1);
                    }
                } else { // A
                    // either eoc char or something not specified. assume eoc
                    dc_write(env, err, STDOUT_FILENO, "A", 1);
                }
            } else { // E
                // either eoc char or something not specified. assume eoc
                dc_write(env, err, STDOUT_FILENO, "E", 1);
            }
        } else if (currentBitPair == dah){ // -
            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
            if (currentBitPair == dit) { // -.
                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                if (currentBitPair == dit) { // -..
                    readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                    if (currentBitPair == dit) { // -...
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // -....
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // -.....
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // -....-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, "-", 1);
                                }
                            } else { // 6
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "6", 1);
                            }
                        } else if (currentBitPair == dah){ // -...-
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // -...-.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // -...--
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // =
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "=", 1);
                            }
                        } else { // B
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "B", 1);
                        }
                    } else if (currentBitPair == dah){ // -..-
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // -..-.
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // -..-..
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // -..-.-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // /
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "/", 1);
                            }
                            // N/A
                        } else if (currentBitPair == dah){ // -..--
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            // N/A
                        } else { // V
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "X", 1);
                        }
                    } else { // S
                        // either eoc char or something not specified. assume eoc
                        dc_write(env, err, STDOUT_FILENO, "D", 1);
                    }
                } else if (currentBitPair == dah){ // -.-
                    readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                    if (currentBitPair == dit) { // -.-.
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // -.-..
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            // N/A
                        } else if (currentBitPair == dah){ // -.-.-
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // -.-.-.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // -.-.--
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, "!", 1);
                                }
                                // N/A
                            } else { //
                                // either eoc char or something not specified. assume eoc
                                //dc_write(env, err, STDOUT_FILENO, '', 1);
                            }
                            // N/A
                        } else { // F
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "C", 1);
                        }
                    } else if (currentBitPair == dah){ // -.--
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // -.--.
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // -.--..
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // -.--.-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, ")", 1);
                                }
                            } else { // (
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "(", 1);
                            }
                        } else if (currentBitPair == dah){ // -.---
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // -.---.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // -.----
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { //
                                // either eoc char or something not specified. assume eoc
                                //dc_write(env, err, STDOUT_FILENO, '', 1);
                            }
                        } else { // Y
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "Y", 1);
                        }
                    } else { // K
                        // either eoc char or something not specified. assume eoc
                        dc_write(env, err, STDOUT_FILENO, "K", 1);
                    }
                } else { // N
                    // either eoc char or something not specified. assume eoc
                    dc_write(env, err, STDOUT_FILENO, "N", 1);
                }
            } else if (currentBitPair == dah){ // --
                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                if (currentBitPair == dit) { // --.
                    readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                    if (currentBitPair == dit) { // --..
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // --...
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // --....
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else if (currentBitPair == dah){ // --...-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // 7
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "7", 1);
                            }
                        } else if (currentBitPair == dah){ // --..-
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // --..-.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else if (currentBitPair == dah){ // --..--
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, ",", 1);
                                }
                            } else { // N/A
                                // either eoc char or something not specified. assume eoc
                                //dc_write(env, err, STDOUT_FILENO, '', 1);
                            }
                        } else { // L
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "Z", 1);
                        }
                    } else if (currentBitPair == dah){ // --.-
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // --.-.
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                        } else if (currentBitPair == dah){ // --.--
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                        } else { // Q
                            // either eoc char or something not specified. assume eoc
                            dc_write(env, err, STDOUT_FILENO, "Q", 1);
                        }
                    } else { // G
                        // either eoc char or something not specified. assume eoc
                        dc_write(env, err, STDOUT_FILENO, "G", 1);
                    }
                } else if (currentBitPair == dah){ // ---
                    readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                    if (currentBitPair == dit) { // ---.
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // ---..
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // ---...
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                if (currentBitPair == eoc) {
                                    dc_write(env, err, STDOUT_FILENO, ":", 1);
                                }
                                // N/A
                            } else if (currentBitPair == dah){ // ---..-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                                // N/A
                            } else { // 8
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "8", 1);
                            }
                        } else if (currentBitPair == dah){ // ---.-
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // ---.-.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else if (currentBitPair == dah){ // ---.--
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else { // N/A
                                // either eoc char or something not specified. assume eoc
                                //dc_write(env, err, STDOUT_FILENO, '', 1);
                            }
                        } else { //
                            // either eoc char or something not specified. assume eoc
                            //dc_write(env, err, STDOUT_FILENO, '', 1);
                        }
                    } else if (currentBitPair == dah){ // ----
                        readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                        if (currentBitPair == dit) { // ----.
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // ----..
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else if (currentBitPair == dah){ // ----.-
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            } else { // 9
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "9", 1);
                            }
                        } else if (currentBitPair == dah){ // -----
                            readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            if (currentBitPair == dit) { // -----.
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);
                            } else if (currentBitPair == dah){ // ------
                                readNextBitPair(env, err, &currentPos, &byteBuffer, &currentBitPair);

                            } else { //
                                // either eoc char or something not specified. assume eoc
                                dc_write(env, err, STDOUT_FILENO, "0", 1);
                            }
                        } else { // J
                            // either eoc char or something not specified. assume eoc
                            //dc_write(env, err, STDOUT_FILENO, 'J', 1);
                        }
                    } else { // O
                        // either eoc char or something not specified. assume eoc
                        dc_write(env, err, STDOUT_FILENO, "O", 1);
                    }
                } else { // M
                    // either eoc char or something not specified. assume eoc
                    dc_write(env, err, STDOUT_FILENO, "M", 1);
                }
            } else { // E
                // either eoc char or something not specified. assume eoc
                dc_write(env, err, STDOUT_FILENO, "T", 1);
            }
        } else if (currentBitPair == space) {
            dc_write(env, err, STDOUT_FILENO, " ", 1);
        } else {
            // either eoc char or something not specified. return
            return EXIT_SUCCESS;
        }
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
