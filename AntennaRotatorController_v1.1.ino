//Antenna Rotator Controller Project 11/25/2020
//Samuel Rebosura

void(* resetFunc) (void) = 0; //software reset vector
#include <EEPROM.h> 
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

const int clockwise_button = 2;     // Button to move motor clockwise
const int counterclockwise_button = 3;     // Button to move motor counter clockwise
const int set_button = 4;
const int clockwise_relay = 5;
const int counterclockwise_relay = 6;
const int run_relay =  13;

int EE_addr1 = 1;
int clockwise_flag = 0;
int counterclockwise_flag = 0;
double motor_position_var = 0;
double motor_position = 0;
double motor_runtime_var = 0;
double motor_runtime = 0;
double motor_timer_check = 0;
double TimeLastCheck = 0;
long debounceDelay = 300;
int lastButtonPushed = 1;
double motor_startposition = 0;
double motor_timelaps = 0;

long lastClockwiseDebounceTime = 0;  
long lastCounterclockwiseDebounceTime = 0;  
long lastSetDebounceTime = 0;

int lastClockwiseButtonState = HIGH;  
int lastCounterclockwiseButtonState = HIGH;   
int lastSetButtonState = HIGH;

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
       
       digitalWrite(clockwise_relay, LOW); //make sure soft reset will turn off the relays
       digitalWrite(counterclockwise_relay, LOW);
       digitalWrite(run_relay, LOW);
       scroll("DV7BJC",20,1);
       lcd.begin(20, 4);
       //motor_position = 45;
      //EEPROM.put(EE_addr1, motor_position); 
}

void read_inputs() {
 // read the state of the switches/sensors into a local variable:
  int reading;
  int Clockwise_buttonState=HIGH;             
  int Counterclockwise_buttonState=HIGH;             
  int Set_buttonState=HIGH;     
 
  
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

//main menu actions

if(Set_buttonState==LOW && (menu_display==2  || menu_display==3)){
  //tone( piezoPin, 2000, 250);
  lastButtonPushed=1;
 }else if(Clockwise_buttonState==LOW && menu_display==1){
 //tone( piezoPin, 2100, 250);
  lastButtonPushed=2;
 }else if(Counterclockwise_buttonState==LOW && menu_display==1){
  //tone( piezoPin, 2100, 250);
  lastButtonPushed=3;
 //}else if(Counterclockwise_buttonState==LOW && (menu_display==1 || menu_display==2  || menu_display==3)){
  //tone( piezoPin, 2100, 250);
  //lastButtonPushed=1;
 }else if(Set_buttonState==LOW && menu_display==2){
  //tone( piezoPin, 2000, 250);
  lastButtonPushed=4;
 }else if(Set_buttonState==LOW && menu_display==3){
  //tone( piezoPin, 2000, 250);
  lastButtonPushed=5;
 }else if(Set_buttonState==LOW && menu_display==1){
  //tone( piezoPin, 2000, 250);
  lastButtonPushed=4;
   }else if(Clockwise_buttonState==LOW && Counterclockwise_buttonState==LOW && menu_display==1){
  //tone( piezoPin, 2000, 250);
  lastButtonPushed=6;
  }else if(Set_buttonState==LOW && menu_display==4 ){
  //tone( piezoPin, 2000, 250);
  lastButtonPushed=7;
}
}

void update_motor_position(){//update motor position

  EEPROM.get(EE_addr1,motor_position);
  //motor_position = 0;
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Rotator Controller ");
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  lcd.print("    AZIMUTH : ");
  lcd.print(motor_position,1);
  lcd.setCursor(0, 3);
  lcd.print("<<--    -->>     SET");
    
}

