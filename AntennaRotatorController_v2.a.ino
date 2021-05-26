
//Antenna Rotator Controller Project 11/25/2020
//Samuel Rebosura

void(* resetFunc) (void) = 0; //software reset vector
#include <EEPROM.h> 
//#include <LiquidCrystal.h>
//LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

const int clockwise_button = 2;     // Button to move motor clockwise
const int counterclockwise_button = 3;     // Button to move motor counter clockwise
const int set_button = 4;
const int clockwise_relay = 5;
const int counterclockwise_relay = 6;
const int run_relay =  7;
const int piezoPin = 8;

char *locations[] = {"DUMAGUETE", "CEBU  CITY"," BACOLOD  ", " ILO-ILO  ", "UBAY BOHOL", "GUINDULMAN", " TACLOBAN ", " SURIGAO  ", "CAGAYAN DO", " PAGADIAN "};
int positions[10]={240, 1, 315, 310, 50, 80, 32, 90, 148, 191};

int EE_addr1 = 1;
int EE_addr2 = 4;
int EE_addr3 = 8;

int clockwise_flag = 0;
int counterclockwise_flag = 0;
int analogValue = 0;
double analog_position = 0;
double motor_speed = 60.0;
double motor_speed2 = 60.0;
double motor_position_var = 0;
double motor_position = 0;
double motor_runtime_var = 0;
double motor_runtime = 0;
double motor_timer_check = 0;
double TimeLastCheck = 0;
long debounceDelay = 300;
long longdebounceDelay = 1500;
int lastButtonPushed = 1;
int run_mode = 0;
double motor_startposition = 0;
double motor_timelaps = 0;

long lastClockwiseDebounceTime = 0;  
long lastCounterclockwiseDebounceTime = 0;  
long lastSetDebounceTime = 0;
long lastLongSetDebounceTime = 0;

int lastClockwiseButtonState = HIGH;  
int lastCounterclockwiseButtonState = HIGH;   
int lastSetButtonState = HIGH;
int lastLongSetButtonState = HIGH;


int menu_display = 1;//0-default display 
int last_menu=0; //last displayed menu flag


void setup()  {
       Serial.begin(9600);
       pinMode(clockwise_button, INPUT_PULLUP);
       pinMode(counterclockwise_button, INPUT_PULLUP);
       pinMode(set_button, INPUT_PULLUP);
       
       pinMode(clockwise_relay, OUTPUT);
       pinMode(counterclockwise_relay, OUTPUT);       
       pinMode(run_relay, OUTPUT);
       pinMode(piezoPin, OUTPUT);
       
       digitalWrite(clockwise_relay, HIGH); //make sure soft reset will turn off the relays
       digitalWrite(counterclockwise_relay, HIGH);
       digitalWrite(run_relay, HIGH);
       //lcd.begin(20, 4);
       lcd.init();                  
       lcd.init();
       lcd.backlight();
       delay(1000);
       if(digitalRead(set_button)== LOW){
        motor_position = 0.1;
        EEPROM.put(EE_addr1, motor_position); 
        lcd.setCursor(0, 0);
        lcd.print("ZERO POSITION...");
        delay(2000);
       }
       if(digitalRead(clockwise_button)== LOW){
       lastButtonPushed=8;
       EEPROM.get(EE_addr2,motor_speed);
       EEPROM.get(EE_addr3,motor_speed2);
       set_motor_speed();
       }
       scroll("ROTATOR",10,1);
       lcd.setCursor(0, 1);
       scroll("CONTROLLER V2.a",10,1);
       delay(2000);
       //motor_position = 45;
      //EEPROM.put(EE_addr1, motor_position); 
      tone( piezoPin, 1500, 250);
      update_motor_position();
}

