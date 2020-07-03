//Roger beeper and DF player voice confirmation Station/Club ID version 1.0
//Samuel Rebosura
//Note: First time loading the sketch to a new Arduino board needs to select beep 
//MP3 confirmation recording required:
//001- Controller Version / Welcome ID
//002-Access Granted
//003-Repeater is Active
//004-Repeater is OFF
//005-Beep Change
//006-Beep OFF
//007-Auxiliary ON
//008-Auxiliary OFF
//009-Steering UP
//010-Steering DOWN
//011-IDier ON
//012-IDier OFF
//013-passcode change
//014-passcode change confirm
//015-Club ID change
//016-Announcement is active
//017-Announcement is OFF
//018-Club ID timer set to 15 minutes
//019-Club ID timer set to 30 minutes
//020-Club ID timer set to 45 minutes
//021-Club ID timer set to 60 minutes
//Below are files for Beep and Club ID/Announcement
//022-alarm file
//023-Beep1 file
//024-Beep2 file
//025-Beep3 file
//026-Beep4 file
//027-Beep5 filee
//028-Beep6 file
//029-Beep7 file
//030-Beep8 file
//031-034 Station ID file

  void(* resetFunc) (void) = 0; //software reset vector
  #include <DFRobotDFPlayerMini.h>
  #include <SoftwareSerial.h>
  #include "Arduino.h" 
  #include <DTMF.h>
  #include <EEPROM.h>
  #define rxPin   6  // Serial input DF player
  #define txPin   7  // Serial output DF player
  SoftwareSerial mySoftwareSerial(6, 7); // RX, TX
  DFRobotDFPlayerMini myDFPlayer; //set Serial for DFPlayer-mini mp3 module

    const int rx_signal =  10;
    const int tx_control =  11;
    const int tone_out = 12;
    const int trigger_led =  13;
    const int aux_control = 8;
    const int mute_control = 5; //mute control pin
    const int busy_pin = 4;
    const int timer_select = 3;
    const int beep_select = 2;
    int rx_active = 1;
    int tone_signal = 0;
    int tone_ready = 0;
    int tx_hangtime = 50; //TX hangtime in milli secs
    long TimeLastCheck = 0;
    long trigger_timeout = 240;//repeater time out variable in secs
    long transmit_TOT = 30; //Transmit time out timer in seconds 
    long TimeoutTime = 0;
    long time_var = 0;
    long auto_id_timer = 0;
    long id_timer_check = 0;
    long club_id_time = 15; // Change here default auto Club ID time in minutes  
    int trigger_loop = 0;
    int dtmf_detected = 0;
    int tx_enable = 1;
    int EE_addr1 = 0; //tone select eeprom address
    int EE_addr2 = 2; //passcode eeprom address
    int EE_addr3 = 4; //aux status eeprom address
    int EEPROM_read_beep = 0;
    int admin_passcode = 0;
    int default_passcode = 2468; // set default passcode here
    int tone_enable = 1; //
    int club_ID_enable = 0; //club ID disable by default
    int announcement_enable = 0;
    int set_passcode_enable = 0;
    int club_id_no = 32; // default is mp3 file 0026
    int admin_status = 1; //Set to 1 to enable admin as default, 0 will require admin passcode
    int admin_enable = admin_status; 
    int voltageInput = 1; // voltage monitor
    float vout = 0.0;
    float vin = 0.0;
    float R1 = 101500.0; // actual resistance of R1 (100K)
    float R2 = 9995.0; // actual resistance of R2 (10K)
    int value = 0;
    int low_voltage = 0;
    int aux_status = 0;      
  int sensorPin = A0; //DTMF input
  int dtmf_led = 9;
  String s_hash = "#";
  char thischar;
  float n=128.0;
  // sampling rate in Hz
  float sampling_rate=8926.0;

// Instantiate the dtmf library with the number of samples to be taken
// and the sampling rate.
DTMF dtmf = DTMF(n,sampling_rate);
float d_mags[8];
String readString;
    
