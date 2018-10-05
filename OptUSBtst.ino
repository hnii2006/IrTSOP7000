// Read data
int IRin = 2;
int IRout = 3;
int LEDout = 13;
int StartWidth = 480;
int StartJit = 30;
int Smin=StartWidth-StartJit;
int Smax= StartWidth+StartJit;

int Tick=52;

unsigned long time,base;
void setup()  {
  pinMode(IRin, INPUT);
  pinMode(IRout, OUTPUT);
  pinMode(LEDout, OUTPUT);
  digitalWrite(IRout, HIGH);
  Serial.begin(19200);

}
void IRsend(int data)
{
  int i;
  unsigned long base, t;
  data = data |0xff00;
  base = micros();
  digitalWrite(IRout, LOW);
  t=base+Tick;
  for(i=0; i<9; i++) {
    while(t>micros());
    if((data&0x01)==0) digitalWrite(IRout, LOW);
    else digitalWrite(IRout, HIGH);
    data = data>>1;
    t=base+(i+2)*Tick;
  }
  while(t>micros());
}
int bin2hex(int i)
{
  int c;
  i=i&0x0f;
  if(i>9) c = (i-10) + 'a';
  else c = i + '0';
  return c;  
}
int p=3;
void loop() {
  int c;
  int d;
  while(digitalRead(IRin)==LOW);
  while(digitalRead(IRin)==HIGH);
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
    if(digitalRead(LEDout)==LOW) digitalWrite(LEDout,HIGH);
    else digitalWrite(LEDout, LOW);
    delay(2);
    if((d&0x0c03)==0x0401) {
      d=(d>>2)&0xff;
      switch(d) {
       case '0':
         d=2;
         break;
       case '1':
         d=3;
         delay(20);
         break;
       case '2':
         if(p<18) p+=3;
         d=p;
         delay(50);
         break;
       case '3':
         if(p>3) p-=3;
         d=p;
         delay(50);
         break;
//       default:
       //
      }

      Serial.write(0x00);
      Serial.write(d);
      Serial.write(0xff);
//      noInterrupts();
      IRsend(0x00);IRsend(d);IRsend(0xff);
//      interrupts();
      Serial.flush();
    }
  }
}
