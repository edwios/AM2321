/**************************************************************** 
  This is the Spark Core library for the AM2321 Humidity and Temperature sensor

  This sensor uses custom protocol based on the I2C communication
  and supports both one and two wires configuration.
  
  This Library only supports the 2 wires configuration.
  
  Written by Ed Wios.
  Oct 2014
 ***************************************************************/
 
#include "AM2321.h"

AM2321::AM2321() {
  SSDA = _SSDA;
  SSCL = _SSCL;
}

void delay10us(void)
{
	delayMicroseconds(10);
}

/*************************************************
	I2C Start sequence 
*************************************************/
void AM2321::I2C_Start()
{
  pinMode(SSDA, OUTPUT);
  digitalWrite(SSDA,1);
  digitalWrite(SSCL,1);
  delay10us();
  digitalWrite(SSDA,0);
  delay10us();
  digitalWrite(SSCL,0); 
}

/*************************************************
	I2C Stop sequence
*************************************************/
void AM2321::I2C_Stop()
{
	pinMode(SSDA, OUTPUT);
	digitalWrite(SSDA,0);
	delay10us();
	digitalWrite(SSCL,1);
	delay10us();
	digitalWrite(SSDA,1);
}

/*************************************************
	I2C Ack sequence 
*************************************************/
void AM2321::Ack(void)
{
	pinMode(SSDA, OUTPUT);
	digitalWrite(SSDA,0);
	digitalWrite(SSCL,0);
	delay10us();
	digitalWrite(SSCL,1);
	delay10us(); 
	digitalWrite(SSCL,0);
	digitalWrite(SSDA,1);
}

/*************************************************
	AM2321 specific: No ACK back to I2C slave 
*************************************************/
void AM2321::NoAck(void)
{
	pinMode(SSDA, OUTPUT);
	digitalWrite(SSDA,1);  
	digitalWrite(SSCL,0);
	delay10us();
	digitalWrite(SSCL,1);
	delay10us();
	digitalWrite(SSDA,1);
	digitalWrite(SSCL,0);
}

/*************************************************
	Test if AM2321 has returned ACK 
*************************************************/
int AM2321::Test_Ack()
{
	int ACK_Flag=0;
	digitalWrite(SSCL,0);
	pinMode(SSDA, INPUT);	
	delay10us();
	digitalWrite(SSCL,1);
	delay10us();
	if (digitalRead(SSDA) == 0)
		ACK_Flag = 1;
	else 
		ACK_Flag = 0;
	digitalWrite(SSCL,0);
	pinMode(SSDA, OUTPUT);	 
	digitalWrite(SSDA,1);
	return ACK_Flag;
}

/*************************************************
	Send 1 byte of data to the I2C slave
	Will not consider ACK from slave
*************************************************/
void AM2321::SendData(unsigned char buffer)
{
	unsigned char BitCnt=8; // 8-bit
	pinMode(SSDA, OUTPUT);
	digitalWrite(SSDA,1);
	do {
		digitalWrite(SSCL,0);
		delay10us();
		if((buffer&0x80)==0)
			digitalWrite(SSDA,0);
		else
			digitalWrite(SSDA,1);
		digitalWrite(SSCL,1);
		delay10us();
		buffer=buffer<<1; // Left shift buffer
		BitCnt--;
	} while(BitCnt);
	digitalWrite(SSCL,0);		 
}

/*************************************************
	Receive 1 byte from I2C slave into the buffer 
	Must be followed by ACK() afterwards
*************************************************/
unsigned char AM2321::ReceiveData()
{
	unsigned char BitCnt=8,IIC_RX_Data=0;
	pinMode(SSDA,INPUT);
	do {
		digitalWrite(SSCL,0);
		delay10us();	 
		IIC_RX_Data <<= 1;
		BitCnt--;	  
		digitalWrite(SSCL,1);
		delay10us();
		if(digitalRead(SSDA)==1)
			IIC_RX_Data = IIC_RX_Data|0x01;
		else
			IIC_RX_Data = IIC_RX_Data&0xfe;	  
	} while(BitCnt);
	digitalWrite(SSCL,0);
	return IIC_RX_Data;
}

