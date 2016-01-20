#include "LD_NFClib.h"

LD_NFC::LD_NFC(Stream *serial, Stream *serialDebug)
{
	this->serial=serial;
	this->serialDebug = serialDebug;
	this->isDebug = true;
}


void LD_NFC::wakeUp(){
	const unsigned char wake[24]={
	0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xfd, 0xd4, 0x14, 0x01, 0x17, 0x00};//wake up NFC module
	for(int i=0;i<24;i++) //send command
	{
		this->serial->write(wake[i]);
	}
	delay(10);
	readAck(15);
	if (this->isDebug)
	{
		for(int i=0;i<15;i++)  this->serialDebug->print(receive_ACK[i]);
		this->serialDebug->println();
	}
}


void LD_NFC::scan(){
	const unsigned char cmdUID[11]={ 0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};
	//0 0 FF 0 FF 0 0 0 FF C F4 D5 4B 1 1 0 4 8 4 D1 AA 40 EA 29 0 
	//0 0 FF 0 FF 0 ----ACK
	//0 0 FF C F4 
	//D5---PN532 to Arduino
	//4B----respond command 
	//1 1----target ID  and target amount
	//0 4----atq 
	//8----capacity of the card  is 8K
	//4 ---- 4 numbers of the UID
	//D1 AA 40 EA----UID 
	//29 0------DCS  POST---  DCS=0xff&(SUM(0 0 FF C F4 D5 4B 1 1 0 4 8 4 D1 AA 40 EA))
	unsigned char NFC_UID[5],count=0; 
	for(int i=0;i<11;i++)  this->serial->write(cmdUID[i]);
	delay(10); 
	readAck(25);
	delay(10); 
}

int LD_NFC::passWordCheck(int block,unsigned char id[],unsigned char st[])
{
	//---------head-------  card 1  check blocknumber  password               UID D1 AA 40 EA      DCS+POST
	//00 00 FF 0F F1 D4 40    01     60    07          FF FF FF FF FF FF       02 F5 13 BE           C2 00
	
	//out: 00 00 FF 00 FF 00 00 00 FF 03 FD D5 41 00 EA 00  ------41 00 //正确状态    16Bytes
	//     0  0  FF 0  FF 0  0  0  FF  3 FD D5 41 27 C3 0 
	unsigned char cmdPassWord[22]={0x00,0x00,0xFF,0x0F,0xF1,0xD4,0x40,0x01,0x60,\
								0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xD1,0xAA,0x40,0xEA,0xC2,0x00};                                 
	unsigned char sum=0,count=0;
	unsigned char UID[4]={0xD1,0xAA,0x40,0xEA};
	cmdPassWord[9]=block;
	for(int i=10;i<16;i++) cmdPassWord[i]=st[i-10];// 密码
	for(int i=16;i<20;i++) cmdPassWord[i]=UID[i-16];// UID
	for(int i=0;i<20;i++) sum+=cmdPassWord[i];
	cmdPassWord[20]=0xff-sum&0xff;

	if (this->isDebug)
	{
		this->serialDebug->println("passWordSend: ");
		for(int i=0;i<22;i++)  {this->serialDebug->print(cmdPassWord[i],HEX); this->serialDebug->print(" ");}
		this->serialDebug->println();
	}
	
	
	while(this->serial->available())   char xx=this->serial->read();//clear the serial data
	
	for (int i=0;i<22;i++)  this->serial->write(cmdPassWord[i]);
	delay(100);
	while(this->serial->available())
	{
		this->receive_ACK[count]=this->serial->read();
		count++;
	}
	//readAck(16);delay(10);
	
	if (this->isDebug)
	{
		this->serialDebug->println("passWordRes: ");
		for(int i=0;i<16;i++)  {this->serialDebug->print(this->receive_ACK[i],HEX); this->serialDebug->print(" ");}
		this->serialDebug->println();
	}
	if(checkDCS(16)==1 && this->receive_ACK[12]==0x41 && this->receive_ACK[13]==0x00)  return 1;
	else return 0; 
}

