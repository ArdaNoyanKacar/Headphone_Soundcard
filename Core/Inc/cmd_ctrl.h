#ifndef CMD_CTRL_H
#define CMD_CTRL_H

#include <stdint.h>

#define CMD_MAX_ARGS 8
#define CMD_NAME_MAX_LEN 32
#define CMD_MAX_LEN 128
#define CMD_PARSE_SUCCESS 0
#define CMD_PARSE_FAIL    1

#define CMD_VALID 1
#define CMD_INVALID 0



void ctrl_init(void);
uint8_t ctrl_parse_cmd(char* line, char* cmd_name, uint8_t cmd_name_len, char* args[], uint16_t* arg_count);
uint8_t ctrl_execute_cmd(char* cmd_name, char* args[], int arg_count);
void ctrl_poll(void);



#endif // CMD_CTRL_H