void read_inputs() {
 // read the state of the switches/sensors into a local variable:
  int reading;
  int Clockwise_buttonState=HIGH;             
  int Counterclockwise_buttonState=HIGH;             
  int Set_buttonState=HIGH; 
  int LongSet_buttonState=HIGH;    
 
  
  //clockwise button              
  reading = digitalRead(clockwise_button);
  if (reading != lastClockwiseButtonState) {
   lastClockwiseDebounceTime = millis();
     }                
      if ((millis() - lastClockwiseDebounceTime) > debounceDelay) {
        Clockwise_buttonState = reading;
        lastClockwiseDebounceTime=millis();
        }                               
        lastClockwiseButtonState = reading; 
        
 //counter clockwise button              
  reading = digitalRead(counterclockwise_button);
  if (reading != lastCounterclockwiseButtonState) {
   lastCounterclockwiseDebounceTime = millis();
     }                
      if ((millis() - lastCounterclockwiseDebounceTime) > debounceDelay) {
        Counterclockwise_buttonState = reading;
        lastCounterclockwiseDebounceTime=millis();
        }                               
        lastCounterclockwiseButtonState = reading; 

 //set button              
  reading = digitalRead(set_button);
  if (reading != lastSetButtonState) {
   lastSetDebounceTime = millis();
     }                
      if ((millis() - lastSetDebounceTime) > debounceDelay) {
       Set_buttonState = reading;
        lastSetDebounceTime=millis();
        }                               
        lastSetButtonState = reading;
//long_set button
  reading = digitalRead(set_button);
  if (reading != lastLongSetButtonState) {
   lastLongSetDebounceTime = millis();
     }                
      if ((millis() - lastLongSetDebounceTime) > longdebounceDelay) {
       LongSet_buttonState = reading;
        lastLongSetDebounceTime=millis();
        }                               
        lastLongSetButtonState = reading;

//main menu actions

if(Set_buttonState==LOW && (menu_display==2  || menu_display==3)){
  tone( piezoPin, 2000, 250);
  lastButtonPushed=1;
 }else if(Clockwise_buttonState==LOW && menu_display==1){
 tone( piezoPin, 2100, 250);
  lastButtonPushed=5;
 }else if(Counterclockwise_buttonState==LOW && menu_display==1){
  tone( piezoPin, 2100, 250);
  lastButtonPushed=7;
 //}else if(Counterclockwise_buttonState==LOW && (menu_display==1 || menu_display==2  || menu_display==3)){
  //tone( piezoPin, 2100, 250);
  //lastButtonPushed=1;
 }else if(Set_buttonState==LOW && menu_display==2){
  tone( piezoPin, 2000, 250);
  lastButtonPushed=4;
 }else if(Set_buttonState==LOW && menu_display==3){
  tone( piezoPin, 2000, 250);
  lastButtonPushed=5;
 }else if(Set_buttonState==LOW && menu_display==1){
  tone( piezoPin, 2000, 250);
  lastButtonPushed=4;
   }else if(LongSet_buttonState==LOW && menu_display==1){
  tone( piezoPin, 2000, 250);
  lastButtonPushed=6;
  }else if(Clockwise_buttonState==LOW && menu_display==7){
 tone( piezoPin, 2100, 250);
  lastButtonPushed=2;
 }else if(Counterclockwise_buttonState==LOW && menu_display==7){
  tone( piezoPin, 2100, 250);
  lastButtonPushed=3;
 }else if(Set_buttonState==LOW && menu_display==4 ){
  tone( piezoPin, 2000, 250);
  lastButtonPushed=9;
  }else if(Set_buttonState==LOW && menu_display==5 ){
  tone( piezoPin, 2000, 250);
  lastButtonPushed=9;
}
}

