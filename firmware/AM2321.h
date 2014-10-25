#include "application.h"

#define BAUD  9600
#define _SSDA D2
#define _SSCL D3

#define IIC_Add 0xB8			// Device address of AM2321
#define IIC_RX_Length 15		// Max data buffer size


class AM2321 {
public:
	AM2321();
	bool begin();
	void WakeUp(void);
	unsigned int readID();
	float readTemperature(void);
	float readHumidity(void);
	unsigned long readAll(void);
	void SendData(unsigned char buffer);
	unsigned char ReceiveData();

private:

	int SSDA = _SSDA;
	int SSCL = _SSCL;

	unsigned char *String;
	unsigned char WR_Flag;
	unsigned char IIC_TX_Buffer[IIC_RX_Length];	  // Tx Data buffer
	unsigned char IIC_RX_Buffer[IIC_RX_Length];	  // Rx Data buffer

	void I2C_Start();
	void I2C_Stop();

	void Ack(void);
	void NoAck(void);
	int Test_Ack();

	int WriteNByte(unsigned char sla,unsigned char *s,unsigned char n);
	int ReadNByte(unsigned char Sal, unsigned char *p,unsigned char n);
	unsigned int CRC16(unsigned char *ptr, unsigned char len);
	unsigned char CheckCRC(unsigned char *ptr,unsigned char len);
	void Clear_Data (void);

};
