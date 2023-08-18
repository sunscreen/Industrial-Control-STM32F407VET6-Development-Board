void empty_buffer(uint8_t *buff) {
	for (int z=0;z<40;z++) buff[z]=0;
}

uint8_t rx_byte =0;
uint8_t ibusdata_len=0;
uint8_t inbuff[40]={0x00};
uint8_t ibusdata[40]={0x00};
uint8_t sync_status = 0;

int IbusAvailable() {
uint16_t Len=0;
uint8_t sync_pos=-1;

Len=SoftUartRxAlavailable(0);
		if(Len > 0)
		{

			for (int z = 0; z < 40; z++) inbuff[z] = 0;
			// Attempt to Buffer up bytes
			if(SoftUartReadRxBuffer(0,inbuff,Len) == SoftUart_OK)
			{
				if (sync_status == 0) {
				  sync_pos=checkIbusSync(inbuff,Len);
				  if (sync_pos == -1) return 0; /* discard can not find sync byte */
				  if (sync_pos > -1) {sync_status=1;}

				} else {
				  sync_pos=0;
				}

				memcpy(ibusdata+ibusdata_len,inbuff+sync_pos,Len);
				rx_bytes=+Len;
				ibusdata_len=+Len;

				int res = checkIbusFrame(ibusdata,ibusdata_len);
				if (res == 1) {

				//uint8_t checksum_valid=checkIbusMessage(ibusdata);
				uint8_t tst=extractIbusMessages(ibusdata,ibusdata_len);
				ibusdata_len = 0;
				empty_buffer(ibusdata);
				return tst;
				}
				if (ibusdata_len > 7) { /* failed to buffer any thing meaningful discard */
				empty_buffer(ibusdata);
				ibusdata_len = 0;
				}
				return res;
			}
		}
	  return 0;
}
int checkIbusSync(uint8_t *checkbuf, uint8_t Len) {
	for (uint8_t z=0;z<Len;z++) {
		if (checkbuf[z] == 0x80) return z;
	}
return -1;
}
int checkIbusFrame(uint8_t *checkbuf,int Len) {
	uint8_t frame_len=checkbuf[1];
	if (Len < 4) {return 0;}
	if ((Len-2) < frame_len) return 0;
	if (frame_len > 16) return 0;
	if (frame_len == 0) return 0;
	if (frame_len == (Len-2)) return 1;


	return 0;
	/* FRAME INCOMPLETE */
	/* FRAME OK */

}
uint8_t ib_msgs[8][32]={0x0};

int extractIbusMessages(uint8_t *ibus_buf,int Len) {
		int msgindx=0;
		if (Len <4) return 0;

		for (uint8_t z=0;z<Len;z++) {
		uint8_t sender = ibus_buf[z];
		uint8_t length = ibus_buf[z+1];
	    uint8_t chksum = sender ^ length;
	    if (length == 0) continue;
	    if (length > 16 || length > Len) continue;

	    for (uint16_t i = 2; i < length+1; i++) {
	      chksum = chksum ^ ibus_buf[z+i];
	    }
	    if (ibus_buf[z+length+1] == chksum) {
	      // checksums match, buffer contains a valid message

	    	memcpy(ib_msgs[msgindx],ibus_buf+z,length+2);
	    	rawsend(ib_msgs[msgindx],length+2);
	    	msgindx++;
	    	if ((length+2) < Len) z=z+length+2;

	     //return 1;
	    }

		}
return msgindx;
}

int checkIbusMessage(uint8_t *ibus_buf) {

		uint8_t sender = ibus_buf[0];
		uint8_t length = ibus_buf[1];
	    uint8_t chksum = sender ^ length;
	    for (uint16_t i = 2; i < length+1; i++) {
	      chksum = chksum ^ ibus_buf[i];
	    }
	    if (ibus_buf[length+1] == chksum) {
	      // checksums match, buffer contains a valid message
	      return 1;
	    }
	    else {
	      // message received with invalid checksum: discard buffer
	      return 0;
	    }

}
