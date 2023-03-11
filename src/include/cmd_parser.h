#ifndef SRC_INCLUDE_CMD_PARSER_H_
#define SRC_INCLUDE_CMD_PARSER_H_

enum {
    CMD_SET_MEASUR_PERIOD   = 0xF1, /* set measurement period in sec. */
    CMD_CLEAR_WHIYELIST     = 0xF2, /* reset whitelist                */
    CMD_RESET               = 0xF3, /* reset module                   */
    CMD_MAIN_NOTIFY         = 0xF4, /* start notify                   */
    CMD_SET_BINDKEY         = 0xF5, /* set secure bind key            */
    CMD_RESET_BINDKEY       = 0xF6, /* reset bind key                 */
    CMD_SET_ADDRESS         = 0xF7, /* set device address             */
    CMD_CLEAR_CFG           = 0XF8, /* set dedfault config            */
    CMD_SET_DEVICE_TYPE     = 0xF9, /* set device type                */
    CMD_SET_DIVISOR         = 0xFA  /* set divisor for HA             */
} cmd_key;

void cmd_parser(void * p);

#endif /* SRC_INCLUDE_CMD_PARSER_H_ */
