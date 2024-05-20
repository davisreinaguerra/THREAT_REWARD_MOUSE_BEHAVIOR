//Custom Classes______________________
#include "lick_sensor.h"
#include "solenoid.h"
#include "alignment.h"
#include "IR_sensor.h"

//LCD Display_________________________
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);
String menu_categories[4] = {"", "", "", ""};
String updatable_menu[4] = {"", "", "", ""};

//Pin Assignments_____________________
                                 // D2
                                 // D3
                                 // D4 
                                 // D5   
IR_sensor trigIR(6);             // D6   > Selectable
IR_sensor nestIR(7);             // D7   > Fixed
looming loom(8);                 // D8   > Simple trigger (TRIG, GND)
sound_player sound(9);           // D9   > Simple trigger (TRIG, GND)
LED LED_Cue_Nest(10);            // D10  > LED Vf=3V,Vc=200mA
LED LED_Cue_Reward(11);          // D11  > LED Vf=3V,Vc=200mA
lick_sensor lick(12);            // D12  > capacative sensor breakout
solenoid reward_port(13);        // D13  > solenoid MOSFET breakout

//Session config initialize________________________
long int n_trials;
long int intertrial_interval;
long int enter_time_limit;
long int reward_volume;
long int loom_qm;
long int sound_qm;
long int begin_qm;

//Data Structure____________________________________
typedef struct {
  // numerical data
  long int trial_duration;
  long int latency_to_trigger;
  long int latency_to_lick;
  // logical data  
  bool threat_triggered = false;
  bool port_licked = false;
} monster_session;

monster_session trial[max_trials];

//State Names______________________________________
#define trial_begun 1
#define mouse_entered 2
#define threat-triggered 3
#define reward_delivered 4

#define trial_ended 10

//Main Menu Indices_______________________________
#define idx_trial 0
#define idx_trig 1
#define idx_lick 2
#define idx_stopwatch 3

// Variable Initializations________________________
const int delay_between_lick_and_deliver = 500;
const int intertrial_interval = 50;
int current_trial = 1;
unsigned long start_time;
unsigned long lick_time;  
int previous_state;
int state = 0;
bool ready_to_begin = false;
const int max_trials = 30;
const int flush_volume = 200;


void setup() {
  // put your setup code here, to run once:

  // Initialize LCD screen_________________________
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // Configure Session_____________________________
  salient_display_message("Waiting 4 config...");
  
  mode = read_config(); // modes = debug, training, testing
  n_trials = read_config();
  trigger_time_limit = read_config();
  reward_volume = read_config();
  loom_qm = read_config();
  sound_qm = read_config();
  begin_qm = read_config();

  // Report Configuration_________________________
  menu_categories[0] = "n_trials";
  menu_categories[1] = "reward_volume";
  menu_categories[2] = "Loom?";
  menu_categories[3] = "Sound?";
  initialize_menu();
  updatable_menu[0] = String(n_trials);
  updatable_menu[1] = String(reward_volume) + " (ms)";
  updatable_menu[2] = String(loom_qm);
  updatable_menu[3] = String(sound_qm);
  update_menu();

  delay(5000);

  // Initialize Menu______________________________
  menu_categories[0] = "Trial: ";
  menu_categories[1] = "Triggered?: ";
  menu_categories[2] = "Licked?";
  menu_categories[3] = "Stopwatch";
  initialize_menu();
  updatable_menu[idk_trial] = "1";
  updatable_menu[idx_trig] = "False";
  updatable_menu[idx_lick] = "False";
  updatable_menu[idx_stopwatch] = "0";
  delay(5000);
  
  
  // Instantiate first trial________________________
  state = trial_begun;
  start_alignment.align_onset();
  start_time = millis();
  fast_open();

}