void setup() {
  delay(2000);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT); 
  mySoftwareSerial.begin (9600);
  Serial.begin(9600);
  pinMode(tx_control,OUTPUT);
  pinMode(trigger_led, OUTPUT);
  pinMode(rx_signal, INPUT_PULLUP);
  pinMode(dtmf_led, OUTPUT);
  pinMode(aux_control, OUTPUT);
  pinMode(mute_control, OUTPUT); //set mute control as output
  pinMode(busy_pin, INPUT);
  pinMode(timer_select, INPUT_PULLUP);
  pinMode(beep_select, INPUT_PULLUP);
  
  EEPROM.get(EE_addr3,aux_status); //read from aux status  in eeprom
  //if (aux_status != 0 && aux_status != 1){aux_status = 0;}//For initial start-up, assuming EEPROM data is FF hex.
  digitalWrite(aux_control,aux_status); //make sure all output pins are low after boot 
  digitalWrite(tx_control,LOW);
  digitalWrite(trigger_led,LOW);
  digitalWrite(mute_control,LOW); //set mute control initially to low at startup
  pinMode(voltageInput, INPUT);
  EEPROM.get(EE_addr1,EEPROM_read_beep); //read from beep option in eeprom
  if (EEPROM_read_beep > 5){EEPROM_read_beep = 1;}//For initial start-up, assuming EEPROM data is FF hex.
  tone_enable =  EEPROM_read_beep;
  EEPROM.get(EE_addr2,admin_passcode); //read from passcode saved in eeprom  
  //tone_enable = 1;
  //EEPROM.put(EE_addr1, tone_enable);
   Serial.println();
  Serial.println(F("DFPlayer"));
  Serial.println(F("Initializing"));
  delay(4000); // delay for DF module to load up
  myDFPlayer.begin(mySoftwareSerial);
  delay(4000); // delay for mcard reading

  //if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    
    //Serial.println(F("Unable to begin:"));
    //Serial.println(F("1.Re-check the connection!"));
    //Serial.println(F("2.Insert the SD card!"));
    //while(true);
    //tone_enable = 0;
    //club_ID_enable = 0; 
  //}
  Serial.println(F("DFPlayer ready"));
  myDFPlayer.volume(20);  //Set volume value. From 0 to 30
  Serial.println("Roger Beeper DF_v3.1");
  Serial.println("System Ready!");
   delay(2000);
   
  tx_on();
  mute_off();
  //if (club_ID_enable == 1 && tone_enable == 1){
  Serial.println("Playing welcome announcement!");
  playfile(1); 
 
  delay(500);
    
  //}
  tx_off();
  set_id_timer();
 
}

void loop() {
    
  dtmf_decode();
  //read_voltage(); //remove this line to disable low voltage check
  trigger_control();
  auto_ID_timer();
  button_check();
  
}

void trigger_control() {
  if (digitalRead(rx_signal) == 0 && tone_ready == 1)  {
    check_timeout();
    return;
  }
  if (digitalRead(rx_signal) == 0 && tx_enable == 1)  {
    mute_on(); //COS received, mute is ON
    tx_on();
    Serial.println("RX detected");
    tone_ready = 1;
    set_timeout();       
   }
  if (digitalRead(rx_signal) == 1 && tone_ready == 1)  {
    mute_off(); //COS off, mute signal off
    delay(500);
    if (tone_enable == 1) {//rotation beep 
    send_tone();
    }
    if (tone_enable == 0) {//beep off
    delay(600);
    tone_ready = 0;
    }
    if (tone_enable == 2) {//change tone number below if you want to re-order
    tone0();
    }
    if (tone_enable == 3) {
    tone1();
    }
    if (tone_enable == 4) {
    tone2();
    }
    if (tone_enable == 5) {
    tone3();
    }
     if (tone_enable == 6) {
    tone4();
    }
     if (tone_enable == 7) {
    tone5();
    }
     if (tone_enable == 8) {
    tone6();
    }
    if (tone_enable == 9) {
    tone7();
    }
    delay(100);
      if ( dtmf_detected == 1)  {
      delay(250);
      tx_on();
      dtmf_detected = 0;
      playfile(5);
      
      if (tone_enable == 2) {
      tone0();
      }
      if (tone_enable == 3) {
      tone1();
      }
      if (tone_enable == 4) {
      tone2();
      }
      if (tone_enable == 5) {
      tone3();
      }
      if (tone_enable == 6) {
      tone4();
      }
      if (tone_enable == 7) {
      tone5();
      }
      if (tone_enable == 8) {
      tone6();
      }
      if (tone_enable == 9) {
      tone7();
      }
      }
      
     if ( dtmf_detected == 2)  {
      delay(250);
      tx_on();
      dtmf_detected = 0;
      //beep_alert();
      if (tx_enable == 0) {
       playfile(4); //play repeater off
      }
      if (tx_enable == 1) {
       playfile(3); //play repeater is active
     }
     admin_enable = admin_status;
     delay(250);
     }
    tx_off();
    readString="";
    trigger_loop = trigger_loop+1;
    //Serial.println(trigger_loop);
    tone_signal = tone_signal+1;
    if (tone_signal == 8) {
      tone_signal = 0;
    }
   
    set_timeout();
   } 
  //clear DTMF String on RX stanby / TX is off 
  if (digitalRead(rx_signal) == 0 && tx_enable == 0)  {
    Serial.println("RX detected");
    Serial.println("TX disabled!");
     if ( dtmf_detected == 2)  {
      delay(250);
      if (tx_enable == 1) {
        tx_on();
        playfile(3);
        tx_off();
     }
     dtmf_detected == 0;
     tone_ready == 0;
     }     
  }
  if (digitalRead(rx_signal) == 1 && tone_ready == 0)  {
    return;        
  } 
}