void main_menu() {
  
   switch (lastButtonPushed) {
    case 1:
      update_motor_position();
      menu_display = 1;
      last_menu = menu_display;
      break;
    
    case 2:
      
      lcd.setCursor(0, 0);
      lcd.print(" RUN CLOCKWISE >>>>>");
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print("    AZIMUTH : ");
      lcd.print(motor_position,1);
      
      menu_display = 2;
      last_menu = menu_display;
      delay(1000);
      get_position();

      run_clockwise();
      
      break;
      
    case 3:
      
      lcd.setCursor(0, 0);
      lcd.print(" RUN COUNTERCLOCK <<<");
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print("    AZIMUTH : ");
      lcd.print(motor_position,1);
      
      menu_display = 3;
      last_menu = menu_display;
      delay(1000);
      get_position();
     
      run_counterclockwise();
      
      break;

      case 4:
      
      lcd.setCursor(0, 0);
      lcd.print("<<< MANUAL MODE >>>>");
      lcd.setCursor(0, 1);
      lcd.print("                    ");
      lcd.setCursor(0, 1);
      lcd.print("    AZIMUTH : ");
      lcd.print(motor_position,1);
      
      menu_display = 4;
      last_menu = menu_display;
      delay(1000);
      
      
      break;
    
    case 6:
      
      lcd.setCursor(0, 0);
      lcd.print(" CALIBRATION MODE!  ");
      motor_position = 0;
      EEPROM.put(EE_addr1, motor_position);
      lcd.setCursor(0, 1);
      lcd.print("                    "); 
      lcd.setCursor(0, 1);
      lcd.print("    AZIMUTH : ");
      lcd.print(motor_position,1);
      
      menu_display = 6;
      last_menu = menu_display;
      delay(3000);
      lastButtonPushed = 1;      
      break;
   }
      
  }  



void set_position() {
    //Serial.println("Setting position");
    motor_runtime = (motor_runtime_var/6)*1000;
    TimeLastCheck = millis();
    motor_timelaps = millis();
    motor_startposition = motor_position;
    motor_position_var = 0;
    //lcd.setCursor(0, 3);
    //lcd.print(" << Motor running >>");
    //tone( piezoPin, 1750, 150);
  }
    
void manage_position() {
  
 if (lastButtonPushed == 2 || lastButtonPushed == 3){
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
    motor_position = (motor_startposition + ((motor_position_var/1000)*6)); 
  
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0, 1);
    lcd.print("    AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 2);
    lcd.print("  << CLOCKWISE >>   ");
   
    }
    
    if (counterclockwise_flag == 1) {
    motor_position_var += ((millis() - motor_timelaps));
    motor_timelaps = millis();
    //Serial.println(motor_position_var);
    motor_position = (motor_startposition - ((motor_position_var/1000)*6));
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0, 1);
    lcd.print("    AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 2);
    lcd.print(" << COUNTERCLOCK >> ");
    }
    //TimeLastCheck = millis(); 
    lcd.setCursor(0, 3);
    lcd.print("SYSTEM RUNNING .....");
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
     return; 
     //delay (1000); 
    
     }
    }
 
 if (lastButtonPushed == 4){

   if (clockwise_flag == 1) {
    //motor_position_var = (((motor_runtime_var/6)*1000) - motor_runtime);
    motor_position_var += ((millis() - motor_timelaps));
    motor_timelaps = millis();

    //Serial.println(motor_position_var);
    motor_position = (motor_startposition + ((motor_position_var/1000)*6)); 
  
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0, 1);
    lcd.print("    AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 2);
    lcd.print("PUSH SET KEY to STOP");
   
    }
    
    if (counterclockwise_flag == 1) {
    motor_position_var += ((millis() - motor_timelaps));
    motor_timelaps = millis();
    //Serial.println(motor_position_var);
    motor_position = (motor_startposition - ((motor_position_var/1000)*6));
    //Serial.println("Position :");
    //Serial.println(motor_position);
    lcd.setCursor(0, 1);
    lcd.print("    AZIMUTH : ");
    lcd.print(motor_position,1);
    lcd.setCursor(0, 2);
    lcd.print("PUSH SET KEY to STOP");
   
    }
    //TimeLastCheck = millis(); 
    lcd.setCursor(0, 3);
    lcd.print("SYSTEM RUNNING .....");
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
      return; 
    }
    if (motor_position <= 0) {
      stop_motor();
      return; 
    }
    read_inputs();
    if (lastButtonPushed == 7){
      stop_motor();
      return;
    }
    
 }
    
   
  }  

 

