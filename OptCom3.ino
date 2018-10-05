/* IR TSOP7000 series communication module
 * Push Button -> 'a'
 * IR send 
 * Wait receive serial in 10mS
 * LCD update
 */
#include <LiquidCrystal.h>

// Read data
int IRin = 2;
int IRout = 3;
int LEDout = 13;
int SWdn = 10;
int SWen = 11;
int SWup = 12;
int StartWidth = 480;
int StartJit = 30;
int Smin=StartWidth-StartJit;
int Smax= StartWidth+StartJit;
unsigned long time,base;
LiquidCrystal lcd(9,8,4,5,6,7);

//ID:08... .2233444
//Txp12dBm .Node:00
void LCDinit()
{
  lcd.clear();
  lcd.print("ID:**...OptComm.");
  lcd.setCursor(0,1);
  lcd.print("Txp00dBm.Node:00");
}
void setup()  {
    Serial.begin(19200);
  pinMode(IRin, INPUT_PULLUP);
  pinMode(SWdn, INPUT_PULLUP);
  pinMode(SWen, INPUT_PULLUP);
  pinMode(SWup, INPUT_PULLUP);
  
  pinMode(IRout, OUTPUT);
  pinMode(LEDout, OUTPUT);
  lcd.begin(16, 2);
  // Print a message to the LCD.
  LCDinit();
}

int bin2hex(int i)
{
  int c;
  i=i&0x0f;
  if(i>9) c = (i-10) + 'a';
  else c = i + '0';
  return c;  
}
int bin2LCD(int i)
{
  if(i>=0) {
    if(i<10) {
      lcd.write('0');
      lcd.write('0'+i);
      
    } else if(i<100) lcd.print(i);
    else lcd.print("OF");
  } else {
    lcd.print("**");
  }
}  
int IRrecv(int waittime)
{
  int c;
  int d;
  while(digitalRead(IRin)==LOW);
  while(digitalRead(IRin)==HIGH) {
    if(waittime==0) return -1;
    waittime--;
  }
  time=micros();
  while(digitalRead(IRin)==LOW);
  base=micros();
  time=base-time;
  if((time>Smin)&&(time<Smax)) {
    d=0;
    for(c=0; c<12; c++) {
      d=d>>1;
      while((micros()-base)<(c*52+20));
      if(digitalRead(IRin)==HIGH) {
        d|=0x800;
      }
    }
    if((d&0x0c03)==0x0401) {
      return (d>>2)&0xff;
    } else return -1;
  }
}
void IRsend(int cmd)
{
    Serial.write(0x00);
    Serial.write(cmd);
    Serial.write(0xff);
    Serial.flush();
}


// UID is the USB adapter ID
int UIDget()
{
  int i;
  IRsend('0');
  i=IRrecv(1000);
  return i;
}
// ID from the hostname in the WiFi-AP
// Meshy-AP08 -> 0x08がIRで帰ってくる。
void IDget()
{
  int i;
  IRsend('v');
  i=IRrecv(10000);
  lcd.setCursor(3,0);
  bin2LCD(i);
  delay(10);
}

void STget()
{
  int i;
  IRsend('n');
  i=IRrecv(10000);
  lcd.setCursor(14,1);
  bin2LCD(i);
  delay(10);
}
void PWchange(int diff)
{
  int i;
  if(diff>0)  IRsend('u');
  else if(diff==0) IRsend('s');
  else IRsend('d');
  i=IRrecv(30000);
  lcd.setCursor(3,1);
  bin2LCD(i);
}
int SWhistory=0;
int SWupc=0;
int SWdnc=0;
int SWenc=0;
int SWtime=2;
int SWget()
{
  int sw,i;
  sw=SWhistory;i=0;
  if(digitalRead(SWup)==LOW) {
    if(SWupc<SWtime) SWupc++;
    else sw|=1;
  } else {
    if(SWupc>0) SWupc--;
    else sw&=~1;
  }
  if(digitalRead(SWdn)==LOW) {
    if(SWdnc<SWtime) SWdnc++;
    else sw|=2;
  } else {
    if(SWdnc>0) SWdnc--;
    else sw&=~2;
  }
  if(digitalRead(SWen)==LOW) {
    if(SWenc<SWtime) SWenc++;
    else sw|=4;
  } else {
    if(SWenc>0) SWenc--;
    else sw&=~4;
  }
  i=(sw&0x0f)<<8;//raw status
  i|= ((~sw & SWhistory)&0x0f)<<4;// release status
  i|= ((sw & ~SWhistory)&0x0f);// push-on status
  SWhistory=sw;
  return i;
}
void loop() {
  int i,j;
  int f;
  unsigned long time,base;
  base = millis();
  f=0;
  i=0;
  while(1==1) {
    f=SWget();
    if     ((f&0x400)!=0)  IDget();
    else if((f&0x040)!=0)  {STget();PWchange(0);}
    else if((f&0x003)==3)  LCDinit();
    else if((f&0x001)!=0)  PWchange(1);
    else if((f&0x002)!=0)  PWchange(-1);
    time=millis();
    delay(10);
    if((time-base)>100) {
      f=UIDget();
      lcd.setCursor((i%8)+8,0);
      if(f>0) {
        if(f>9) lcd.write('*');
        else lcd.write('0'+f);
      } else lcd.write('.');
      i++;
      if(i>16) i=0;
      base=time;
    }
  }
}
