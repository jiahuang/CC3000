#include <Arduino.h>
#include <string.h>

#include "tm_net.h"
#include "tm_debug.h"
#include "host_spi.h"


/**
 * HTTP
 */

enum http_header_status
{
    http_header_status_done,
    http_header_status_continue,
    http_header_status_version_character,
    http_header_status_code_character,
    http_header_status_status_character,
    http_header_status_key_character,
    http_header_status_value_character,
    http_header_status_store_keyvalue
};

static unsigned char http_header_state[] = {
/*     *    \t    \n   \r    ' '     ,     :   PAD */
    0x80,    1, 0xC1, 0xC1,    1, 0x80, 0x80, 0xC1, /* state 0: HTTP version */
    0x81,    2, 0xC1, 0xC1,    2,    1,    1, 0xC1, /* state 1: Response code */
    0x82, 0x82,    4,    3, 0x82, 0x82, 0x82, 0xC1, /* state 2: Response reason */
    0xC1, 0xC1,    4, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, /* state 3: HTTP version newline */
    0x84, 0xC1, 0xC0,    5, 0xC1, 0xC1,    6, 0xC1, /* state 4: Start of header field */
    0xC1, 0xC1, 0xC0, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, /* state 5: Last CR before end of header */
    0x87,    6, 0xC1, 0xC1,    6, 0x87, 0x87, 0xC1, /* state 6: leading whitespace before header value */
    0x87, 0x87, 0xC4,   10, 0x87, 0x88, 0x87, 0xC1, /* state 7: header field value */
    0x87, 0x88,    6,    9, 0x88, 0x88, 0x87, 0xC1, /* state 8: Split value field value */
    0xC1, 0xC1,    6, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, /* state 9: CR after split value field */
    0xC1, 0xC1, 0xC4, 0xC1, 0xC1, 0xC1, 0xC1, 0xC1, /* state 10:CR after header value */
};

int tm_http_parse_header_char(int* state, char ch)
{
    int newstate, code = 0;
    switch (ch) {
    case '\t': code = 1; break;
    case '\n': code = 2; break;
    case '\r': code = 3; break;
    case  ' ': code = 4; break;
    case  ',': code = 5; break;
    case  ':': code = 6; break;
    }

    newstate = http_header_state[*state * 8 + code];
    *state = (newstate & 0xF);

    switch (newstate) {
    case 0xC0: return http_header_status_done;
    case 0xC1: return http_header_status_done;
    case 0xC4: return http_header_status_store_keyvalue;
    case 0x80: return http_header_status_version_character;
    case 0x81: return http_header_status_code_character;
    case 0x82: return http_header_status_status_character;
    case 0x84: return http_header_status_key_character;
    case 0x87: return http_header_status_value_character;
    case 0x88: return http_header_status_value_character;
    }

    return http_header_status_continue;
}

int tm_http_parse_headers (int ulSocket, void (*on_header_key)(char *, int), void (*on_header_value)(char *, int), void (*on_body)(int status, int content_len))
{
  int code = 0, content_len = 0;

  int state = 0;
  char buf[32] = { 0 };
  int keyn = 0, valn = 0;
  uint8_t is_content_len_header = 0;

  while (true) {
    // TODO does this block?
    char ch = tm_net_tcp_read_byte(ulSocket);

    switch (tm_http_parse_header_char(&state, ch)) {
      case http_header_status_done:
        on_body(code, content_len);
        return content_len;
        break;

      case http_header_status_code_character:
        code = code * 10 + ch - '0';
        break;

      case http_header_status_key_character:
        if (keyn + 1 < sizeof(buf)) {
          buf[keyn] = tolower(ch);
          keyn++;
        }
        break;

      case http_header_status_value_character:
        if (keyn > 0) {
          buf[keyn] = '\0';
          if (keyn == 14 && 0 == strncmp(buf, "content-length", keyn)) {
            is_content_len_header = 1;
          }
          on_header_key(buf, keyn);
          keyn = 0;
        }

        buf[valn] = ch;
        if (valn + 2 < sizeof(buf)) {
          valn++;
        }
        break;

      case http_header_status_store_keyvalue:
        if (valn > 0) {
          buf[valn] = '\0';
          if (is_content_len_header) {
            for (int i = 0; i < valn; i++) {
              content_len = content_len * 10 + buf[i] - '0';
            }
            is_content_len_header = 0;
          }
          on_header_value(buf, valn);
          valn = 0;
        }
        break;
    }
  }
}






void on_http_header_key (char *buf, int len)
{
  TM_DEBUG("HEADER matched %s\n", buf);
}

void on_http_header_value (char *buf, int len)
{
  TM_DEBUG("VALUE matched %s\n", buf);
}

void on_http_body (int status, int len)
{
  TM_DEBUG("BODY %d length %d\n", status, len);
}



// int crudeReadTCP (int ulSocket)
// {
//   if (tm_net_tcp_block_until_readable(ulSocket) < 0) {
//     return -1;
//   }
//   delay(100);

//   http_parse_header(ulSocket, http_receive_byte, on_http_header, on_http_body);

//   // int totalbytes = 0, bytesReceived = 0;
//   // while (block_readable_socket(ulSocket) == 0) {
//   //   bytesReceived = recv(ulSocket, pucCC3000_Rx_Buffer, CC3000_APP_BUFFER_SIZE, 0);
//   //   if (bytesReceived <= 0) {
//   //     break;
//   //   }
//   //   totalbytes += bytesReceived;
//   //   pucCC3000_Rx_Buffer[bytesReceived] = '\0';
//   //   TM_DEBUG("Result of TCP %d bytes: \"%s\"\n", bytesReceived, pucCC3000_Rx_Buffer);
//   // } 

//   // return bytesReceived;
// }
