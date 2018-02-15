#include <stdio.h>
#include "dev/uart1.h"

#if DISABLE_UART
int
putchar(int c)
{
  return c;
}

#elif TINYOS_SERIAL_FRAMES

/* TODO handle multiple message types */

#ifndef AM_WILAB_CONTIKI_PRINTF
#define AM_WILAB_CONTIKI_PRINTF 66
#endif

#ifndef SERIAL_FRAME_SIZE_CONF
#define SERIAL_FRAME_SIZE 50
#else
#define SERIAL_FRAME_SIZE SERIAL_FRAME_SIZE_CONF
#endif

/* frame format:
 SF [ { Pr Seq Disp ( Dest Src len Grp Type | Payload ) } CRC ] EF


 SF      1   0x7E    Start Frame Byte
 Pr      1   0x45    Protocol Byte (SERIAL_PROTO_PACKET_NOACK)
 Seq     0           (not used) Sequence number byte, not used due to SERIAL_PROTO_PACKET_NOACK
 Disp    1   0x00    Packet format dispatch byte (TOS_SERIAL_ACTIVE_MESSAGE_ID)
 Dest    2   0xFFFF  (not used)
 Src     2   0x0000  (not used)
 len     1   N       Length of Payload (bytes) => in WiLab, this is predefined and depends on Message ID
 Grp     1   0x00    Group
 Type    1           Message ID
 Payload N           The actual serial message => pad with zeroes if message is smaller than predefined length
 CRC     2           Checksum of {Pr -> end of payload}
 EF      1   0x7E    End Frame Byte

*/

/* store all characters so we can calculate CRC */
static unsigned char serial_buf[SERIAL_FRAME_SIZE];
static unsigned char serial_buf_index = 0;

static u16_t crcByte(u16_t crc, u8_t b) {
  crc = (u8_t)(crc >> 8) | (crc << 8);
  crc ^= b;
  crc ^= (u8_t)(crc & 0xff) >> 4;
  crc ^= crc << 12;
  crc ^= (crc & 0xff) << 5;
  return crc;
}


inline void esc_write(char c)
{
	  /* test if bytes need to be escaped
		7d -> 7d 5d
		7e -> 7d 5e */
	  if (c == 0x7d){
		  uart1_writeb(0x7d);
		  uart1_writeb(0x5d);
	  } else if (c == 0x7e){
		  uart1_writeb(0x7d);
		  uart1_writeb(0x5e);
	  } else {
		  uart1_writeb(c);
	  }
}


int
putchar(int c)
{
  char ch = ((char) c);
  if (serial_buf_index < SERIAL_FRAME_SIZE){
	  serial_buf[serial_buf_index] = ch;
	  serial_buf_index++;
  }
  if (serial_buf_index == SERIAL_FRAME_SIZE || ch == '\n') {

	  u8_t msgID = AM_WILAB_CONTIKI_PRINTF; // TODO look up type?

	  /* calculate CRC */
	  u16_t crc;
	  crc = 0;
	  crc = crcByte(crc, 0x45); // Pr byte
	  crc = crcByte(crc, 0x00);
	  crc = crcByte(crc, 0x0FF); crc = crcByte(crc, 0x0FF); // dest bytes
	  crc = crcByte(crc, 0x00); crc = crcByte(crc, 0x00); // src bytes
	  crc = crcByte(crc, SERIAL_FRAME_SIZE); // len byte
	  crc = crcByte(crc, 0x00);
	  crc = crcByte(crc, msgID);
	  /* XXX since all of the above are constant, do we need to buffer
	   * the characters to calculate CRC? Maybe we don't need the buffer? */
	  int i;
	  for (i=0; i<serial_buf_index; i++){
		  crc = crcByte(crc, serial_buf[i]);
	  }
	  for (i=serial_buf_index; i<SERIAL_FRAME_SIZE; i++){
		  crc = crcByte(crc, 0); // pad with zeroes
	  }

	  /* send message */
	  uart1_writeb(0x7E);
	  uart1_writeb(0x45);
	  uart1_writeb(0x00);
	  uart1_writeb(0x0FF); uart1_writeb(0x0FF);
	  uart1_writeb(0x00); uart1_writeb(0x00);
	  esc_write(SERIAL_FRAME_SIZE);
	  uart1_writeb(0x00);
	  esc_write(msgID);
	  for (i=0; i<serial_buf_index; i++){
		  esc_write(serial_buf[i]);
	  }
	  for (i=serial_buf_index; i<SERIAL_FRAME_SIZE; i++){
		  uart1_writeb(0); // pad with zeroes
	  }
	  // crc in reverse-byte oreder
	  esc_write((u8_t)(crc & 0x00FF));
	  esc_write((u8_t)((crc & 0xFF00) >> 8));
	  uart1_writeb(0x7E);

	  serial_buf_index = 0;
  }
  return c;
}

#else

int
putchar(int c)
{
  uart1_writeb((char)c);
  return c;
}

#endif