void send_tone() {//this is the part where the multiple beep seaquence is ordered
 switch (tone_signal) {
      case 0:
        tone0();
        break;
      case 1:
        tone1();
        break;
      case 2:
        tone2();
        break;
      case 3:
        tone3();
        break;
      case 4:
        tone4();
        break;
      case 5:
        tone5();
        break;
      case 6:
        tone6();
        break;
      case 7:
        tone7();
        break;
 }
}

void tone0() {
      playfile(23);
      tone_ready = 0;
      Serial.println("Tone_0");    
}
void tone1() {
      playfile(24);
      tone_ready = 0;
      Serial.println("Tone_1");
}
void tone2() {
      playfile(25);
      tone_ready = 0;
      Serial.println("Tone_2");
}
void tone3() {
      playfile(26);
      tone_ready = 0;
      Serial.println("Tone_3");
}
void tone4() {
      playfile(27);
      tone_ready = 0;
      Serial.println("Tone_4");    
}
void tone5() {
      playfile(28);
      tone_ready = 0;
      Serial.println("Tone_5");
}
void tone6() {
      playfile(29);
      tone_ready = 0;
      Serial.println("Tone_6");
}
void tone7() {
      playfile(30);
      tone_ready = 0;
      Serial.println("Tone_7");
}
void tone8() {
      playfile(22);
      tone_ready = 0;
      Serial.println("Tone_8");
}

 void set_timeout() {
   
    TimeoutTime = trigger_timeout*1000L;
    TimeLastCheck = millis();
    Serial.println("Set_time_out");
             
  }

  void check_timeout() {
    TimeoutTime -= ((millis() - TimeLastCheck));
    TimeLastCheck = millis();
    //Serial.println(TimeoutTime);
    if (TimeoutTime <= 0) {
      delay(640);
      Serial.println("Timeout");
      tone8();
      tone8();
      tone8();
      tx_off();
      delay(transmit_TOT*1000L); //transmit time out time
      }
    }

  void dtmf_decode()  {

      /* while(1) */dtmf.sample(sensorPin);
 
  dtmf.detect(d_mags,475); // adjust from 450-512

  thischar = dtmf.button(d_mags,1800.);
  if(thischar) {
    Serial.println(thischar);
    
    char C = thischar;
    digitalWrite(dtmf_led,HIGH);
    delay(150);
    digitalWrite(dtmf_led,LOW);
    readString += C;
    if (readString == "#") {
      digitalWrite(tx_control,LOW);
      digitalWrite(trigger_led,LOW);
    }
  }
  
  if (readString.length() == 4 && set_passcode_enable == 1) {
    
    //Serial.println(readString);
    set_passcode();
    readString="";  
  }
  
  if (readString.length() == 5) {
    if (readString == "#6470") {//Beep Select 0-4
       
       tone_enable = 0;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone off");
       dtmf_detected = 1;
    }
    if (readString == "#6471") {
       
       tone_enable = 1;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       trigger_loop = 0;
       //Serial.println("tone on");
       dtmf_detected = 1;
    }
    if (readString == "#6472") {
       
       tone_enable = 2;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone4 on");
       dtmf_detected = 1;
    }
    if (readString == "#6473") {
      
       tone_enable = 3;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone2 on");
       dtmf_detected = 1;
    }
    if (readString == "#6474") {
       
       tone_enable = 4;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone1 on");
       dtmf_detected = 1;
    }
    if (readString == "#6475") {
       
       tone_enable = 5;
       EEPROM.put(EE_addr1, tone_enable);//store selected beep in EEPROM
       //Serial.println("tone7 on");
       dtmf_detected = 1;
    }
    if (readString == "#1590") { //Club ID is OFF
       tx_on();
       delay (1000);
       playfile(12);
       tx_off();
       club_ID_enable = 0;   
    }
    if (readString == "#1591") { //Club ID is ON
       tx_on();
       delay (1000);
       playfile(11);
       tx_off();
       club_ID_enable = 1;   
    }
    if (readString == "#1592") { //Club ID select 1
       tx_on();
       delay (1000);
       playfile(15);
       tx_off();
       club_id_no = 26; //set Club ID file no   
    }
    if (readString == "#1593") { //Club ID select 2
       tx_on();
       delay (1000);
       playfile(15);
       tx_off();
       club_id_no = 27; //set Club ID file no  
    }
    if (readString == "#1594") { //Club ID select 3
       tx_on();
       delay (1000);
       playfile(15);
       tx_off();
       club_id_no = 28; //set Club ID file no  
    }
    if (readString == "#1595") { //force club ID
       tx_on();
       delay (1000);
       playfile(club_id_no); //play current selected ID
       tx_off();
      
    }
    if (readString == "#6715") { //Club ID time set to 15mins
       tx_on();
       delay (1000);
       playfile(18);
       tx_off();
       club_id_time = 15;         
    }
    if (readString == "#6730") { //Club ID time set to 30mins
       tx_on();
       delay (1000);
       playfile(19);
       tx_off();
       club_id_time = 30;         
    }
    if (readString == "#6745") { //Club ID time set to 45mins
       tx_on();
       delay (1000);
       playfile(20);
       tx_off();
       club_id_time = 45;         
    }
     if (readString == "#6760") { //Club ID time set to 60mins
       tx_on();
       delay (1000);
       playfile(21);
       tx_off();
       club_id_time = 60;         
    }
    if (readString == "#9430") {//Club announcement disable
       tx_on();
       delay (1000);
       announcement_enable = 0;
       playfile(17);
       tx_off();
    }  
    if (readString == "#9431") {//Club announcement enable
       tx_on();
       delay (1000);
       announcement_enable = 1;
       playfile(16);
       tx_off();
    }
     if (readString == "#9432") {//Club announcement enable
       tx_on();
       delay (1000);
       playfile(29);
       tx_off();
    }
    
      
    if (readString == "#7530" && admin_enable == 1) { // repeater off
      
       tx_enable = 0;
       //Serial.println("tx off");
       dtmf_detected = 2;
    }
    if (readString == "#7531" && admin_enable == 1) {
      
       tx_enable = 1;
       //Serial.println("tx on");
       dtmf_detected = 2;
    }
    
    
    if (readString == "#8520" && admin_enable == 1) {//auxiliary off
       
       tx_on();
       digitalWrite(aux_control,LOW);
       aux_status = 0;
       EEPROM.put(EE_addr3, aux_status);//store aux status in EEPROM
       delay (1000);
       playfile(8);
       tx_off();
       admin_enable = admin_status;
    }
    if (readString == "#8521" && admin_enable == 1) {//auxiliary on
       
       tx_on();
       digitalWrite(aux_control,HIGH);
       aux_status = 1;
       EEPROM.put(EE_addr3, aux_status);//store aux status in EEPROM
       delay (1000);
       playfile(7);
       tx_off();
       admin_enable = admin_status;
    }
    
    if (readString == "#1179") {//reboot and set passcode to default by dtmf
      EEPROM.put(EE_addr2,default_passcode);//Set passcode to default in EEPROM
      tone_enable = 1; //change to set default beep
      EEPROM.put(EE_addr1, tone_enable);//set default beep option
      tx_enable = 1;
      tx_on();
       //Serial.println("reboot");
       delay(640);
       playfile(2);
      tx_off();
      resetFunc();
    }
    
    if (readString == s_hash + admin_passcode) { // Admin passcode
      admin_enable = 1; 
      tx_on();
      playfile(2);
      tx_off();
    }
    if (readString == "#3281" && admin_enable == 1) { // Change Admin passcode
      set_passcode_enable = 1;
      tx_on();
      playfile(13);
      tx_off();
    }
    
  Serial.println(readString);

  readString="";
  }
  //Serial.println("DTMF listening ...."); 
    }
 