void scroll (String msg, int pos, int repeat) {
  
 for (int rep = 0; rep < repeat; rep++)  { 
  lcd.clear();
  lcd.begin(20, 4);
  lcd.print(msg);
  delay(250);
  // scroll 13 positions (string length) to the left
  // to move it offscreen left:
  for (int positionCounter = 0; positionCounter < pos; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    read_inputs();
    delay(250);
  }

  // scroll 13 positions (string length + display length) to the right
  // to move it offscreen right:
  for (int positionCounter = 0; positionCounter < pos; positionCounter++) {
    // scroll one position right:
    lcd.scrollDisplayRight();
    // wait a bit:
    read_inputs();
    delay(250);
  }
 
  // delay at the end of the full loop:
  delay(150);
  read_inputs();
 }
}
 
  void display_lcd(int col, int row, String msg) {
  
  lcd.setCursor(col,row);
  lcd.print(msg);
}

void run_clockwise(){
  if (motor_position + motor_runtime_var >= 360){
    lcd.print("   Out of Range!    ");
    motor_runtime_var = 1;
    return;
  }
  set_position();
  digitalWrite(clockwise_relay, HIGH); 
  digitalWrite(run_relay, HIGH);
  clockwise_flag = 1; 
  lastButtonPushed = 0;
}

void run_counterclockwise(){
  if (motor_position - motor_runtime_var <= 0){
    lcd.print("   Out of Range!    ");
    motor_runtime_var = 1;
    return;
  }
  set_position();
  digitalWrite(counterclockwise_relay, HIGH); 
  digitalWrite(run_relay, HIGH); 
  counterclockwise_flag = 1; 
  lastButtonPushed = 0;
}

void stop_motor(){
  digitalWrite(clockwise_relay, LOW); 
  digitalWrite(counterclockwise_relay, LOW);
  digitalWrite(run_relay, LOW);
  EEPROM.put(EE_addr1, motor_position); 
  clockwise_flag = 0;
  counterclockwise_flag = 0;
  lastButtonPushed = 1;
}

void get_position() {
      update_motor_position();
      lcd.setCursor(0, 2);
      lcd.print(" PUSH  SET to START!");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print("Enter Position<>:");
      motor_runtime_var = 1;
      lcd.print(motor_runtime_var,0);
      
      delay(1000);
      while (lastButtonPushed==2 || lastButtonPushed==3) {
        read_inputs();
        if (digitalRead(clockwise_button)== 0) {
          delay(100);
          motor_runtime_var = motor_runtime_var+1;
        }
        if (digitalRead(counterclockwise_button)== 0) {
          delay(100);
          motor_runtime_var = motor_runtime_var-1;
      }
      if (motor_runtime_var == 360) {
        //lcd.clear();
        lcd.setCursor(0, 3);
        lcd.print("   Out of Range!    ");
        motor_runtime_var = 1;
        delay(500);
        }
      if (motor_runtime_var == 0) {
        //lcd.clear();
        lcd.setCursor(0, 3);
        lcd.print("      INVALID!      ");
        motor_runtime_var = 1;
        delay(500);
        }

        
      lcd.setCursor(0, 3);   
      lcd.print("Enter Position<>:");
      lcd.print(motor_runtime_var,0);       
    }
    
    
  }

void manual_mode() {
      update_motor_position();
      lcd.setCursor(0, 2);
      lcd.print("PUSH </> KEYS to RUN");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      lcd.setCursor(0, 3);
      lcd.print(" << Manual Mode >>> ");
      
      delay(1000);
      while (lastButtonPushed==4) {
        read_inputs();
        if (digitalRead(clockwise_button)== 0) {
          delay(150);
          run_clockwise();
        }
        if (digitalRead(counterclockwise_button)== 0) {
          delay(150);
          run_clockwise();
      }
      
            
    }
      if (digitalRead(set_button)== 0) {
          delay(150);
          stop_motor();
      }
    
  }

void loop() {
   read_inputs();
   main_menu(); 
   while (lastButtonPushed == 0)  {
    manage_position(); 
    //read_inputs();
   }   
  }
