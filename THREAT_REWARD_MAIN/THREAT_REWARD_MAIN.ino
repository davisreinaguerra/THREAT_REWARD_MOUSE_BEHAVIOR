//Custom Classes______________________
#include "lick_sensor.h"
#include "solenoid.h"
#include "alignment.h"
#include "IR_sensor.h"
#include "looming.h"
#include "LED.h"
#include "sound_player.h"

//LCD Display_________________________
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);
String menu_categories[4] = {"", "", "", ""};
String updatable_menu[4] = {"", "", "", ""};

//Pin Assignments_____________________

IR_sensor trigIR[4] = {
  trigIR_1(2),                   // D2   > how far from port
  trigIR_2(3),                   // D3   > how far from port
  trigIR_3(4),                   // D4   > how far from port
  trigIR_4(5)                    // D5   > how far from port
}

IR_sensor noseIR(6);             // D6   > nose poke entry
IR_sensor nestIR(7);             // D7   > nest entry
looming loom(8);                 // D8   > Simple trigger (TRIG, GND)
sound_player sound(9);           // D9   > Simple trigger (TRIG, GND)
LED LED_Cue_Nest(10);            // D10  > LED Vf=3V,Vc=200mA
LED LED_Cue_Reward(11);          // D11  > LED Vf=3V,Vc=200mA
lick_sensor lick(12);            // D12  > capacative sensor breakout
solenoid reward_port(13);        // D13  > solenoid MOSFET breakout

//Session config initialize________________________
long int n_trials; 
long int trigger_time_limit;
long int reward_volume;
bool training_qm = false;
bool loom_qm = false;
bool sound_qm = false;
bool begin_qm= false;

//Data Structure____________________________________
typedef struct {
  // numerical data
  long int trial_duration;                    // Time between trial start and trial end
  long int latency_to_trigger;                // Time between trial start and trigger
  long int latency_to_lick;                   // Time between trigger and lick
  // logical data  
  bool threat_triggered = false;              // was the threat triggered?
  bool port_licked = false;                   // was the port licked?
} monster_session;

monster_session trial[max_trials];

//State Names______________________________________
#define trial_begun 1
#define mouse_entered 2
#define trigger_tripped 3
#define reward_delivered 4
#define trial_ended 5
#define session_complete 6

//Main Menu Indices_______________________________
#define idx_trial 0
#define idx_trig 1
#define idx_lick 2
#define idx_stopwatch 3

// Fixed variables ________________________________
const int delay_between_lick_and_deliver = 500;
const int intertrial_interval = 50;

// Variable Initializations________________________
int current_trial = 1;
unsigned long start_time;
int save_point;
int state = 0;
const int max_trials = 30;
const int flush_volume = 200;
int num_licks_training = 0;


void setup() {

  // Initialize LCD screen_________________________
  lcd.init();
  lcd.clear();
  lcd.backlight();
  salient_display_message("Waiting 4 config...");
  
  // Collect configuration parameters one by one
  n_trials = read_config();
  trigger_time_limit = read_config();
  which_trigger = read_config();
  reward_volume = read_config();
  training_qm = read_config();
  loom_qm = read_config();
  sound_qm = read_config();
  begin_qm = read_config();

  // Training _____________________________________
  if (training_qm = true) {
      
      LED_Cue_Reward.LED_on();
      
      while(num_licks_training<n_trials) {
        if (lick.is_licked(10)) {                                                   // If port has been licked
          num_licks_training++;                                                     //      1) Increment up the number of licks
          LED_Cue_Reward.LED_off();                                                 //      2) Turn off the reward cue
          delay(delay_between_lick_and_deliver);                                    //      7) Delay to disambiguate lick and reward
          reward_port.pulse_valve(reward_volume);                                   //      8) Deliver reward
          delay(5000);
          LED_Cue_Reward.LED_on();
        }
      }
      salient_display_message("Training Complete...");
  }

  // Report Configuration for 5 seconds___________
  menu_categories[0] = "n_trials";
  menu_categories[1] = "reward_volume";
  menu_categories[2] = "Loom?";
  menu_categories[3] = "Sound?";
  menu_category_set();
  updatable_menu[0] = String(n_trials);
  updatable_menu[1] = String(reward_volume) + " (ms)";
  updatable_menu[2] = String(loom_qm);
  updatable_menu[3] = String(sound_qm);
  update_menu();

  delay(3000);

  // Initialize Menu______________________________
  menu_categories[idx_trial] = "Trial: ";
  menu_categories[idx_trig] = "Triggered?: ";
  menu_categories[idx_lick] = "Licked?";
  menu_categories[idx_stopwatch] = "Stopwatch";
  menu_category_set();
  updatable_menu[idk_trial] = "1";
  updatable_menu[idx_trig] = "False";
  updatable_menu[idx_lick] = "False";
  updatable_menu[idx_stopwatch] = "0";
  delay(5000);
  
  
  // Instantiate first trial________________________
  LED_Cue_Nest.LED_on();                           // Turn on the Nest LED Cue
  while(nestIR.isnt_broken()) {}                   // Wait until the mouse enters the nest to start the first trial
  LED_Cue_Nest.LED_off();                          // Turn off the Nest LED
  LED_Cue_Reward.LED_on();                         // Turn on the Reward LED
  state = trial_begun;       
  start_time = millis();

}

