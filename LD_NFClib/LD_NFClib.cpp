#include "LD_NFClib.h"

LD_NFC::LD_NFC(Stream *serial)
{
	this->serial=serial;
	this->serialDebug = serialDebug;
	this->isDebug = false;
}


LD_NFC::LD_NFC(Stream *serial, Stream *serialDebug)
{
	this->serial=serial;
	this->serialDebug = serialDebug;
	this->isDebug = true;
}


void LD_NFC::wakeUp(){//wake up NFC module
	const unsigned char wake[24]={
	0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x03, 0xfd, 0xd4, \
	0x14, 0x01, 0x17, 0x00
	};
	for(int i=0;i<24;i++) {//send command
		this->serial->write(wake[i]);
	}
	delay(10);
	readReceive(15);
	if (this->isDebug){
		for(int i=0;i<15;i++)  {this->serialDebug->print(receive_ACK[i],HEX);this->serialDebug->print(" ");}
		this->serialDebug->println();
	}
}

/*cardbaudrate is the baud rate and the modulation type to be used during the initialization
− 0x00 : 106 kbps type A (ISO/IEC14443 Type A),
− 0x01 : 212 kbps (FeliCa polling),
− 0x02 : 424 kbps (FeliCa polling),
− 0x03 : 106 kbps type B (ISO/IEC14443-3B),
− 0x04 : 106 kbps Innovision Jewel tag. */

uint8_t LD_NFC::scanReadTargetID(unsigned char cardbaudrate,unsigned char *targetID, uint8_t * uidLength){
	uint8_t commandbuffer[3];
	commandbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
	commandbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
	commandbuffer[2] = cardbaudrate;
	this->serialDebug->println("sendCommandWaitAck");
	if (!sendCommandWaitAck(commandbuffer,3,100)){
		this->serialDebug->print("no cards found");
		return 0;
	}
	delay (10);
	readReceive(20); 
	for(int i=0;i<10;i++)  {this->serialDebug->print(this->receive_ACK[i],HEX); this->serialDebug->print(" ");}
		this->serialDebug->println();
	
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
	
	// while(Serial1.available()){
		// this->serialDebug->print(this->serial->read(),HEX); 
		// this->serialDebug->print(" ");
	// }
	if (this->isDebug){
		this->serialDebug->print(F("Found ")); this->serialDebug->print(receive_ACK[7], DEC); this->serialDebug->println(F(" tags"));
	}
	
	if (receive_ACK[7] != 1) 
		return 0;
		
	uint16_t sens_res = receive_ACK[9];
	sens_res <<= 8;
	sens_res |= receive_ACK[10];
	if (this->isDebug){
		this->serialDebug->print(F("ATQA: 0x"));  this->serialDebug->println(sens_res, HEX);
		this->serialDebug->print(F("SAK: 0x"));  this->serialDebug->println(receive_ACK[11], HEX);
	}
	
	/* Card appears to be Mifare Classic */
	*uidLength = receive_ACK[12];
	if (this->isDebug){
		this->serialDebug->print(F("UID:"));
	}
	for (uint8_t i=0; i < receive_ACK[12]; i++) 
	{
		targetID[i] = receive_ACK[13+i];
		if (this->isDebug){
			this->serialDebug->print(F(" 0x"));this->serialDebug->print(targetID[i], HEX);
		}
	}
	if (this->isDebug){
		this->serialDebug->println();
	}

    return 1;
	delay(10); 
}


int LD_NFC::passWordCheck(int block,unsigned char id[],unsigned char st[]){
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
	for(int i=16;i<20;i++) cmdPassWord[i]=id[i-16];// UID
	for(int i=0;i<20;i++) sum+=cmdPassWord[i];
	cmdPassWord[20]=0xff-sum&0xff;

	if (this->isDebug){
		this->serialDebug->println("passWordSend: ");
		for(int i=0;i<22;i++)  {this->serialDebug->print(cmdPassWord[i],HEX); this->serialDebug->print(" ");}
		this->serialDebug->println();
	}
	
	do{
		while(Serial1.available()) char xx=Serial1.read();//clear the serial data
		for (int i=0;i<22;i++) this->serial->write(cmdPassWord[i]);
	} while(readAck(100)!=0x01);
	
	while(this->serial->available()){
		this->receive_ACK[count]=this->serial->read();
		count++;
	}
	this->serialDebug->println(count);
	//10个数据
	//readReceive(16);delay(10);
	
	if (this->isDebug){
		this->serialDebug->println("passWordRes: ");
		for(int i=0;i<count;i++)  {this->serialDebug->print(this->receive_ACK[i],HEX); this->serialDebug->print(" ");}
		this->serialDebug->println();
	}
	if(checkDCS(10)==1 && this->receive_ACK[6]==0x41 && this->receive_ACK[7]==0x00)  return 1;
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
	
	do{
		//this->serial->flush();
		while(Serial1.available())   char xx=Serial1.read();//clear the serial data
		for(int i=0;i<12;i++){this->serial->write(cmdRead[i]);}
		delay(10);
	} while(readAck(10)!=0x01);
	
	while(this->serial->available()){    
		this->receive_ACK[count]=this->serial->read();
		count++;
	}
	
	if (this->isDebug){
		this->serialDebug->println("Read data:   ");
		for(int i=0;i<count;i++)  {this->serialDebug->print(this->receive_ACK[i],HEX); this->serialDebug->print(" ");}
		this->serialDebug->println();
		//    DCS校验是否成功        这两位数据上为0x41和0x00则表示操作成功
		if(checkDCS(26)==1 && this->receive_ACK[6]==0x41 && this->receive_ACK[7]==0x00)  this->serialDebug->println("Finish Reading");
		this->serialDebug->println(" ");
	}
}

