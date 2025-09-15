#include "cmd_ctrl.h"
#include "sgtl5000.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


extern UART_HandleTypeDef huart2;

// RX ring buffer
#define RX_BUFFER_SIZE 128
#define FW_VERSION "Soundcard v1.0"

static volatile char cmd_buf[RX_BUFFER_SIZE];
static volatile uint16_t cmd_len = 0;
static volatile bool cmd_ready = false;
static volatile char rx_char;

// Echo input characters
static void ctrl_putc(char c) {
    (void)HAL_UART_Transmit(&huart2, (uint8_t *)&c, 1, 10);
}

void ctrl_init()
{
    cmd_len = 0;
    cmd_ready = false;
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_char, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart2) {
        char c = rx_char;
        ctrl_putc(c); // Echo back
        if (c == '\b' || c == 0x7F) {// Backspace/DEL
            if (cmd_len > 0) {
                cmd_len--;
                // erase on terminal
                ctrl_putc('\b'); ctrl_putc(' '); ctrl_putc('\b');
            }
        }
        else if (c == '\r' || c == '\n') {
            if (cmd_len > 0 && !cmd_ready) {
                // Terminate the command string
                if (cmd_len >= RX_BUFFER_SIZE) {
                    cmd_len = RX_BUFFER_SIZE - 1;
                }
                cmd_buf[cmd_len] = '\0';
                cmd_ready = true;
            }

            cmd_len = 0; // Reset for next command
        }
        else {
            if (cmd_len < RX_BUFFER_SIZE - 1) {
                cmd_buf[cmd_len++] = c;
            }
            else {
                // Buffer overflow, reset
                cmd_len = 0;
            }
        }

        // Re-arm for the next character
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&rx_char, 1);
    }
}


/**
 * @brief Parse a command line into its components.
 * @param line The command line to parse.
 * @param cmd_name Pointer to store the command name.
 * @param args Array to store the command arguments.
 * @param arg_count Pointer to store the number of arguments.
 * @return CMD_PARSE_SUCCESS on success, CMD_PARSE_FAIL on failure.
 */
uint8_t ctrl_parse_cmd(char* line, char* cmd_name, uint8_t cmd_name_len, char* args[], uint16_t* arg_count)
{
    if (!line || !cmd_name || !args || !arg_count || cmd_name_len == 0) {
        return CMD_PARSE_FAIL;  
    }

    *arg_count = 0;

    const char* delims = " \t\r\n";   // include CR/LF
    
    // first token is command name
    char* token = strtok(line, delims);
    if (!token) {
        return CMD_PARSE_FAIL; // No command found
    }
    strncpy(cmd_name, token, cmd_name_len - 1);
    cmd_name[cmd_name_len - 1] = '\0';

    // subsequent tokens are arguments
    while ((token = strtok(NULL, " \t")) != NULL) {
        if (*arg_count  >= CMD_MAX_ARGS) {
            return CMD_PARSE_FAIL; // Too many arguments
        }
        args[*arg_count] = token;
        (*arg_count)++;
    }

    return CMD_PARSE_SUCCESS;
}

static void str_to_lower(char* s)
{
    if (!s) return;
    while (*s) {
        if (*s >= 'A' && *s <= 'Z') *s = (char)(*s + ('a' - 'A'));
        s++;
    }
}

/**
 * @brief Execute a parsed command.
 * @param cmd_name The name of the command to execute.
 * @param args The arguments for the command.
 * @param arg_count The number of arguments.
 * @return CMD_EXEC_SUCCESS on success, CMD_EXEC_FAIL on failure.
 */