void loop() {

  // End session if you have reached the trial number
  if (current_trial > n_trials) {state = complete;}

  // Control the flow of states
  switch (state) {
    
    case trial_begun:
      
      // Update stopwatch
      updatable_menu[idx_stopwatch] = update_stopwatch_relative_to(start_time);

      // Check for state switching events
      if (trigIR[which_trigger].is_broken()) {                                    // If mouse triggers the infrared beam sensor
        digitalWrite(loom, loom_qm);digitalWrite(sound, sound_qm);                //      1) Engage threatening stimuli if logic conferred
        save_point = state;                                                       //      2) Save current state (before we try to catch Rayquaza, just in case...)
        state = trigger_tripped;                                                  //      3) Move to trigger_tripped state
        updatable_menu[idx_trig] = "True";                                        //      4) Display on the Main Menu that the threat has been triggered
        trial[current_trial].threat_triggered = true;                             //      5) Save to trial data that the threat has been triggered 
        trial[current_trial].latency_to_trigger = millis() - start_time;          //      6) Save the trigger time relative to start time
        break; 
      }

      // Account for failures to trigger
      if ((millis() - start_time) > trigger_time_limit) {                         // If mouse just hangs out in or near shelter     
        save_point = state;                                                       //      1) Save current state
        state = trial_ended;                                                      //      2) Fast forward to end of trial
        LED_Cue_Reward.LED_off();                                                 
        LED_Cue_Nest.LED_on();
        break;
      }

      break; 

    case trigger_tripped:

      // update stopwatch
      updatable_menu[idx_stopwatch] = update_stopwatch_relative_to(start_time);

      // Check for state Switching Events
      if (lick.is_licked(10)) {                                                   // If port has been licked
        lick_time = millis();                                                     //      1) Determine the lick time
        save_point = state;                                                       //      2) Save current state
        state = reward_delivered;                                                 //      3) Indicate the new state
        updatable_menu[idx_lick] = "True";                                        //      4) Display on the Main Menu that the port has been licked
        trial[current_trial].port_licked = true;                                  //      5) Save to trial data that the port has been licked
        trial[current_trial].latency_to_lick = millis() - trigger_time;           //      6) Save the lick time relative to trigger_time
        delay(delay_between_lick_and_deliver);                                    //      7) Delay to disambiguate lick and reward
        reward_port.pulse_valve(reward_volume);                                   //      8) Deliver reward
        LED_Cue_Reward.LED_off();                                                 //      9) Turn off reward LED
        LED_Cue_Nest.LED_on();                                                    //      10) Turn on Nest LED
        break;
      }

      // Account for failures after trigger
      if (nestIR.is_broken()) {                                                   // If mouse returns to shelter without licking    
        save_point = state;                                                       //      1) Save current state
        state = trial_ended;                                                      //      2) Fast forward to end of trial
        LED_Cue_Reward.LED_off();                                                 
        LED_Cue_Nest.LED_on();
        break;
      }
      
      break;
    
    case reward_delivered:     

      // update stopwatch
      updatable_menu[idx_stopwatch] = update_stopwatch_relative_to(start_time);                                                         

      // Check for trial end
      if (nestIR.is_broken()) {                                                   // When mouse returns to shelter after successfully acquiring reward
        trial[current_trial].escape_duration = millis() - trigger_time;           //          
        state = trial_ended;                                                      //       Conclude trial
        break;
      } delay(50);

      break;

    case trial_ended:

      digitalWrite(loom, LOW); digitalWrite(sound, LOW);                          // Turn off any threats which have been engaged

      // display trial outcomes to serial monitor
      Serial.print("Trial = "); Serial.println(current_trial);

      Serial.print("trial_duration = "); Serial.println(trial[current_trial].trial_duration);
      Serial.print("latency_to_trigger = "); Serial.println(trial[current_trial].latency_to_trigger);
      Serial.print("latency_to_lick = "); Serial.println(trial[current_trial].latency_to_lick);
      Serial.print("threat_triggered = "); Serial.println(trial[current_trial].threat_triggered);
      Serial.print("port_licked = "); Serial.println(trial[current_trial].port_licked);

      // Wait for the intertrial interval
      delay(intertrial_interval);

      //Increment up trials
      current_trial += 1;

      // initiate next trial
      LED_Cue_Nest.LED_off();
      LED_Cue_Reward.LED_on();
      state = trial_begun;       
      start_time = millis();
      break;

    case session_complete:
      salient_display_message("Complete!");
      Serial.print("$");
      while(1) {// to flush out port after, touch the reward port
        if (lick.is_licked(10)) {
          reward_port.pulse_valve(flush_volume);
          delay(1000);
        }  
      }
  }
}

// Menu functions 
void menu_category_set() {
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
  menu_category_set();

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

void update_stopwatch_relative_to(start_time) {
  out = millis() - start_time;
  return out;
}