void  LD_NFC::writeData(int block,unsigned char dwic[])//  block：待写入数据的块，dwic[]待写入的数据
{
	//00 00 ff 15 EB D4 40 01 A0 06 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F CD 00
	unsigned char cmdWrite[28]={
	0x00,0x00,0xff,0x15,0xEB,0xD4,0x40,0x01,0xA0,0x06, \
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09, \
	0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xCD,0x00
	};
	unsigned char sum=0,count=0;
	cmdWrite[9]=block;
	for(int i=10;i<26;i++) cmdWrite[i]=dwic[i-10];// 待写入的数据
	for(int i=0;i<26;i++) sum+=cmdWrite[i];//加和
	cmdWrite[26]=0xff-sum&0xff;//  计算DCS
	
	do{
		this->serial->flush();
		for(int i=0;i<28;i++) this->serial->write(cmdWrite[i]);
		delay(10);
	} while(readAck(10)!=0x01);
	
	while(this->serial->available())
	{
		receive_ACK[count]=this->serial->read();
		count++;
	}
	
	if (this->isDebug)
	{   
		this->serialDebug->print("Write respond:   ");
		for(int i=0;i<count;i++)  {this->serialDebug->print(receive_ACK[i],HEX); this->serialDebug->print(" ");}
		this->serialDebug->println("     Write respond End ");
		if(checkDCS(11)==1 && receive_ACK[7]==0x41 && receive_ACK[8]==0x00)
		{
			this->serialDebug->println("WriteFinish!");
		}
	}
}

//判断ack
unsigned char LD_NFC::readAck(uint16_t timeout)
{
	unsigned char receiveData[6];
	unsigned char i=0,j=0;
	const unsigned char ack[6]={0x00,0x00,0xff,0x00,0xff,0x00};
	const unsigned char nack[6]={0x00,0x00,0xff,0xff,0x00,0x00};
	uint16_t time=0;
	//wait 6 data received
	while (this->serial->available()<6){
		delay(1);
		time++;
		//if (time>timeout)return 0x00;
	}
	this->serialDebug->println(this->serial->available());
	this->serialDebug->println("read ACKing");
	for(int x=0;x<6;x++){
		receiveData[x]= this->serial->read();
		if (receiveData[x]==ack[x])i++;
		if (receiveData[x]==nack[x])j++;
	}
	if (i==6)return 0x01;
	else if (j==6){
		if (this->isDebug){
			this->serialDebug->println("Receive nACK!");
		}
		return 0xff;
	}
	else{
		if (this->isDebug){
			this->serialDebug->println("Cant Receive error ACK!");
		}
		return 0x00;
	}
}

unsigned char LD_NFC::sendCommandWaitAck(unsigned char *command,unsigned char commandLEN,uint16_t timeout){
	unsigned char temp=0;
	uint8_t checksum=0;

	//clear the serial data
	while(this->serial->available())   char xx=this->serial->read();
	checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
	this->serial->write((uint8_t)PN532_PREAMBLE);
	this->serial->write((uint8_t)PN532_PREAMBLE);
	this->serial->write((uint8_t)PN532_STARTCODE2);

	this->serial->write(commandLEN+1);
	this->serial->write((uint8_t)(~(commandLEN+1)) + 1);
	
	this->serial->write((uint8_t)PN532_HOSTTOPN532);
	checksum += PN532_HOSTTOPN532;
	
	for(int i=0;i<commandLEN;i++){
		this->serial->write(command[i]);
		checksum += command[i];
	}
	this->serial->write((uint8_t)(~checksum));
	this->serial->write((uint8_t)PN532_POSTAMBLE);
	
	this->serialDebug->println("read ACK");
	return readAck(timeout);
}

//读取x个串口发来的数据
void LD_NFC::readReceive(int x) 
{
	unsigned char i;
	//while(this->serial->available()<x);
	this->serialDebug->println(this->serial->available());
	for(i=0;i<x;i++){
		this->receive_ACK[i]= this->serial->read();
	}
}


char LD_NFC::checkDCS(int x)  //NFC S50卡 DCS校验检测子函数
{
	unsigned char sum=0,dcs=0;
	for(int i=0;i<x-2;i++){
		sum+=this->receive_ACK[i];
	}
	dcs=0xff-sum&0xff;
	if(dcs==this->receive_ACK[x-2])  return 1;
	else return 0;
}