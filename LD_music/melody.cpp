#include "melody.h"
MELODY::MELODY(int tonepin)
{
	pinMode(tonepin,OUTPUT);
	this->pin=tonepin;
	this->debug=false;
}

MELODY::MELODY(int tonepin,Stream *serial)
{
	pinMode(tonepin,OUTPUT);
	this->pin=tonepin;
	this->serial=serial;
	this->debug=true;
}


//playSpeed 为一个节拍的时间 单位是ms
void MELODY::playMelody(char *Melody,int playSpeed){
	const char *d = " ,";
	char *p;
	char cgy[10];
	int noteDuration=0;
	int i,j;
	uint8_t thisNote1=0,thisNote2=0;
	p = strtok(Melody,d);
	sprintf(cgy, "%s", p);
	while(p)
	{
		char note[]="0000000000";
		noteDuration=0;
		for (i=0;*(p+i)!='\0';i++){
			note[i]=*(p+i);
		}
		for (int j=4;j<i;j++){
			int time;
			switch(note[j]){
				case '1':time=1;
				break;
				case '2':time=2;
				break;
				case '3':time=4;
				break;
				case '4':time=8;
				break;
				case '5':time=16;
				break;
				case '6':time=32;
				break;
				
				case '7':time=6;
				break;
			}
			noteDuration += playSpeed/(time);		
		}
		if (this->debug)
			this->serial->println(noteDuration);
		if (note[0]=='n'){
			switch (note[1]){
				case '1':thisNote1=0;
				break;
				case '2':thisNote1=2;
				break;
				case '3':thisNote1=4;
				break;
				case '4':thisNote1=5;
				break;
				case '5':thisNote1=7;
				break;
				case '6':thisNote1=9;
				break;
				case '7':thisNote1=11;
				break;
			}
		}
		else if (note[0]=='s'){
			switch (note[1]){
				case '1':thisNote1=1;
				break;
				case '2':thisNote1=3;
				break;
				case '4':thisNote1=6;
				break;
				case '5':thisNote1=8;
				break;
				case '6':thisNote1=10;
				break;
			}
		}
		
		if (note[3]=='f'){
			switch (note[2]){
				case '0':thisNote2=4;
				break;
				case '1':thisNote2=3;
				break;
				case '2':thisNote2=2;
				break;
				case '3':thisNote2=1;
				break;
				case '4':thisNote2=0;
				break;
			}
		}
		else if (note[3]=='s'){
			switch (note[2]){
				case '1':thisNote2=5;
				break;
				case '2':thisNote2=6;
				break;
				case '3':thisNote2=7;
				break;
				case '4':thisNote2=8;
				break;
			}
		}
		if (note[1]=='0'){
			thisNote2=0;
			thisNote1=0;
		}
		tone(this->pin, notefr[thisNote2][thisNote1],noteDuration);
			
		int pauseBetweenNotes = noteDuration*1.1;
		delay(pauseBetweenNotes);
		
		noTone(this->pin);
		if (this->debug)
			this->serial->println(cgy);  // 印到串口
		p=strtok(NULL,d);
		sprintf(cgy, "%s", p);
	}
}

