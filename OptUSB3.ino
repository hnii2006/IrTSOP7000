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
  Serial.begin(9600);

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

int receive()
{
  int c,d;
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
      while((micros()-base)<(c*Tick+20));
      if(digitalRead(IRin)==HIGH) {
        d|=0x800;
      }
    }
    if(digitalRead(LEDout)==LOW) digitalWrite(LEDout,HIGH);
    else digitalWrite(LEDout, LOW);
    if((d&0x0c03)==0x0401) {
      return (d>>2)&0xff;
    }
    return -1;
  }
  return -2;  
}
int exec_cmd(int cmd)
{
  int c;
  Serial.write(cmd);
  Serial.flush();
  c=100;
  delay(2);
  while((Serial.available()<2)&&(c>0)) {
    delay(2);
    c--;
  }
  if(Serial.available()>=2) {
    while(Serial.available()>2) Serial.read();
    c=Serial.read()-'0';
    c=c*10+Serial.read()-'0';
    return c;
  }
  return -1;
}

void loop() {
  int c,d;
  c=receive();
  switch(c) {
    case -1:
      d=-1;
      break;
    case -2:
      d=-1;
      break;
    case '0':
      d=7;
      delay(2);
      break;
    default:
      d=exec_cmd(c);
      break;
  }
  if(d>=0) IRsend(0x00);IRsend(d);IRsend(0xff);
}