void update_motor_position(){//update motor position

  EEPROM.get(EE_addr1,motor_position);
  EEPROM.get(EE_addr2,motor_speed);
  EEPROM.get(EE_addr3,motor_speed2);
  //motor_position = 0;
  //lcd.clear();
  //lcd.setCursor(0, 0);
  //lcd.print(" Rotator Controller ");
  //lcd.setCursor(0, 1);
  //lcd.print("                    ");
  lcd.setCursor(0, 0);
  lcd.print(" AZIMUTH :      ");
  lcd.setCursor(0, 0);
  lcd.print(" AZIMUTH : ");
  lcd.print(motor_position,1);
  lcd.setCursor(0, 1);
  lcd.print("<<--  -->>   SET");
    
}

void main_menu() {
  
   switch (lastButtonPushed) {
    case 1:
      //update_motor_position();
      menu_display = 1;
      last_menu = menu_display;
      break;
    
    case 2:
      
      lcd.setCursor(0, 0);
      lcd.print(" RUN CLOCKWISE >");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(" AZIMUTH : ");
      lcd.print(motor_position,1);
      
      menu_display = 2;
      last_menu = menu_display;
      delay(1000);
      get_position();
      run_mode = 2;
      run_clockwise();
      
      break;
      
    case 3:
      
      lcd.setCursor(0, 0);
      lcd.print(" RUN CNTRCLOCK <");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(" AZIMUTH : ");
      lcd.print(motor_position,1);
      
      menu_display = 3;
      last_menu = menu_display;
      delay(1000);
      get_position();
      run_mode = 2;
      run_counterclockwise();
      
      break;

      case 4:
      
      lcd.setCursor(0, 0);
      lcd.print("<<MANUAL  MODE>>");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(" AZIMUTH : ");
      lcd.print(motor_position,1);
      
      menu_display = 4;
      last_menu = menu_display;
      delay(500);
      manual_mode();
      
      break;

    case 5:

      lcd.setCursor(0, 0);
      lcd.print("PRESET POSITIONS");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(" AZIMUTH : ");
      lcd.print(motor_position,1);
      menu_display = 5;
      last_menu = menu_display;
      delay(500);
      saved_positions();
      break;
    
    case 6:
      
      lcd.setCursor(0, 0);
      lcd.print("CALIBRATION MODE");
      motor_position = 0.1;
      EEPROM.put(EE_addr1, motor_position);
      lcd.setCursor(0, 1);
      lcd.print("                "); 
      lcd.setCursor(0, 1);
      lcd.print(" AZIMUTH : ");
      lcd.print(motor_position,1);
      
      menu_display = 6;
      last_menu = menu_display;
      delay(3000);
      lastButtonPushed = 1;      
      break;

      case 7:
      //update_motor_position();
      lcd.setCursor(0, 1);
      lcd.print("CkWise<>CtrClock");
      menu_display = 7;
      last_menu = menu_display;
      break;
   }
      
  }  



void set_position() {
    //Serial.println("Setting position");
    //motor_runtime = (motor_runtime_var/(360/motor_speed))*1000;
    TimeLastCheck = millis();
    motor_timelaps = millis();
    motor_startposition = motor_position;
    motor_position_var = 0;
    //lcd.setCursor(0, 3);
    //lcd.print(" << Motor running >>");
    //tone( piezoPin, 1750, 150);
  }
    