void LD_NFC::readData(int block)// 读取数据  block---要读取的块
{
	//----head--------- cmd  card 1   readcmd   block 07  DCS+POST
	//00 00 ff 05 fb D4  40    01      30        07        B4 00 //读第7块
	//out: 00 00 FF 00 FF 00 //ACK
	//------------------41 00for right  41 03wrong        data                                           DCS+POST
	//00 00 FF 13 ED D5 41 00                        00 00 00 00 00 00 FF 07 80 69 FF FF FF FF FF FF     01 00  //7块
	
	//test back:00 00 FF 00 FF 00 00 00 FF 13 ED D5 41 00 01 01 02 02 03 03 04 04 05 05 06 06 07 07 08 08 A2 00 
	unsigned char cmdRead[12]={0x00,0x00,0xff,0x05,0xfb,0xD4,0x40,0x01,0x30,0x07,0xB4,0x00};
	unsigned char sum=0,count=0;
	cmdRead[9]=block;
	for(int i=0;i<10;i++) sum+=cmdRead[i];
	cmdRead[10]=0xff-sum&0xff;
	
	while(this->serial->available())   char xx=this->serial->read();//clear the serial data
	
	//Serial.print("readDCS:");Serial.println(cmdRead[10],HEX);
	for(int i=0;i<12;i++){this->serial->write(cmdRead[i]);}
	delay(10);
	while(this->serial->available())
	{    
		this->receive_ACK[count]=this->serial->read();
		count++;
	}
	
	if (this->isDebug)
	{
		this->serialDebug->println("Read data:   ");
		for(int i=0;i<count;i++)  {this->serialDebug->print(this->receive_ACK[i],HEX); this->serialDebug->print(" ");}
		this->serialDebug->println();
		//    DCS校验是否成功        这两位数据上为0x41和0x00则表示操作成功
		if(checkDCS(32)==1 && this->receive_ACK[12]==0x41 && this->receive_ACK[13]==0x00)  this->serialDebug->println("Finish Reading");
	
		this->serialDebug->println(" ");
	}
}

void  LD_NFC::writeData(int block,unsigned char dwic[])//  block：待写入数据的块，dwic[]待写入的数据
{
	//00 00 ff 15 EB D4 40 01 A0 06 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F CD 00
	unsigned char cmdWrite[]={0x00,0x00,0xff,0x15,0xEB,0xD4,0x40,0x01,0xA0, \
							0x06,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, \
							0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xCD,0x00};
	unsigned char sum=0,count=0;
	cmdWrite[9]=block;
	for(int i=10;i<26;i++) cmdWrite[i]=dwic[i-10];// 待写入的数据
	for(int i=0;i<26;i++) sum+=cmdWrite[i];//加和
	cmdWrite[26]=0xff-sum&0xff;//  计算DCS
	
	while(this->serial->available())   char xx=this->serial->read();//clear the serial data
	
	for(int i=0;i<28;i++) this->serial->write(cmdWrite[i]);
	while(this->serial->available())
	{
		receive_ACK[count]=this->serial->read();
		count++;
	}
	if (this->isDebug)
	{   
		this->serialDebug->print("Write respond:   ");
		for(int i=0;i<17;i++)  {this->serialDebug->print(receive_ACK[i],HEX); this->serialDebug->print(" ");}
		this->serialDebug->println("     Write respond End ");
		if(checkDCS(17)==1 && receive_ACK[13]==0x41 && receive_ACK[14]==0x00)
		{
			this->serialDebug->println("WriteFinish!");
		}
	}
}



void LD_NFC::readAck(int x) //读取x个串口发来的数据
{
	unsigned char i;
	for(i=0;i<x;i++)
	{
		this->receive_ACK[i]= this->serial->read();
	}
}


char LD_NFC::checkDCS(int x)  //  NFC  S50卡  DCS校验检测子函数
{
	unsigned char sum=0,dcs=0;
	for(int i=6;i<x-2;i++)
	{
		sum+=this->receive_ACK[i];
	}
	dcs=0xff-sum&0xff;
	if(dcs==this->receive_ACK[x-2])  return 1;
	else   return 0;
}