void loop() {
  // put your main code here, to run repeatedly:

  switch (mode) {
    
    // Debug
    case 0 {

    }

    // Training
    case 1 {
          // Have the lght turn on before each reward is available but dont require them to enter the nest again

    // Testing
    case 2 {
      
      // end session if you have reached the trial number
      if (current_trial > n_trials) {state = complete;}

      switch (state) {
        
        case trial_begun:
          
          if (trigIR.is_broken()) {                                                   // If mouse triggers the infrared beam sensor
            digitalWrite(loom, loom_qm);                                              //      1) Engage threatening stimuli if logic conferred 
            digitalWrite(sound, sound_qm)                                             //         ...
            updatable_menu[idx_stopwatch] = update_stop_watch(start_time);            //      2) Update the stopwatch
            trigger_time = millis();                                                  //      3) Determine the trigger time
            save_point = state;                                                       //      4) Save current state (before we try to catch Rayquaza, just in case...)
            state = triggered;                                                        //      5) Indicate the new state
            updatable_menu[idx_trig] = "True";                                        //      6) Display on the Main Menu that the threat has been triggered
            trial[current_trial].threat_triggered = true;                             //      7) Save to trial data that the threat has been triggered 
            trial[current_trial].latency_to_trigger = millis() - start_time;          //      8) Save the trigger time relative to start time
            break; 
          }
          if ((millis() - start_time) > trigger_time_limit) {                         // If mouse just hangs out in shelter      
            state = trial_ended;                                                      //      Fast forward to end of trial
            break;
          }

          break; 

        case threat_triggered:

          if (lick.is_licked(10)) {                                                   // If port has been licked
            lick_time = millis();                                                     //      1) Determine the lick time
            save_point = state;                                                       //      2) Save current state
            state = reward_delivered;                                                 //      3) Indicate the new state
            updatable_menu[idx_lick] = "True";                                        //      4) Display on the Main Menu that the port has been licked
            trial[current_trial].port_licked = true;                                  //      5) Save to trial data that the port has been licked
            trial[current_trial].latency_to_lick = millis() - trigger_time;           //      6) Save the lick time relative to trigger_time
            delay(delay_between_lick_and_deliver);                                    //      7) Delay to disambiguate lick and reward
            reward_port.pulse_valve(reward_volume);                                   //      8) Deliver reward

            break;
          }
          if (nestIR.is_broken()) {                                                   // If mouse returns to shelter without licking    
            state = trial_ended;                                                      //      Fast forward to end of trial
            break;
          }
          
          break;
        
        case reward_delivered:                                                        

          if (nestIR.is_broken()) {                                                   // When mouse returns to shelter after successfully acquiring reward
            trial[current_trial].escape_duration = millis() - trigger_time;           //          
            state = trial_ended;                                                      //       Conclude trial
            break;
          } delay(50);

          break;

        case trial_ended:

          digitalWrite(loom, LOW);                                                    // Turn off any threats which have been engaged
          digitalWrite(sound, LOW)                                                    // ...

          // display trial outcomes to serial monitor
          Serial.print("Trial = "); Serial.println(current_trial);

          Serial.print("trial_duration = "); Serial.println(trial[current_trial].trial_duration);
          Serial.print("latency_to_trigger = "); Serial.println(trial[current_trial].latency_to_trigger);
          Serial.print("latency_to_lick = "); Serial.println(trial[current_trial].latency_to_lick);
          Serial.print("threat_triggered = "); Serial.println(trial[current_trial].threat_triggered);
          Serial.print("port_licked = "); Serial.println(trial[current_trial].port_licked);

          // Wait for the intertrial interval
          delay(intertrial_interval);

          //Move on to next trial
          current_trial += 1;
          state = trial_begun;
          start_alignment.align_onset();
          start_time = millis();

          break;

        case 9: // Session Complete
          salient_display_message("Waiting 4 config...");
          while(1) { // to flush out port after, touch the reward port
            if (lick.is_licked(10)) {
              reward_port.pulse_valve(flush_volume);
              delay(1000);
            }  
          };
      }  
    }
  }
}

// Menu functions 
void initialize_menu() {
  lcd.clear();

  lcd.setCursor(12, 0);
  lcd.print(menu_categories[0]);

  lcd.setCursor(12, 1);
  lcd.print(menu_categories[1]);

  lcd.setCursor(12, 2);
  lcd.print(menu_categories[2]);

  lcd.setCursor(12, 3);
  lcd.print(menu_categories[3]);
}

void update_menu() {
  initialize_menu();

  lcd.setCursor(0, 0);
  lcd.print(updatable_menu[0]);

  lcd.setCursor(0, 1);
  lcd.print(updatable_menu[1]);

  lcd.setCursor(0, 2);
  lcd.print(updatable_menu[2]);

  lcd.setCursor(0, 3);
  lcd.print(updatable_menu[3]);
}

void salient_display_message(String message) {
  lcd_clear();

  lcd.setCursor(0, 0);
  lcd.print(message);
  delay(500);

  lcd.setCursor(0, 1);
  lcd.print(updatable_menu[1]);
  delay(500);

  lcd.setCursor(0, 2);
  lcd.print(updatable_menu[2]);
  delay(500);

  lcd.setCursor(0, 3);
  lcd.print(updatable_menu[3]);
  delay(500);
}

// Read configuration Function
long int read_config() {
  while (!Serial.available()) {}
  return Serial.parseInt();
}

// Basic LCD Write
void lcd_write(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void update_stopwatch(start_time) {
  out = millis() - start_time;
  return out;
}