void manage_position() {
  
 if (run_mode == 1){

   if (clockwise_flag == 1) {
    //motor_position_var = (((motor_runtime_var/6)*1000) - motor_runtime);
    motor_position_var += ((millis() - motor_timelaps));
    motor_timelaps = millis();

    //Serial.println(motor_position_var);
    motor_position = (motor_startposition + ((motor_position_var/1000)*(360/motor_speed))); 
  
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0,0);
    lcd.print(" AZIMUTH :      ");
    lcd.setCursor(0, 0);
    lcd.print(" AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 1);
    lcd.print("PUSH SET to STOP");
   
    }
    
    if (counterclockwise_flag == 1) {
    motor_position_var += ((millis() - motor_timelaps));
    motor_timelaps = millis();
    //Serial.println(motor_position_var);
    motor_position = (motor_startposition - ((motor_position_var/1000)*(360/motor_speed)));
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0,0);
    lcd.print(" AZIMUTH :      ");
    lcd.setCursor(0, 0);
    lcd.print(" AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 1);
    lcd.print("PUSH SET to STOP");
   
    }
    //TimeLastCheck = millis(); 
    //lcd.setCursor(0, 1);
    //lcd.print("SYSTEM RUNNING .....");
    //if (motor_position <= 1) {
      //stop_motor();
      //return;  
    //}
    //if (motor_position >= 359) {
      //stop_motor();
      //return;  
    //}
    if (motor_position >= 360 && clockwise_flag ==1) {
      stop_motor();
      return; 
    }
    if (motor_position <= 0 && counterclockwise_flag==1) {
      stop_motor();
      return; 
    }
    read_inputs();
    if (lastButtonPushed == 9){
      stop_motor();
      return;
    }
    
 }
 if (run_mode == 2){
    motor_runtime -= ((millis() - TimeLastCheck));
    TimeLastCheck = millis();
    //Serial.println("Runtime :");
    //Serial.println(motor_runtime);
    //if (floor(Sens1Time/1000) == 99) {
     //lcd.setCursor(0, 1);
     //lcd.print("    AZIMUTH : ");
      //}
    //if (floor(Sens1Time/1000) == 9) {
     //lcd.setCursor(0, 1);
     //lcd.print("    AZIMUTH : ");
      //}
    if (clockwise_flag == 1) {
    //motor_position_var = (((motor_runtime_var/6)*1000) - motor_runtime);
    motor_position_var += ((millis() - motor_timelaps));
    motor_timelaps = millis();

    //Serial.println(motor_position_var);
    motor_position = (motor_startposition + ((motor_position_var/1000)*(360/motor_speed))); 
  
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0,0);
    lcd.print(" AZIMUTH :      ");
    lcd.setCursor(0, 0);
    lcd.print(" AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 1);
    lcd.print("<< CLOCKWISE >> ");
   
    }
    
    if (counterclockwise_flag == 1) {
    motor_position_var += ((millis() - motor_timelaps));
    motor_timelaps = millis();
    //Serial.println(motor_position_var);
    motor_position = (motor_startposition - ((motor_position_var/1000)*(360/motor_speed2)));
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0,0);
    lcd.print(" AZIMUTH :      ");
    lcd.setCursor(0, 0);
    lcd.print(" AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 1);
    lcd.print("<<COUNTERCLOCK>>");
    }
    //TimeLastCheck = millis(); 
    //lcd.setCursor(0, 3);
    //lcd.print("SYSTEM RUNNING .....");
    //if (motor_position <= 1) {
      //stop_motor();
      //return;  
    //}
    //if (motor_position >= 359) {
      //stop_motor();
      //return;  
    //}
    if (motor_position >= 360) {
      stop_motor();
    }
    if (motor_position <= 0) {
      stop_motor();
    }
    if (motor_runtime <= 0) {
      stop_motor();
     //return; 
     //delay (1000); 
     }
     read_inputs();
    if (lastButtonPushed == 9){
      stop_motor();
      return;
    }
    }   
 if (run_mode == 3){
    motor_runtime -= ((millis() - TimeLastCheck));
    TimeLastCheck = millis();
    //Serial.println("Runtime :");
    //Serial.println(motor_runtime);
    //if (floor(Sens1Time/1000) == 99) {
     //lcd.setCursor(0, 1);
     //lcd.print("    AZIMUTH : ");
      //}
    //if (floor(Sens1Time/1000) == 9) {
     //lcd.setCursor(0, 1);
     //lcd.print("    AZIMUTH : ");
      //}
    if (clockwise_flag == 1) {
    //motor_position_var = (((motor_runtime_var/6)*1000) - motor_runtime);
    motor_position_var += ((millis() - motor_timelaps));
    motor_timelaps = millis();

    //Serial.println(motor_position_var);
    motor_position = (motor_startposition + ((motor_position_var/1000)*(360/motor_speed))); 
  
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0,0);
    lcd.print(" AZIMUTH :      ");
    lcd.setCursor(0, 0);
    lcd.print(" AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 1);
    lcd.print("<< CLOCKWISE >> ");
   
    }
    
    if (counterclockwise_flag == 1) {
    motor_position_var += ((millis() - motor_timelaps));
    motor_timelaps = millis();
    //Serial.println(motor_position_var);
    motor_position = (motor_startposition - ((motor_position_var/1000)*(360/motor_speed2)));
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0,0);
    lcd.print(" AZIMUTH :      ");
    lcd.setCursor(0, 0);
    lcd.print(" AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 1);
    lcd.print("<<COUNTERCLOCK>>");
    }
    //TimeLastCheck = millis(); 
    //lcd.setCursor(0, 3);
    //lcd.print("SYSTEM RUNNING .....");
    //if (motor_position <= 1) {
      //stop_motor();
      //return;  
    //}
    //if (motor_position >= 359) {
      //stop_motor();
      //return;  
    //}
    if (motor_position >= 360 && clockwise_flag == 1) {
      stop_motor();
    }
    if (motor_position <= 0 && counterclockwise_flag == 1) {
      stop_motor();
    }
    if (motor_runtime <= 0) {
      stop_motor();
     return; 
     delay (1000); 
     }
     read_inputs();
    if (lastButtonPushed == 9){
      stop_motor();
      return;
    }
    }   
  }  

 