/*************************************************
	Write bytes to I2C slave from the buffer
	by calling SendData
	Must be followed by ACK() afterwards
*************************************************/
int AM2321::WriteNByte(unsigned char sla,unsigned char *s,unsigned char n)
{
	unsigned char i;
	
	I2C_Start();
	SendData(sla); // Send device address
	if(!Test_Ack()) {	
		WR_Flag = 1;
		return(0);
	}
	for(i=0;i<n;i++) { // Send buffer to slave
		SendData(*(s+i));
		if(!Test_Ack()) {
			WR_Flag = 2;
			return(0);
		}
	}
	I2C_Stop();
	return(1);
}

int AM2321::ReadNByte(unsigned char Sal, unsigned char *p,unsigned char n)
{
	unsigned char i;
	I2C_Start();
	SendData((Sal)| 0x01); // Send device Read address
	if(!Test_Ack()) {
		WR_Flag = 1;
		return(0);
	}
	// Then must wait > 30us according to manual
	delay10us();	
	delay10us();
	delay10us(); 
		
	// Now we can start reading data from AM2321
	for(i=0;i<n-1;i++) {
		*(p+i)=ReceiveData();
		Ack(); 
	}
	*(p+n-1)=ReceiveData();		 
	NoAck();
	I2C_Stop();
	return(1);	 
}

/*************************************************
	Calc CRC from received buffer 
*************************************************/
unsigned int AM2321::CRC16(unsigned char *ptr, unsigned char len)
{
	unsigned int crc=0xffff;
	unsigned char i;
	while(len--) {
		crc ^=*ptr++;
		for(i=0;i<8;i++) {
			if(crc & 0x1) {
				crc>>=1;
				crc^=0xa001;
			} else {
				crc>>=1;
			}
		}
	}
	return crc;
}

/*************************************************
	Affirm if CRC matches 
*************************************************/
unsigned char AM2321::CheckCRC(unsigned char *ptr,unsigned char len)
{
	unsigned int crc;
	crc=(unsigned int)CRC16(ptr,len-2);
	if(ptr[len-1]==(crc>>8) && ptr[len-2]==(crc & 0x00ff)) {
		return 0xff;
	} else {
		return 0x0;
	}
}

/*************************************************
	Wake up the AM2321 
	AM2321 will NOT ack when wake up, however,
	according to the manual, ACK clock MUST be 
	sent or error will occur.
	A delay of at least 1.5ms is REQUIRED
	before I2C stop.
*************************************************/

void AM2321::WakeUp(void)
{
	I2C_Start();
	SendData(IIC_Add);		  // Send device address
	Test_Ack();				  // AM2321 will not ACK when waking up
	delayMicroseconds(2000);  // Delay at least 1.5ms
	I2C_Stop(); 
}


/*************************************************
	Clear the buffer 
*************************************************/
void AM2321::Clear_Data (void)
{
	int i;
	for(i=0;i<IIC_RX_Length;i++) {
		IIC_RX_Buffer[i] = 0x00;
	}
}

/*************************************************
	Initialize and ready the I/O pins
	Clear the buffer
*************************************************/

bool AM2321::begin()
{
	SSDA = _SSDA;
	SSCL = _SSCL;

	pinMode(SSCL, OUTPUT);
	pinMode(SSDA, OUTPUT);
	digitalWrite(SSCL,1);
	digitalWrite(SSDA,1);
	Clear_Data();

	// Todo: check and verify Device ID and
	// return FAIL if device ID not match
	return true;
}