void tx_on(){//trigger control
   //Serial.println("TX ON");
  digitalWrite(tx_control,HIGH);
  digitalWrite(trigger_led,HIGH); 
}
void tx_off(){
  //if (low_voltage == 1){
    //  tone(tone_out, 220, 250);
    //  delay(250);
    //  noTone(tone_out);
     // delay(250);
  //}
  delay(tx_hangtime);//hang time 
   //Serial.println("TX OFF");
  digitalWrite(tx_control,LOW);
  digitalWrite(trigger_led,LOW);
}

void set_passcode(){
  String new_passcode = readString;
  admin_passcode = new_passcode.toInt();
  EEPROM.put(EE_addr2, admin_passcode);//store new passcode in EEPROM
  set_passcode_enable = 0;
  delay (1000);  
  tx_on();
  delay (640);
  playfile(14);
  tx_off(); 
  admin_enable = admin_status;
}


void read_voltage(){ // read the value at analog input
    if (tx_enable == 0){
      return;
    }
       float min_voltage = 9.0; //change here to set the low voltage limit
       value = analogRead(voltageInput);
       vout = ((value) * 5.0) / 1024.0; 
       vin = vout / (R2/(R1+R2)); 
       if (vin < min_voltage){
       low_voltage = 1;
       }
       else if (vin > min_voltage){
       low_voltage = 0;
       }
       //Serial.println(vin);
}