void scroll (String msg, int pos, int repeat) {
  
 for (int rep = 0; rep < repeat; rep++)  { 
  //lcd.clear();
  //lcd.begin(16, 2);
  lcd.print(msg);
  delay(150);
  // scroll 13 positions (string length) to the left
  // to move it offscreen left:
  for (int positionCounter = 0; positionCounter < pos; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    read_inputs();
    delay(150);
  }

  // scroll 13 positions (string length + display length) to the right
  // to move it offscreen right:
  for (int positionCounter = 0; positionCounter < pos; positionCounter++) {
    // scroll one position right:
    lcd.scrollDisplayRight();
    // wait a bit:
    read_inputs();
    delay(150);
  }
 
  // delay at the end of the full loop:
  delay(100);
  read_inputs();
 }
}
 
  void display_lcd(int col, int row, String msg) {
  
  lcd.setCursor(col,row);
  lcd.print(msg);
}

void run_clockwise(){
  if (motor_position + motor_runtime_var >= 360.1){
    lcd.setCursor(0,1);
    lcd.print(" Out of Range!  ");
    motor_runtime_var = 1;
    return;
  }
  motor_runtime = (motor_runtime_var/(360/motor_speed))*1000;
  lcd.setCursor(0,0);
  lcd.print("                ");
  set_position();
  digitalWrite(clockwise_relay, LOW); 
  digitalWrite(run_relay, LOW);
  clockwise_flag = 1; 
  lastButtonPushed = 0;
}

void run_counterclockwise(){
  if (motor_position - motor_runtime_var <= 0){
    lcd.setCursor(0,1);
    lcd.print(" Out of Range!  ");
    motor_runtime_var = 1;
    return;
  }
  motor_runtime = (motor_runtime_var/(360/motor_speed2))*1000;
  lcd.setCursor(0,0);
  lcd.print("                ");
  set_position();
  digitalWrite(counterclockwise_relay, LOW); 
  digitalWrite(run_relay, LOW); 
  counterclockwise_flag = 1; 
  lastButtonPushed = 0;
}

