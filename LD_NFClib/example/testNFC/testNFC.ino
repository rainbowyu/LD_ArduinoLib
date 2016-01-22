#include "LD_NFClib.h"
unsigned char dataWriteIntoCard[16]={0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08};
unsigned char UID[4]={0xD1,0xAA,0x40,0xEA};
unsigned char secret[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
unsigned char id[7]={0};
unsigned char length=0;
LD_NFC myNFC(&Serial1,&Serial);
void setup()
{
        delay(3000);
	Serial.begin(9600);
	Serial1.begin(115200);
	myNFC.wakeUp();
}
int ctr=0;
void loop()
{ 
	myNFC.scanReadTargetID(0x00, id, &length);
	if(myNFC.passWordCheck(0x08,UID,secret)==1)
	{   
		Serial.println("passed");   
		if(ctr<4)
		{  
			myNFC.writeData(0x08,dataWriteIntoCard);
			Serial.println("written");
			ctr++;
		} 
		delay(2000);
		myNFC.readData(0x08);
	} 
        Serial.println();
        Serial.println();
        Serial.println();
	delay(4000);
}