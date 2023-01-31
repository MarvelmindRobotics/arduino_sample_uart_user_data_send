/*
 *   This simple example sends user data to the hedgehog or modem via UART
 */

/*
  The circuit:
 * Serial data from hedgehog : digital pin 0 (RXD)
 * Serial data to hedgehog : digital pin 1 (TXD)
 * LCD RS pin : digital pin 8
 * LCD Enable pin : digital pin 9
 * LCD D4 pin : digital pin 4
 * LCD D5 pin : digital pin 5
 * LCD D6 pin : digital pin 6
 * LCD D7 pin : digital pin 7
 * LCD BL pin : digital pin 10
 *Vcc pin :  +5
 */

#include <LiquidCrystal.h>

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//  MARVELMIND HEDGEHOG RELATED PART

typedef union {byte b[2]; unsigned int w;} uni_8x2_16;

///

#define HEDGEHOG_BUF_SIZE 250 
byte hedgehog_serial_buf[HEDGEHOG_BUF_SIZE];

#define USER_FRAME_SIZE 32
uint8_t user_data_buf[USER_FRAME_SIZE];
uint8_t user_packet_counter;

#define USER_DATA_RATE_MSEC 125 /* 8 Hz */

////////////////////////////////////////////////////////////////////////////

//    Marvelmind hedgehog support initialize
void setup_hedgehog() 
{
  Serial.begin(500000); // hedgehog transmits data on 500 kbps  

  user_packet_counter= 0;
}

////////////////////////////////////////

void hedgehog_send_packet(byte address, byte *data_buf, byte data_size)
{byte frameSizeBeforeCRC;
 byte i;

   hedgehog_serial_buf[0]= address;

   hedgehog_serial_buf[1]= 0x49;

   hedgehog_serial_buf[2]= 0x00;
   hedgehog_serial_buf[3]= 0x02;// 0x200 - user payload data

   hedgehog_serial_buf[4]= data_size;

   for(i=0;i<data_size;i++)
    hedgehog_serial_buf[5 + i]= data_buf[i];

   hedgehog_set_crc16(&hedgehog_serial_buf[0], data_size+5);

   Serial.write(hedgehog_serial_buf, data_size+7);
}

// Sends user data 
void hedgehog_send_user_data()
{byte i;

  // ---- Fill payload data begin 
  for(i=0;i<USER_FRAME_SIZE;i++)
    user_data_buf[i]= user_packet_counter++;
  // ---- Fill payload data end
  
  hedgehog_send_packet(0, &user_data_buf[0], USER_FRAME_SIZE); 
}

////////////////////////////////////////

// Calculate CRC-16 of hedgehog packet
void hedgehog_set_crc16(byte *buf, byte size)
{uni_8x2_16 sum;
 byte shift_cnt;
 byte byte_cnt;

  sum.w=0xffffU;

  for(byte_cnt=size; byte_cnt>0; byte_cnt--)
   {
   sum.w=(unsigned int) ((sum.w/256U)*256U + ((sum.w%256U)^(buf[size-byte_cnt])));

     for(shift_cnt=0; shift_cnt<8; shift_cnt++)
       {
         if((sum.w&0x1)==1) sum.w=(unsigned int)((sum.w>>1)^0xa001U);
                       else sum.w>>=1;
       }
   }

  buf[size]=sum.b[0];
  buf[size+1]=sum.b[1];// little endian
}// hedgehog_set_crc16

//  END OF MARVELMIND HEDGEHOG RELATED PART
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);

void setup()
{
  lcd.clear(); 
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Sends user data"); 

  setup_hedgehog();//    Marvelmind hedgehog support initialize
}

void loop()
{
   delay(USER_DATA_RATE_MSEC);
   
   hedgehog_send_user_data();// Send user data to hedgehog
}
