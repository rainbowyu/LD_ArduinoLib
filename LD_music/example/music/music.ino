#include "melody.h"
//天空之城
char melody[]= "n61f4,n71f4,n10f34,n71f4,n10f3,n30f3,n71f333,n31f3,n61f34,n51f4,n61f3,n10f3,n51f333,n31f3,n41f34,n31f4,n41f4,n10f43,\
                n31f333,n10f3,n71f34,s41f4,s41f3,n71f3,n71f33,n00f3,n61f4,n71f4,n10f34,n71f4,n10f3,n10f3,n71f333,n31f4,n31f4\
                n61f34,n51f4,n61f3,n10f3,n51f333,n31f3,n41f3,n10f4,n71f43,n10f3,n20f3,n30f4,n10f33,n00f4,n10f4,n71f4,n61f3,n71f3,s51f3\
                n61f33,n00f3,n10f4,n20f4,n30f34,n20f7,n30f7,n40f7,n50f4,n20f333,n51f3,n20f5,n10f5,n71f5,n10f534,n20f3,n30f4\
                n30f333,n61f4,n71f4,n10f3,n71f4,n10f4,n20f3,n10f34,n51f4,n51f33,n40f3,n30f3,n20f3,n10f3,n30f333,n61f4,n71f4,n10f34,n71f4,n10f3,n30f3,\
                n71f333,n31f3,n61f34,n51f4,n61f3,n10f3,n51f333,n31f3,n41f3,n10f4,n71f43,n10f3,n20f3,n30f4,n10f433,\
                n10f4,n71f4,n61f3,n71f3,s51f3,n61f333";
//no debug                
MELODY myMelody(8);
//debug
//MELODY myMelody(8,&Serial);
void setup() {
  //no debug
  //Serial.begin(115200);
  myMelody.playMelody(melody,2000);
}

// the loop function runs over and over again forever
void loop() {
         
}