/*************************************************
	Read the device ID of AM2321 
	Shall return 0x2321
	Return 0xFFFF when error occured
*************************************************/
unsigned int AM2321::readID()
{
	unsigned int m = 0;

	Clear_Data();
	WR_Flag = 0;
	WakeUp();
	IIC_TX_Buffer[0] = 0x03;
	IIC_TX_Buffer[1] = 0x08;
	IIC_TX_Buffer[2] = 0x07;
	if (WriteNByte(IIC_Add,IIC_TX_Buffer,3)) {
		//Wait at least 2ms for device to response with data
		delayMicroseconds(2000);	
		ReadNByte(IIC_Add,IIC_RX_Buffer,7);
		digitalWrite(SSCL,1); digitalWrite(SSDA,1); //Release the lines
		if (!WR_Flag) {
			m = IIC_RX_Buffer[2]*256+IIC_RX_Buffer[3];
		} else {
			m = 0xFFFF;
		}
	} else {
		m = 0xFFFF;
	}
	return m;
}

/*************************************************
	Read the temperature from AM2321 
	Result returned as a float
	Returns 999.9 if error occured and cannot
	read form the device
*************************************************/
float AM2321::readTemperature()
{
	float temp = 0.0;

	Clear_Data();
	WR_Flag = 0;
	WakeUp();
	IIC_TX_Buffer[0] = 0x03;
	IIC_TX_Buffer[1] = 0x00;
	IIC_TX_Buffer[2] = 0x04;
	if (WriteNByte(IIC_Add,IIC_TX_Buffer,3)) {
		//Wait at least 2ms for device to response with data
		delayMicroseconds(2000);	
		ReadNByte(IIC_Add,IIC_RX_Buffer,8);
		digitalWrite(SSCL,1); digitalWrite(SSDA,1);
		if (!WR_Flag) {
			temp = (IIC_RX_Buffer[4]*256+IIC_RX_Buffer[5])/10.0;
		} else {
			temp = 999.9;
		}
	} else {
		temp = 999.9;
	}
	return temp;
}

/*************************************************
	Read the humidity from AM2321 
	Result returned as a float
	Returns 999.9 if error occured and cannot
	read form the device
*************************************************/
float AM2321::readHumidity()
{
	float humi = 0.0;
	
	Clear_Data();
	WR_Flag = 0;
	WakeUp();
	IIC_TX_Buffer[0] = 0x03;
	IIC_TX_Buffer[1] = 0x00;
	IIC_TX_Buffer[2] = 0x04;
	if (!WriteNByte(IIC_Add,IIC_TX_Buffer,3)) {
		humi = 999.9;
	} else {
		//Wait at least 2ms for device to response with data
		delayMicroseconds(2000);	
		ReadNByte(IIC_Add,IIC_RX_Buffer,8);
		digitalWrite(SSCL,1); digitalWrite(SSDA,1);
		if (!WR_Flag) {
			humi = (IIC_RX_Buffer[2]*256+IIC_RX_Buffer[3])/10.0;
		} else {
			humi = 999.9;
		}
	}
	return humi;
}

/*************************************************
	Read both temperature and humidity from AM2321 
	Results returned as unsigned long
	aaaabbbb where aaaa is the humidity*10 and
	bbbb is temperature*10
*************************************************/
unsigned long AM2321::readAll()
{
	unsigned long m = 0;

	Clear_Data();
	WR_Flag = 0;
	WakeUp();
	IIC_TX_Buffer[0] = 0x03;
	IIC_TX_Buffer[1] = 0x00;
	IIC_TX_Buffer[2] = 0x04;
	if (!WriteNByte(IIC_Add,IIC_TX_Buffer,3)) {
		m = 0xFFFFFFFF;
	} else {
		//Wait at least 2ms for device to response with data
		delayMicroseconds(2000);	
		ReadNByte(IIC_Add,IIC_RX_Buffer,8);
		digitalWrite(SSCL,1); digitalWrite(SSDA,1);
		if (!WR_Flag) {
			m = (IIC_RX_Buffer[2]<<24) + (IIC_RX_Buffer[3]<<16) + (IIC_RX_Buffer[4]<<8) + IIC_RX_Buffer[5];
		} else {
			m = 0xFFFFFFFF;
		}
	}
	return	m;	 
}