uint8_t ctrl_execute_cmd(char* cmd_name, char* args[], int arg_count)
{
    if (!cmd_name) {
        return CMD_PARSE_FAIL;
    }

    // Convert command name to lowercase for case-insensitive comparison
    str_to_lower(cmd_name);

    if (strcmp(cmd_name, "help") == 0 && arg_count == 0) {
        printf("\r\nCommands:\r\n");
        printf("  help\r\n");
        printf("  setEQ b0 b1 b2 b3 b4            (-12..+12 dB; ramped)\r\n");
        printf("  setEQProfile NAME               (ROCK, POP, CLASSICAL, RAP, JAZZ, EDM, VOCAL, BRIGHT, WARM, BASSBOOST, TREBLEBOOST, MAXSMILE, MIDSPIKE, FLAT)\r\n");
        printf("  setBassEnhance on|off [lr bass] (0|1 [0..63 0..127]; ramped amount)\r\n");
        printf("  setSurround on|off [width]      (0|1 [0..7])\r\n");
        printf("  setVolume code                  (raw DAC code 0..255 or 0xNN)\r\n");
        printf("  dump\r\n\r\n");
        return CMD_VALID;
    }
    else if (strcmp(cmd_name, "version") == 0 && arg_count == 0) {
        printf("\r\n%s\r\n\r\n", FW_VERSION);
        return CMD_VALID;
    }
    else if (strcmp(cmd_name, "seteq") == 0 && (arg_count == 5)) {
        int b0 = atoi(args[0]);
        int b1 = atoi(args[1]);
        int b2 = atoi(args[2]);
        int b3 = atoi(args[3]);
        int b4 = atoi(args[4]);

        sgtl5000_dap_geq_set_bands_db((int8_t)b0, (int8_t)b1, (int8_t)b2, (int8_t)b3, (int8_t)b4);

        return CMD_VALID;
    }
    else if (strcmp(cmd_name, "seteqprofile") == 0 && (arg_count == 1)) {
        str_to_lower(args[0]);
        if (strcmp(args[0], "flat") == 0) {
            sgtl5000_dap_geq_set_bands_db( 0,  0,   0,  0,  0);
        }
        else if (strcmp(args[0], "rock") == 0) {
            sgtl5000_dap_geq_set_bands_db( 4,  2,   0,  3,  5);
        }
        else if (strcmp(args[0], "pop") == 0) {
            sgtl5000_dap_geq_set_bands_db( 3,  1,   0,  2,  4);
        }
        else if (strcmp(args[0], "classical") == 0) {
            sgtl5000_dap_geq_set_bands_db(-1,  2,   3,  2, -1);
        }
        else if (strcmp(args[0], "rap") == 0) {
            sgtl5000_dap_geq_set_bands_db( 6,  3,   0,  1,  2);
        }
        else if (strcmp(args[0], "jazz") == 0) {
            sgtl5000_dap_geq_set_bands_db( 2,  2,   1,  2,  2);
        }
        else if (strcmp(args[0], "edm") == 0) {
            sgtl5000_dap_geq_set_bands_db( 6,  2,   0,  2,  6);
        }
        else if (strcmp(args[0], "vocal") == 0) {
            sgtl5000_dap_geq_set_bands_db(-2,  3,   4,  3, -2);
        }
        else if (strcmp(args[0], "bright") == 0) {
            sgtl5000_dap_geq_set_bands_db(-3, -1,   0,  3,  6);
        }
        else if (strcmp(args[0], "warm") == 0) {
            sgtl5000_dap_geq_set_bands_db( 6,  2,   0, -2, -3);
        }
        else if (strcmp(args[0], "bassboost") == 0) {
            sgtl5000_dap_geq_set_bands_db( 9,  3,   0,  0,  0);
        }
        else if (strcmp(args[0], "trebleboost") == 0) {
            sgtl5000_dap_geq_set_bands_db( 0,  0,   0,  6,  9);
        }
        else if (strcmp(args[0], "maxsmile") == 0) {
            sgtl5000_dap_geq_set_bands_db(12,  8, -12,  8, 12);
        }
        else if (strcmp(args[0], "midspike") == 0) {
            sgtl5000_dap_geq_set_bands_db(-12, 12, 12, 12, -12);
        }
        else {
            printf("ERR invalid: unknown EQ profile\r\n");
            return CMD_INVALID;
        }
        return CMD_VALID;
    }
    else if (strcmp(cmd_name, "setbassenhance") == 0 && (arg_count == 1 || arg_count == 3)) {
        bool enable = false;
        if (strcmp(args[0], "on") == 0) {
            enable = true;
        }
        else if (strcmp(args[0], "off") == 0) {
            enable = false;
        }
        else {
            printf("ERR invalid: first argument must be 'on' or 'off'\r\n");
            return CMD_INVALID;
        }
        uint8_t lr_level = 0x5; // default
        uint8_t bass_level = 0x1f; // default
        if (arg_count == 3) {
            lr_level = (uint8_t)atoi(args[1]);
            bass_level = (uint8_t)atoi(args[2]);
        }
        sgtl5000_dap_bass_enhance_set(enable, lr_level, bass_level);
        return CMD_VALID;
    }
    else if (strcmp(cmd_name, "setsurround") == 0 && (arg_count == 1 || arg_count == 2)) {
        bool enable = false;
        if (strcmp(args[0], "on") == 0) {
            enable = true;
        }
        else if (strcmp(args[0], "off") == 0) {
            enable = false;
        }
        else {
            printf("ERR invalid: first argument must be 'on' or 'off'\r\n");
            return CMD_INVALID;
        }
        uint8_t width = 4; // default
        if (arg_count == 2) {
            width = (uint8_t)atoi(args[1]);
        }
        sgtl5000_dap_surround_set(enable ? SGTL_SURROUND_STEREO : SGTL_SURROUND_OFF, width);
        return CMD_VALID;
    }
    else if (strcmp(cmd_name , "setvolume") == 0 && (arg_count == 1)) {
        uint8_t vol_percent = (uint8_t)atoi(args[0]);
        sgtl5000_change_dac_volume(vol_percent);
        return CMD_VALID;
    }
    else if (strcmp(cmd_name, "dumpregs") == 0 && arg_count == 0) {
        sgtl5000_print_all_regs();
        return CMD_VALID;
    }
    else {
        printf("Command not recognized!\r\n");
        return CMD_INVALID;
    }
}

/**
 * @brief Poll the USART for incoming commands and process them.
 */
void ctrl_poll(void)
{
    if (cmd_ready) {
        char raw[RX_BUFFER_SIZE];
        char line[RX_BUFFER_SIZE];
        memset(raw, 0, sizeof(raw));
        memset(line, 0, sizeof(line));

        // Copy the raw line for the echo
        strncpy(raw, (const char*)cmd_buf, RX_BUFFER_SIZE);
        raw[RX_BUFFER_SIZE - 1] = '\0';

        strncpy(line, (const char*)cmd_buf, RX_BUFFER_SIZE);
        line[RX_BUFFER_SIZE - 1] = '\0';
        cmd_ready = false;

        // Echo the received command
        printf("\r\n> %s\r\n", raw);

        char cmd_name[CMD_NAME_MAX_LEN];
        char* args[CMD_MAX_ARGS];
        uint16_t arg_count = 0;
        memset(cmd_name, 0, sizeof(cmd_name));
        memset(args, 0, sizeof(args));

        if (ctrl_parse_cmd(line, cmd_name, sizeof(cmd_name), args, &arg_count) == CMD_PARSE_SUCCESS) {
            ctrl_execute_cmd(cmd_name, args, arg_count);
        }
    }
}