void stop_motor(){
  tone( piezoPin, 1500, 300);
  digitalWrite(run_relay, HIGH);
  digitalWrite(clockwise_relay, HIGH); 
  digitalWrite(counterclockwise_relay, HIGH);
  EEPROM.put(EE_addr1, motor_position); 
  clockwise_flag = 0;
  counterclockwise_flag = 0;
  //if (run_mode == 3){
    //lastButtonPushed = 4;
    //run_mode = 1;
  //}
  //if (run_mode == 1 || run_mode == 2){
  lastButtonPushed = 1;
  //}
  //lcd.setCursor(0, 0);
  //lcd.print("                ");
  update_motor_position();
}

void read_analog(){
  analogValue = analogRead(A0);
  analog_position = map(analogValue, 0, 1023, 0, 360);
  delay(100);
}

void get_position() {
      update_motor_position();
      //lcd.setCursor(0, 1);
      //lcd.print(" PUSH  SET to START!");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Enter Pos.<>:");
      motor_runtime_var = 1;
      lcd.print(motor_runtime_var,0);
      
      delay(1000);
      while (lastButtonPushed==2 || lastButtonPushed==3) {
        read_inputs();
        if (digitalRead(clockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(200);
          motor_runtime_var = motor_runtime_var+1;
        }
        if (digitalRead(counterclockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(200);
          motor_runtime_var = motor_runtime_var-1;
      }
      if (motor_runtime_var == 360) {
        //lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(" Out of Range!  ");
        motor_runtime_var = 1;
        tone( piezoPin, 1500, 250);
        delay(200);
        }
      if (motor_runtime_var == 0) {
        //lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("    INVALID!    ");
        motor_runtime_var = 1;
        tone( piezoPin, 2000, 250);
        delay(200);
        }

        
      lcd.setCursor(0, 1);   
      lcd.print("Enter Pos.<>:");
      lcd.print(motor_runtime_var,0);       
    }
    
    
  }

void manual_mode() {
      update_motor_position();
      lcd.setCursor(0, 1);
      lcd.print("PUSH </>  to RUN");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("<<Manual Mode>> ");
      delay(1000);
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("Target Pos. :");
      lcd.print(analog_position,0); 
      
      run_mode = 1;//manual
      
      delay(1000);
      while (lastButtonPushed==4) {
        read_inputs();
        read_analog();
        if (digitalRead(clockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(150);
          run_clockwise();
        }
        if (digitalRead(counterclockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(150);
          run_counterclockwise();
        }
        if (digitalRead(set_button)== 0) {
          tone( piezoPin, 1500, 250);
          delay(300);
          run_mode = 3;//analog mode
          if (motor_position < analog_position){
            motor_runtime_var = analog_position - motor_position;
            run_clockwise();
            return;
          }
          if (motor_position > analog_position){
            motor_runtime_var = motor_position - analog_position;
            run_counterclockwise();
            return;
          }
       
      }
      lcd.setCursor(0, 1);
      lcd.print("Target Pos. :   ");
      lcd.setCursor(0, 1);
      lcd.print("Target Pos. :");
      lcd.print(analog_position,0);            
      }
      if (digitalRead(set_button)== 0) {
          delay(150);
          stop_motor();
      }
    
  }
  
void set_motor_speed() {
      lcd.setCursor(0, 0);
      lcd.print("CALIBRATION MODE");
      lcd.setCursor(0, 1);
      lcd.print("PUSH SET to SAVE");
      delay(2000);
      lcd.setCursor(0, 0);
      lcd.print("SPEED SETTING   ");
      
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("CWiseSpeed:");
      //motor_speed = 5.0;
      lcd.print(motor_speed,1);
      
      delay(1000);
      while (lastButtonPushed==8) {
        read_inputs();
        if (digitalRead(clockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(250);
          motor_speed = motor_speed+0.5;
        }
        if (digitalRead(counterclockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(250);
          motor_speed = motor_speed-0.5;
        }
      if (motor_speed == 301) {
        //lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(" Out of Range!  ");
        motor_speed = 300.0;
        delay(1500);
        }
      if (motor_speed == 29 ) {
        //lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("    INVALID!    ");
        motor_speed = 30.0;
        delay(1500);
        }
        lcd.setCursor(0, 1);
        lcd.print("CWiseSpeed:");
        lcd.print(motor_speed,1);
      if (digitalRead(set_button)== 0) {
          lcd.setCursor(0, 1);
          lcd.print("SAVED TO MEMORY!");
          delay(1500);
          tone( piezoPin, 2000, 250);
          delay(150);
          EEPROM.put(EE_addr2, motor_speed);
          lastButtonPushed = 9;
          set_motor_speed_ccw();
      }
              
    }
  }
void set_motor_speed_ccw(){
  lcd.setCursor(0, 0);
      lcd.print("CALIBRATION MODE");
      lcd.setCursor(0, 1);
      lcd.print("PUSH SET to SAVE");
      delay(2000);
      lcd.setCursor(0, 0);
      lcd.print("SPEED SETTING   ");
      
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("CCWS Speed:");
      //motor_speed = 5.0;
      lcd.print(motor_speed2,1);
      
      delay(1000);
      while (lastButtonPushed==9) {
        read_inputs();
        if (digitalRead(clockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(250);
          motor_speed2 = motor_speed2+0.5;
        }
        if (digitalRead(counterclockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(250);
          motor_speed2 = motor_speed2-0.5;
        }
      if (motor_speed2 == 301) {
        //lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print(" Out of Range!  ");
        motor_speed2 = 300.0;
        delay(1500);
        }
      if (motor_speed2 == 29) {
        //lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("    INVALID!    ");
        motor_speed2 = 30.0;
        delay(1500);
        }
        lcd.setCursor(0, 1);
        lcd.print("CCWS Speed:");
        lcd.print(motor_speed2,1);
      if (digitalRead(set_button)== 0) {
          lcd.setCursor(0, 1);
          lcd.print("SAVED TO MEMORY!");
          delay(1500);
          tone( piezoPin, 2000, 250);
          delay(150);
          EEPROM.put(EE_addr3, motor_speed2);
          lastButtonPushed = 1;
      }
      
            
    }  
}
void saved_positions() {

      lcd.setCursor(0, 1);
      lcd.print("SELECT<>SETtoRUN");
      delay(2000);
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(locations[0]);
      int loc_index = 0;
      
      delay(1000);
      while (lastButtonPushed==5) {
        read_inputs();
        if (digitalRead(clockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(150);
          loc_index = loc_index+1;
          lcd.setCursor(0, 1);
          lcd.print("                ");
        }
        if (digitalRead(counterclockwise_button)== 0) {
          tone( piezoPin, 2000, 250);
          delay(150);
          loc_index = loc_index-1;
          lcd.setCursor(0, 1);
          lcd.print("                ");
        }
      if (loc_index == 9) {
        //lcd.clear();
        loc_index = 0;
        delay(500);
        }
      if (loc_index < 0) {
        //lcd.clear();
        loc_index = 0;
        delay(500);
        }
        //lcd.setCursor(0, 3);
        //lcd.print("                    ");
        lcd.setCursor(0, 1);
        lcd.print(locations[loc_index]);
        lcd.print("   ");
        lcd.print(positions[loc_index]);
        //lcd.print(" DEG");

      if (digitalRead(set_button)== 0) {
          tone( piezoPin, 1500, 250);
          delay(250);
          run_mode = 3;
          if (motor_position < positions[loc_index]){
            motor_runtime_var = positions[loc_index] - motor_position;
            run_clockwise();
            return;
          }
          if (motor_position > positions[loc_index]){
            motor_runtime_var = motor_position - positions[loc_index];
            run_counterclockwise();
            return;
          }         
      }           
    } 
  }


void loop() {
   read_inputs();
   main_menu(); 
   while (lastButtonPushed == 0)  {
    delay(10);
    manage_position(); 
    //read_inputs();
   }   
  }
