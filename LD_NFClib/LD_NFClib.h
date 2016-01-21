#include <Arduino.h>
#include <Stream.h>
#include <avr/pgmspace.h>

class LD_NFC
{
	public:
		LD_NFC(Stream *serial, Stream *serialDebug);
		LD_NFC(Stream *serial);
		void wakeUp();
		void scan();
		int passWordCheck(int block,unsigned char id[],unsigned char st[]);
		void writeData(int block,unsigned char dwic[]);
		void readData(int block);
		
	private: 
		unsigned char readAck();
		char checkDCS(int x);  //  NFC  S50卡  DCS校验检测子函数
		void readReceive(int x);
	
	private:
		Stream *serial;                                            
		Stream *serialDebug;
		unsigned char password[6];
		bool isDebug;
		unsigned char receive_ACK[35];
};