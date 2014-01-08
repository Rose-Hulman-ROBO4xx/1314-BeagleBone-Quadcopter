#define sda_out 5
#define scl_out 10
#define sda_in 3
#define scl_in 8
#define PERIOD 0

#define clear_scl digitalWrite(scl_out, HIGH)
#define set_scl digitalWrite(scl_out, LOW)
#define write_scl(level) (level ? set_scl : clear_scl)

#define clear_sda digitalWrite(sda_out, HIGH)
#define set_sda digitalWrite(sda_out, LOW)
#define write_sda(level) (level ? set_sda : clear_sda)

#define release_scl set_scl
#define release_sda set_sda

#define read_scl digitalRead(scl_in)
#define read_sda digitalRead(sda_in)
#define delay_ delay(PERIOD/3)

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(sda_out, OUTPUT);
  pinMode(scl_out, OUTPUT);
  pinMode(scl_in, INPUT);
  pinMode(sda_in, INPUT);
  write_scl(1);
  write_sda(1);
  wait_for_user();
  Serial.println("Starting...");
  send_write(0xD0, 0x6B, 0);
  
}
void wait_for_user(){
  while(Serial.read()==-1){}
}

void loop()
{
  // put your main code here, to run repeatedly:
  //wait_for_user();
  long average = 0;
  int i;
  for (i = 0; i < 8; i++){
    int data = 0;
    data |= send_read(0xD0, 0x3B);
    data = data << 8;
    data |= send_read(0xD0, 0x3C);
    average += data;
  }
  average = average / 8;
  Serial.println(average, DEC);
  //delay(10);
  
}
void send_start(){
  write_scl(1);
  write_sda(1);
  delay_;
  write_sda(0);
  delay_;
  
}
void send_write(byte address, byte reg, byte data){
  
  address &= 0xFE;
  int i;
  send_start();
  for (i = 0; i < 8; i ++){
    write_scl(0);
    delay_;
    write_sda(address & 0x80);
    address = address << 1;
    delay_;
    write_scl(1);
    delay_;
  }
  
  write_scl(0); //clocking ack bit
  write_sda(1); //gotta let the slave ack
  delay_;
  write_scl(1); //clock ack bit
  
  while(read_sda){} //wait for the slave to ack
  //Serial.println("got ack");
  write_scl(0); //slave should release the line when the clock goes low.
  while(!read_sda){}
  //Serial.println("released line like expected!");
  delay_;
  
  //write the register address
  for (i = 0; i < 8; i ++){
    write_scl(0);
    delay_;
    write_sda(reg & 0x80);
    reg = reg << 1;
    delay_;
    write_scl(1);
    delay_;
  }
  
  write_scl(0);
  delay_;
  write_sda(0); //master needs to ack
  delay_;
  write_scl(0); //make a clock pulse
  delay_;
  write_scl(1);
  delay_;
  write_scl(0);
  delay_;
  write_sda(1); //release the sda
  delay_;
  
  //now we can write the actual data!
  
  for (i = 0; i < 8; i++){
    write_scl(0);
    delay_;
    write_sda(data & 0x80);
    data = data << 1;
    delay_;
    write_scl(1);
    delay_;
  }
  write_scl(0);
  delay_;
  write_sda(0); //master needs to ack
  delay_;
  write_scl(1);
  delay_;
  write_scl(0);
  delay_;
  write_scl(1);
  delay_;
  write_sda(1);
  delay_;
  //Serial.println("finished writing");
  
  
}
byte send_read(byte address, byte reg){
  byte address_saved = address;
  address &= 0xFE;
  int i;
  send_start();
  for (i = 0; i < 8; i ++){
    write_scl(0);
    delay_;
    write_sda(address & 0x80);
    address = address << 1;
    delay_;
    write_scl(1);
    delay_;
  }
  
  write_scl(0); //clocking ack bit
  write_sda(1); //gotta let the slave ack
  delay_;
  write_scl(1); //clock ack bit
  while(read_sda){} //wait for the slave to ack
  //Serial.println("got ack");
  write_scl(0); //slave should release the line when the clock goes low.
  while(!read_sda){}
  //Serial.println("released line like expected!");
  delay_;
  
  //write the register address
  for (i = 0; i < 8; i ++){
    write_scl(0);
    delay_;
    write_sda(reg & 0x80);
    reg = reg << 1;
    delay_;
    write_scl(1);
    delay_;
  }
  
  write_scl(0);
  delay_;
  write_sda(0); //master needs to ack
  delay_;
  write_scl(0); //make a clock pulse
  delay_;
  write_scl(1);
  delay_;
  write_scl(0);
  delay_;
  write_sda(1); //release the sda
  delay_;
  write_scl(1);
  delay_;
  //now send another start code
  send_start();
  //now send the address again!
  address = address_saved | 0x01;
  for (i = 0; i < 8; i ++){
    write_scl(0);
    delay_;
    write_sda(address & 0x80);
    address = address << 1;
    delay_;
    write_scl(1);
    delay_;
  }
  
  write_scl(0); //clocking ack bit
  write_sda(1); //gotta let the slave ack
  delay_;
  write_scl(1); //clock ack bit
  while(read_sda){} //wait for the slave to ack
  //Serial.println("got ack");
  write_scl(0); //slave should release the line when the clock goes low.
  //Serial.println("released line like expected!");
  delay_;
  
  //now we can read the actual data!
  byte data = 0;
  for (i = 0; i < 8; i++){
    write_scl(0);
    delay_;
    data = data << 1;
    data |= read_sda;
    delay_;
    write_scl(1);
    delay_;
  }
  write_scl(0);
  delay_;
  write_sda(1); //master needs to ack
  delay_;
  write_scl(1);
  delay_;
  write_scl(0);
  delay_;
  write_sda(0);
  delay_;
  write_scl(1);
  delay_;
  write_sda(1);
  delay_;
  //Serial.println("finished reading");
  
  return data;
  
  
}