void beep_alert(){
  delay(640);
      tone8();
      tone8();
      tone8();
      
}

void mute_on(){
  digitalWrite(mute_control,HIGH);
}

void mute_off(){
  digitalWrite(mute_control,LOW);
}

void check_busy(){//will just loop into checking the busy pin every 200msec and exit
   //Serial.println("Check Busy");
  delay(100);
  while (digitalRead (busy_pin) == 0){
    delay(200); 
  }
  delay (100);
}

void set_id_timer() {
   
    auto_id_timer = club_id_time*60000L;
    id_timer_check = millis();             
  }


void auto_ID_timer() {
    auto_id_timer -= ((millis() - id_timer_check));
    //Serial.println(auto_id_timer/1000);
    id_timer_check = millis();
    if (auto_id_timer <= 0) {
      if (digitalRead(rx_signal) == 0) {//if COS is active postponed for 2 minutes
       auto_id_timer = 120000L; 
       return;
       }
      if (club_ID_enable == 1) {
       tx_on(); 
       playfile(club_id_no); //play club ID
       check_announcement();
       set_id_timer();
       tx_off();
      }
    }
   }

void check_announcement(){
  if (announcement_enable == 1){       
       delay(1000);
       playfile(29); //Play Anouncement file when enabled by DTMF code
       tx_off();
  }
}

void playfile(int file_num){ //function to play any file
  mute_off();
  //Serial.println("Playing file");
  myDFPlayer.play(file_num); //Play file no. in sd card
  check_busy();
}

void button_check(){
  if (digitalRead (beep_select) == 0 && digitalRead(rx_signal) == 1){
    delay (250);
    tx_on();
    delay(500);
   tone_enable = tone_enable+1;
    if (tone_enable == 10) {
      tone_enable = 0;
    }
    
    if (tone_enable == 0){playfile(6);}
    else {playfile(5);}
    delay(500);
    EEPROM.put(EE_addr1, tone_enable);
    if (tone_enable == 2) {
    tone0();
    }
    if (tone_enable == 3) {
    tone1();
    }
    if (tone_enable == 4) {
    tone2();
    }
    if (tone_enable == 5) {
    tone3();
    }
     if (tone_enable == 6) {
    tone4();
    }
     if (tone_enable == 7) {
    tone5();
    }
     if (tone_enable == 8) {
    tone6();
    }
    if (tone_enable == 9) {
    tone7();
    }
   
    tx_off();
  }

  if (digitalRead (timer_select) == 0 && digitalRead(rx_signal) == 1){
    delay (250);
    tx_on();
    delay(500);
   club_id_time = club_id_time+15;      
    if (club_id_time == 75) {
      club_id_time = 15;
    }
    
    //if (club_id_time == 0){club_ID_enable = 0;playfile(12);}
    if (club_id_time == 15){playfile(18);}
    if (club_id_time == 30){playfile(19);}
    if (club_id_time == 45){playfile(20);}
    if (club_id_time == 60){playfile(21);}
    delay(500);
    tx_off();
  }
  if (digitalRead (timer_select) == 0 && tone_ready == 1){
    delay (500);
    tx_on();
    delay(500);
   club_id_no = club_id_no+1;      
    
    if (club_id_no == 31){club_ID_enable = 1;playfile(31);}
    if (club_id_no == 32){club_ID_enable = 1;playfile(32);}
    if (club_id_no == 33){club_ID_enable = 1;playfile(33);}
    if (club_id_no == 34){club_ID_enable = 1;playfile(34);}
    if (club_id_no == 35) {
      playfile(12);
      club_id_no = 30;
      club_ID_enable = 0;
      }
    
    delay(500);
    tx_off();
  }

  if (digitalRead (beep_select) == 0 && tone_ready == 1){
    delay (500);
    tx_on();
    delay(2000);
    playfile(club_id_no); //play club ID
    delay(500);
    tx_off();
  }
}
