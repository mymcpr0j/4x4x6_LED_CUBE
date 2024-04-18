#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <string.h>
#include "fonts.h"

const uint16_t recvPin = 32;  // Infrared receiving pin
IRrecv irrecv(recvPin);       // Create a class object used to receive class
decode_results results;       // Create a decoding results class object

#define FRQ 1000   // define the pwm frequency
#define PWM_BIT 8  // define the pwm precision

#define PWM_LOW 0
#define PWM_HIGH 255
#define PWM_CHANGE_STEP 20  // manual brightness adjusting step

#define LAYER_SHOW_TIME 2  // time in ms to show a single layer

#define COLUMNS_COUNT 16
#define LAYERS_COUNT 6

#define VERTICAL_PLANES_COUNT 4
#define ELEMENTS_IN_VERTICAL_PLANE_COUNT 4
#define VERTICAL_PLANES_DIRECTION_COUNT 4

#define MODES_COUNT 6  // 5 modes + all off
#define SUBMODES_COUNT 9

// ESP32 pin and pwm channel for the pin, used to manage layers
int layers[LAYERS_COUNT][2] = {
  { 17, 0 },
  { 19, 1 },
  { 22, 2 },
  { 23, 3 },
  { 21, 4 },
  { 18, 5 }
};

// Current PWM for each layer
byte current_pwm[LAYERS_COUNT];

// 74HC595 IC2 and IC1 pin corresponding to the column
byte columns[COLUMNS_COUNT][2] = {
  { 0b00100000, 0b00000000 },  // Col 1 IC2 Pin QC
  { 0b00001000, 0b00000000 },  // Col 2 IC2 Pin QE
  { 0b00000000, 0b00010000 },  // Col 3 IC1 Pin QD
  { 0b00000000, 0b00000100 },  // Col 4 IC1 Pin QF
  { 0b00010000, 0b00000000 },  // Col 5 IC2 Pin QD
  { 0b01000000, 0b00000000 },  // Col 6 IC2 Pin QB
  { 0b00000001, 0b00000000 },  // Col 7 IC2 Pin QH
  { 0b00000000, 0b00100000 },  // Col 8 IC1 Pin QC
  { 0b00000100, 0b00000000 },  // Col 9 IC2 Pin QF
  { 0b10000000, 0b00000000 },  // Col 10 IC2 Pin QA
  { 0b00000010, 0b00000000 },  // Col 11 IC2 Pin QG
  { 0b00000000, 0b10000000 },  // Col 12 IC1 Pin QA
  { 0b00000000, 0b01000000 },  // Col 13 IC1 Pin QB
  { 0b00000000, 0b00001000 },  // Col 14 IC1 Pin QE
  { 0b00000000, 0b00000001 },  // Col 15 IC1 Pin QH
  { 0b00000000, 0b00000010 },  // Col 16 IC1 Pin QG
};

// Full description of all LEDs states
byte current_animation[LAYERS_COUNT][2] = {
  { 0b00000000, 0b00000000 },
  { 0b00000000, 0b00000000 },
  { 0b00000000, 0b00000000 },
  { 0b00000000, 0b00000000 },
  { 0b00000000, 0b00000000 },
  { 0b00000000, 0b00000000 }
};

// The direction in which vertical planes are filled when displaying symbols;
// this variable is used to rotate symbols
int current_cols_direction = 0;

// 4 display modes for symbols
int cols_diff_direction[VERTICAL_PLANES_DIRECTION_COUNT][VERTICAL_PLANES_COUNT][ELEMENTS_IN_VERTICAL_PLANE_COUNT]{
  { // front to back
    { 0, 1, 2, 3 },
    { 4, 5, 6, 7 },
    { 8, 9, 10, 11 },
    { 12, 13, 14, 15 } },
  { // left to right
    { 12, 8, 4, 0 },
    { 13, 9, 5, 1 },
    { 14, 10, 6, 2 },
    { 15, 11, 7, 3 } },
  { // back to front
    { 15, 14, 13, 12 },
    { 11, 10, 9, 8 },
    { 7, 6, 5, 4 },
    { 3, 2, 1, 0 } },
  { // right to left
    { 3, 7, 11, 15 },
    { 2, 6, 10, 14 },
    { 1, 5, 9, 13 },
    { 0, 4, 8, 12 } }
};

int latchPin = 2;  // IC 74HC595 #1 & #2 RCK
int clockPin = 4;  // IC 74HC595 #1 & #2 SCK
int dataPin = 15;  // IC 74HC595 #1 SER

// current animation start time
unsigned long current_animation_start_time;

// Strings to show in text mode
char *str_to_show[SUBMODES_COUNT] = {
  "HELLO, WORLD!",
  "ASK, AND IT WILL BE GIVEN TO YOU; SEEK, AND YOU WILL FIND; KNOCK, AND IT WILL BE OPENED TO YOU./MATTHEW 7:7|",
  "LIFE ALL COMES DOWN TO A FEW MOMENTS. THIS IS ONE OF THEM./BUD FOX, WALL STREET|",
  "THE MUSIC IS NOT IN THE NOTES, BUT IN THE SILENCE BETWEEN./WOLFGANG AMADEUS MOZART|",
  "MONEY NEVER SLEEPS, PAL./GORDON GEKKO, WALL STREET|",
  "FIRST PRINCIPLES, CLARICE. SIMPLICITY. READ MARCUS AURELIUS./HANNIBAL LECTER, THE SILENCE OF THE LAMBS|",
  "DIAMONDS ARE A GIRL'S BEST FRIEND./MARILYN MONROE|",
  my_font_symbols
};

// Indicates which mode/submode is active
// only one mode/submode can be active at the time
int modes[MODES_COUNT][SUBMODES_COUNT] = {
  { // 0 - all off
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0 },
  {     // 1 - all on modes
    1,  // all on
    0,  // breathing light
    0,  // down-top brightness decreasing
    0,  // top-down brightness decreasing
    0,  // edges-center brightness decreasing
    0,  // center-edges brightness decreasing
    0,  // opposite change in brightness of neighboring layers
    0,
    0 },
  {     // 2 - random modes
    0,  // random LEDs
    0,  // random symbols
    0,
    0,
    0,
    0,
    0,
    0,
    0 },
  {     // 3 - clock mode
    0,  // show time
    0,  // set/show first hours number
    0,  // set/show second hours number
    0,  // set/show first mins number
    0,  // set/show second mins number
    0,
    0,
    0,
    0 },
  { // 4 - text mode
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0 },
  {      // 5 - technical modes
    0,   // show columns
    0,   // show horizontal layers
    0,   // show individual LEDs (voxels)
    0,   // show vertical layers
    0,   // show vertical layers
    0,   // show vertical layers
    0,   // show vertical layers
    0,   // show edges
    0 }  // measurement mode
};

// Indicates whether manual brightness change is allowed
// for any given mode/submode
int manual_pwm_change[MODES_COUNT][SUBMODES_COUNT] = {
  { // 0 - all off
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0 },
  {     // 1 - all on modes
    1,  // all on
    0,  // breathing light
    0,  // down-top brightness decreasing
    0,  // top-down brightness decreasing
    0,  // edges-center brightness decreasing
    0,  // cehter-edges brightness decreasing
    0,  // opposite change in brightness of neighboring layers
    0,
    0 },
  {     // 2 - random modes
    1,  // random LEDS
    1,  // random symbols
    0,
    0,
    0,
    0,
    0,
    0,
    0 },
  {     // 3 - clock mode
    1,  // show time
    1,  // set/show first hours number
    1,  // set/show second hours number
    1,  // set/show first mins number
    1,  // set/show second mins number
    0,
    0,
    0,
    0 },
  { // 4 - text mode
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1 },
  {      // 5 - technical modes
    1,   // show columns
    1,   // show horizontal layers
    1,   // show individual LEDs (voxels)
    1,   // show vertical layers
    1,   // show vertical layers
    1,   // show vertical layers
    1,   // show vertical layers
    1,   // show edges
    0 }  // measurement mode
};

// used to switch between sumbodes
int submodes_count_in_modes[MODES_COUNT]{
  1,
  7,
  2,
  5,
  8,
  9
};

// functions of this type are used for update cube state
typedef void (*anim)();

// struct of this type is used to return current mode/submode from a function
typedef struct {
  int mode;
  int submode;
} mode_submode;

// used to determine current mode/submode
mode_submode current_mode_submode;

// an array to store animations update functions
anim animations[MODES_COUNT][SUBMODES_COUNT];

// a structure to store time for clock
struct my_time {
  unsigned long hours;
  unsigned long minutes;
  unsigned long seconds;
  unsigned long millis_time_set;
};

struct my_time clock_time;

// a char array to store string time representation to show
char time_to_show[8];

// a char array to store string number representation to show during time setting
char time_setting_to_show[2];

// used to store piece data during time setting
unsigned long time_setting;

// used to store all time settings
unsigned long stored_time_settings[4];

// used to determine first calls of the animation functions
int first_anim_pass = 1;

void setup() {
  // enabling integrated pull-up resistor
  pinMode(recvPin, INPUT_PULLUP);

  // start the IR receiver
  irrecv.enableIRIn();

  // uncomment to enable printing through serial port
  /*
  Serial.begin(115200); 
  */

  for (int i = 0; i < LAYERS_COUNT; i++) {
    ledcSetup(layers[i][1], FRQ, PWM_BIT);      // setup pwm channel
    ledcAttachPin(layers[i][0], layers[i][1]);  // attach the led pin to pwm channel
    ledcWrite(layers[i][1], PWM_LOW);           // all layers are in off mode
  }

  // setup pins for IC's 74HC595
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);

  // maximum brightness for all LEDs
  set_all_pwm(PWM_HIGH);

  // filling the array of animation functions
  animations[0][0] = &update_animation_0_0;  // all off

  animations[1][0] = &update_animation_1_0;  // all on
  animations[1][1] = &update_animation_1_1;  // breathing light
  animations[1][2] = &update_animation_1_2;  // down-top brightness decreasing
  animations[1][3] = &update_animation_1_3;  // top-down brightness decreasing
  animations[1][4] = &update_animation_1_4;  // edges-center brightness decreasing
  animations[1][5] = &update_animation_1_5;  // center-edges brightness decreasing
  animations[1][6] = &update_animation_1_6;  // opposite change in brightness of neighboring layers

  animations[2][0] = &update_animation_2_0;  // random LEDs
  animations[2][1] = &update_animation_2_1;  // random symbols

  animations[3][0] = &update_animation_3_0;  // clock
  animations[3][1] = &update_animation_3_1;  // set/show first hours number
  animations[3][2] = &update_animation_3_2;  // set/show second hours number
  animations[3][3] = &update_animation_3_3;  // set/show first mins number
  animations[3][4] = &update_animation_3_4;  // set/show second mins number

  animations[4][0] = &update_animation_4_0;  // text
  animations[4][1] = &update_animation_4_0;  // text
  animations[4][2] = &update_animation_4_0;  // text
  animations[4][3] = &update_animation_4_0;  // text
  animations[4][4] = &update_animation_4_0;  // text
  animations[4][5] = &update_animation_4_0;  // text
  animations[4][6] = &update_animation_4_0;  // text
  animations[4][7] = &update_animation_4_0;  // text

  animations[5][0] = &update_animation_5_0;  // show columns
  animations[5][1] = &update_animation_5_1;  // show horizontal layers
  animations[5][2] = &update_animation_5_2;  // show individual LEDs (voxels)
  animations[5][3] = &update_animation_5_3;  // show vertical layers
  animations[5][4] = &update_animation_5_4;  // show vertical layers
  animations[5][5] = &update_animation_5_5;  // show vertical layers
  animations[5][6] = &update_animation_5_6;  // show vertical layers
  animations[5][7] = &update_animation_5_7;  // show edges
  animations[5][8] = &update_animation_5_8;  // measurement mode

  current_animation_start_time = millis();

  // we should set the clock manually later
  set_current_time(12, 34, 0);
}

void loop() {
  // find out the current mode and submode to use in the program
  current_mode_submode = mode_submode_index();

  // clock time update
  update_time();

  // Calling a function which scans layers and show an animation
  // described in a function which is correspond to current mode/submode
  scan_layers(animations[current_mode_submode.mode][current_mode_submode.submode]);

  // find out the current IR command (if any)
  decode_ir();
}

void scan_layers(anim anim_func) {
  // this function may update current_animation array and current_pwm arrays to
  // control on/off state of the LEDs and brightness of LEDs layers
  anim_func();

  // if in measurement mode
  if (current_mode_submode.mode == 5 && current_mode_submode.submode == 8) {
    send_data_to_ICs(current_animation[0][0], current_animation[0][1]);
    ledcWrite(layers[0][1], current_pwm[0]);
  } else {
    // for each layer
    for (int layer = 0; layer < LAYERS_COUNT; layer++) {
      // turn off all layers
      off_all_layers();
      // turn on/off LEDs for the current layer using shift registers
      send_data_to_ICs(current_animation[layer][0], current_animation[layer][1]);
      // turn on current layer using PWM settings
      ledcWrite(layers[layer][1], current_pwm[layer]);
      // wait for some time
      delay(LAYER_SHOW_TIME);
    }
  }
}

void decode_ir() {
  if (irrecv.decode(&results)) {  // Waiting for decoding
                                  // uncomment to see IR commands
                                  /*
  serialPrintUint64(results.value, HEX);
  Serial.println("");
  */
    switch (results.value) {
      case 0xFF02FD:  // + - increase brightness level
        if (manual_pwm_change[current_mode_submode.mode][current_mode_submode.submode] == 1) {
          change_brightness_level(1, PWM_CHANGE_STEP);
        }
        break;
      case 0xFF9867:  // - - decrease brightness level
        if (manual_pwm_change[current_mode_submode.mode][current_mode_submode.submode] == 1) {
          change_brightness_level(-1, PWM_CHANGE_STEP);
        }
        break;
      case 0xFF906F:  // >>| - next submode
        if (current_mode_submode.mode != 0 && current_mode_submode.mode != 3) {
          switch_submode(1, current_mode_submode.mode, current_mode_submode.submode);
        }
        first_anim_pass = 1;
        break;
      case 0xFFE01F:  // |<< - previous submode
        if (current_mode_submode.mode != 0) {
          switch_submode(-1, current_mode_submode.mode, current_mode_submode.submode);
        }
        first_anim_pass = 1;
        break;
      case 0xFFA25D:  // POWER - off/on and clear all
        if (current_mode_submode.mode == 0) {
          change_mode_submode(1, 0);
        } else {
          change_mode_submode(0, 0);
        }
        break;
      case 0xFF30CF:  // 1 - all on
        if (current_mode_submode.mode != 0) {
          change_mode_submode(1, 0);
        }
        break;
      case 0xFF18E7:  // 2 - random modes
        if (current_mode_submode.mode != 0) {
          change_mode_submode(2, 0);
        }
        first_anim_pass = 1;
        break;
      case 0xFF7A85:  // 3 - clock mode
        if (current_mode_submode.mode != 0) {
          change_mode_submode(3, 0);
        }
        first_anim_pass = 0;
        break;
      case 0xFF10EF:  // 4 - text modes
        if (current_mode_submode.mode != 0) {
          change_mode_submode(4, 0);
        }
        break;
      case 0xFF38C7:  // 5 - technical modes
        if (current_mode_submode.mode != 0) {
          change_mode_submode(5, 0);
        }
        break;
      case 0xFFC23D:  // reverse arrow  - change symbol orientation in text and clock modes
        if (current_mode_submode.mode == 4 || current_mode_submode.mode == 3 || current_mode_submode.mode == 2) {
          change_symbol_orientation();
        }
        break;
      case 0xFFA857:  // Play - reset current mode/submode
        change_mode_submode(current_mode_submode.mode, current_mode_submode.submode);
        break;
      case 0xFFE21D:  // Menu - clock settings
                      // for clock setting
        if (current_mode_submode.mode == 3) {
          switch_submode(1, current_mode_submode.mode, current_mode_submode.submode);

          //check previous submode
          if (current_mode_submode.submode == 1) {
            stored_time_settings[0] = time_setting;
            first_anim_pass = 1;
          }
          if (current_mode_submode.submode == 2) {
            stored_time_settings[1] = time_setting;
            first_anim_pass = 1;
          }
          if (current_mode_submode.submode == 3) {
            stored_time_settings[2] = time_setting;
            first_anim_pass = 1;
          }
          if (current_mode_submode.submode == 4) {
            stored_time_settings[3] = time_setting;

            set_current_time(
              stored_time_settings[0] * 10 + stored_time_settings[1],
              stored_time_settings[2] * 10 + stored_time_settings[3],
              clock_time.seconds);
            first_anim_pass = 0;
          }
          time_setting = 0;
        }
        break;
      case 0xFFB04F:  // C - increase a number during clock settings
        if (time_setting + 1 < 10) {
          time_setting += 1;
        } else {
          time_setting = 0;
        }
        if (current_mode_submode.submode == 1) {
          if (time_setting > 2) {
            time_setting = 0;
          }
        }
        if (current_mode_submode.submode == 2) {
          if (stored_time_settings[0] == 2 && time_setting > 3) {
            time_setting = 0;
          }
        }
        if (current_mode_submode.submode == 3) {
          if (time_setting > 6) {
            time_setting = 0;
          }
        }

        break;
    }
    irrecv.resume();  // Release the IRremote. Receive the next value
  }
}

void update_time() {
  unsigned long time_passed = millis() - clock_time.millis_time_set;
  unsigned long h_calc, m_calc, s_calc;

  if (time_passed >= 1000) {
    s_calc = clock_time.seconds + 1;
    if (s_calc < 60) {
      set_current_time(clock_time.hours, clock_time.minutes, s_calc);
    } else {
      s_calc = 0;
      m_calc = clock_time.minutes + 1;
      if (m_calc < 60) {
        set_current_time(clock_time.hours, m_calc, s_calc);
      } else {
        m_calc = 0;
        h_calc = clock_time.hours + 1;
        if (h_calc < 24) {
          set_current_time(h_calc, m_calc, s_calc);
        } else {
          h_calc = 0;
          set_current_time(h_calc, m_calc, s_calc);
        }
      }
    }
  }
}

void set_current_time(unsigned long h, unsigned long m, unsigned long s) {
  clock_time.hours = (h < 24) ? h : 0;
  clock_time.minutes = (m < 60) ? m : 0;
  clock_time.seconds = (s < 60) ? s : 0;
  clock_time.millis_time_set = millis();
  construct_time_string(time_to_show, clock_time);
}

void construct_time_string(char *time_string, my_time source_time) {
  char temp[3];

  time_string[0] = '[';

  sprintf(temp, "%d", source_time.hours);

  if (strlen(temp) == 1) {
    time_string[1] = '0';
    time_string[2] = temp[0];
  } else {
    time_string[1] = temp[0];
    time_string[2] = temp[1];
  }

  time_to_show[3] = ':';

  sprintf(temp, "%d", source_time.minutes);

  if (strlen(temp) == 1) {
    time_string[4] = '0';
    time_string[5] = temp[0];
  } else {
    time_string[4] = temp[0];
    time_string[5] = temp[1];
  }

  time_string[6] = ']';

  time_string[7] = '\0';
}

void set_all_pwm(byte pwm) {
  for (int layer = 0; layer < LAYERS_COUNT; layer++) {
    current_pwm[layer] = pwm;
  }
}

void set_pwm_for_layer(byte pwm, int layer) {
  current_pwm[layer] = pwm;
}

void turn_on_all_leds() {
  for (int i = 0; i < LAYERS_COUNT; i++) {
    for (int j = 0; j < 2; j++) {
      current_animation[i][j] = 0b11111111;
    }
  }
}

// 0.0 - all off
void update_animation_0_0() {
  clear_animation();
  off_all_layers();
}

// 1.0 - all on
void update_animation_1_0() {
  turn_on_all_leds();
}

// 1.1 - breathing light
void update_animation_1_1() {
  int brightness_step = 5;
  unsigned long one_brightness_level_duration = 100;
  unsigned long animation_duration = millis() - current_animation_start_time;
  unsigned long animation_step = animation_duration / one_brightness_level_duration;
  unsigned long one_pwm_pass_steps = PWM_HIGH / brightness_step;

  turn_on_all_leds();

  if (animation_step <= one_pwm_pass_steps) {
    set_all_pwm(PWM_HIGH - brightness_step * animation_step);
  }

  if (animation_step > one_pwm_pass_steps && animation_step <= one_pwm_pass_steps * 2) {
    set_all_pwm(PWM_LOW + brightness_step * (animation_step - one_pwm_pass_steps));
  }

  if (animation_step > one_pwm_pass_steps * 2) {
    set_all_pwm(PWM_HIGH);
    current_animation_start_time = millis();
  }
}

// 1.2 - down-top brightness decreasing
void update_animation_1_2() {
  int brightness_divisor = 1;

  turn_on_all_leds();

  for (int i = 1; i < LAYERS_COUNT + 1; i++) {
    current_pwm[i - 1] = PWM_HIGH / (i * brightness_divisor);
  }
}

// 1.3 - top-down brightness decreasing
void update_animation_1_3() {
  int brightness_divisor = 1;

  turn_on_all_leds();

  for (int i = LAYERS_COUNT; i >= 1; i--) {
    current_pwm[i - 1] = PWM_HIGH / ((LAYERS_COUNT - i + 1) * brightness_divisor);
  }
}

// 1.4 - edges-center brightness decreasing
void update_animation_1_4() {
  double brightness_divisor = 1.5;

  turn_on_all_leds();

  for (int i = 0; i < LAYERS_COUNT / 2; i++) {
    current_pwm[i] = PWM_HIGH / ((i + 1) * brightness_divisor);
    current_pwm[LAYERS_COUNT - i - 1] = PWM_HIGH / ((i + 1) * brightness_divisor);
  }
}

// 1.5 - cehter-edges brightness decreasing
void update_animation_1_5() {
  double brightness_divisor = 1.5;

  turn_on_all_leds();

  for (int i = 0; i < LAYERS_COUNT / 2; i++) {
    current_pwm[i] = PWM_HIGH / ((LAYERS_COUNT / 2 - i) * brightness_divisor);
    current_pwm[LAYERS_COUNT - i - 1] = PWM_HIGH / ((LAYERS_COUNT / 2 - i) * brightness_divisor);
  }
}

// 1.6 - opposite change in brightness of neighboring layers
void update_animation_1_6() {
  int brightness_step = 5;
  unsigned long one_brightness_level_duration = 100;
  unsigned long animation_duration = millis() - current_animation_start_time;
  unsigned long animation_step = animation_duration / one_brightness_level_duration;
  unsigned long one_pwm_pass_steps = PWM_HIGH / brightness_step;

  turn_on_all_leds();

  if (animation_step <= one_pwm_pass_steps) {
    for (int i = 0; i < LAYERS_COUNT; i++) {
      if (i % 2 == 0) {
        set_pwm_for_layer(PWM_HIGH - brightness_step * animation_step, i);
      } else {
        set_pwm_for_layer(PWM_LOW + brightness_step * animation_step, i);
      }
    }
  }

  if (animation_step > one_pwm_pass_steps && animation_step <= one_pwm_pass_steps * 2) {
    for (int i = 0; i < LAYERS_COUNT; i++) {
      if (i % 2 != 0) {
        set_pwm_for_layer(PWM_HIGH - brightness_step * (animation_step - one_pwm_pass_steps), i);
      } else {
        set_pwm_for_layer(PWM_LOW + brightness_step * (animation_step - one_pwm_pass_steps), i);
      }
    }
  }

  if (animation_step > one_pwm_pass_steps * 2) {
    current_animation_start_time = millis();
  }
}

// 2.0 - random LEDS
void update_animation_2_0() {

  unsigned long one_element_duration = 1000;
  unsigned long animation_duration = millis() - current_animation_start_time;

  if (animation_duration >= one_element_duration || first_anim_pass == 1) {
    clear_animation();

    for (int i = 0; i < LAYERS_COUNT; i++) {
      for (int j = 0; j < 2; j++) {
        current_animation[i][j] = (byte)random(0, 255);
      }
    }
    current_animation_start_time = millis();
    first_anim_pass = 0;
  }
}

// 2.1 - random symbols
void update_animation_2_1() {
  unsigned long one_element_duration = 1000;
  unsigned long animation_duration = millis() - current_animation_start_time;
  int random_symbol_index;
  byte bit = 0;

  if (animation_duration >= one_element_duration || first_anim_pass == 1) {
    clear_animation();
    random_symbol_index = random(0, SYMBOLS_COUNT);

    for (int i = 0; i < LAYERS_COUNT; i++) {
      for (int bit_pos = 0; bit_pos < BITS_FILLED; bit_pos++) {
        bit = bitRead(my_font[random_symbol_index][5 - i], bit_pos);
        if (bit == 1) {
          set_data_for_symbols(bit_pos, i);
        }
      }
    }
    current_animation_start_time = millis();
    first_anim_pass = 0;
  }
}

void show_numbers_for_clock(char *time_string_to_show, int pause) {
  byte bit = 0;
  unsigned long settings_separator_duration = 400;
  unsigned long one_letter_duration = 1500;
  unsigned long pause_duration = pause;
  unsigned long animation_duration = millis() - current_animation_start_time;
  unsigned long animation_step = animation_duration / one_letter_duration;
  int len = strlen(time_string_to_show);
  int current_symbol_index = my_font_symbol_pos(time_string_to_show[animation_step]);

  if (animation_step >= len) {
    current_animation_start_time = millis();
    return;
  }

  clear_animation();

  if (current_symbol_index == -1) {
    current_symbol_index = 41;  // * instead of unsupported symbols
  }

  if (first_anim_pass == 1 && animation_duration < settings_separator_duration) {
    current_symbol_index = 39;
  }

  if (animation_duration > settings_separator_duration) {
    first_anim_pass = 0;
  }

  if ((animation_step + 1) * one_letter_duration - animation_duration > pause_duration) {
    for (int i = 0; i < LAYERS_COUNT; i++) {
      for (int bit_pos = 0; bit_pos < BITS_FILLED; bit_pos++) {
        bit = bitRead(my_font[current_symbol_index][5 - i], bit_pos);
        if (bit == 1) {
          set_data_for_symbols(bit_pos, i);
        }
      }
    }
  } else {
    clear_animation();
  }
}

void update_clock_number() {
  sprintf(time_setting_to_show, "%d", time_setting);
}

// 3.0 - show time
void update_animation_3_0() {
  show_numbers_for_clock(time_to_show, 40);
}

// 3.1 - set/show first hours number
void update_animation_3_1() {
  update_clock_number();
  show_numbers_for_clock(time_setting_to_show, 0);
}

// 3.2 - set/show second hours number
void update_animation_3_2() {
  update_clock_number();
  show_numbers_for_clock(time_setting_to_show, 0);
}

// 3.3 - set/show first mins number
void update_animation_3_3() {
  update_clock_number();
  show_numbers_for_clock(time_setting_to_show, 0);
}

// 3.4 - set/show second mins number
void update_animation_3_4() {
  update_clock_number();
  show_numbers_for_clock(time_setting_to_show, 0);
}

void set_data_for_symbols(int bit_pos, int layer) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 2; j++) {
      current_animation[layer][j] = current_animation[layer][j] | columns[cols_diff_direction[current_cols_direction][bit_pos][i]][j];
    }
  }
}

// 4.0 - 4.7 - shows text according to current mode/submode from str_to_show array
void update_animation_4_0() {
  byte bit = 0;
  unsigned long one_letter_duration = 1500;
  unsigned long pause_duration = one_letter_duration / 30;
  unsigned long animation_duration = millis() - current_animation_start_time;
  unsigned long animation_step = animation_duration / one_letter_duration;
  int len = strlen(str_to_show[current_mode_submode.submode]);
  int current_symbol_index = my_font_symbol_pos(str_to_show[current_mode_submode.submode][animation_step]);

  if (animation_step >= len) {
    current_animation_start_time = millis();
    return;
  }

  clear_animation();

  if (current_symbol_index == -1) {
    current_symbol_index = 41;  // * instead of unsupported symbols
  }

  if ((animation_step + 1) * one_letter_duration - animation_duration > pause_duration) {
    for (int i = 0; i < LAYERS_COUNT; i++) {
      for (int bit_pos = 0; bit_pos < BITS_FILLED; bit_pos++) {
        bit = bitRead(my_font[current_symbol_index][5 - i], bit_pos);
        if (bit == 1) {
          set_data_for_symbols(bit_pos, i);
        }
      }
    }
  } else {
    clear_animation();
  }
}

// 5.0 - show columns
void update_animation_5_0() {
  unsigned long col_show_time = 1000;
  unsigned long animation_duration = millis() - current_animation_start_time;
  clear_animation();

  for (int col = 0; col < COLUMNS_COUNT; col++) {
    if (animation_duration > col_show_time * col && animation_duration < col_show_time * (col + 1)) {
      for (int i = 0; i < LAYERS_COUNT; i++) {
        for (int j = 0; j < 2; j++) {
          current_animation[i][j] = columns[col][j];
        }
      }
    }
  }

  if (animation_duration >= col_show_time * COLUMNS_COUNT) {
    current_animation_start_time = millis();
  }
}

// 5.1 - show horizontal layers
void update_animation_5_1() {
  unsigned long one_element_duration = 1000;
  unsigned long animation_duration = millis() - current_animation_start_time;
  clear_animation();

  for (int i = 0; i < LAYERS_COUNT; i++) {
    if (animation_duration >= one_element_duration * i && animation_duration < one_element_duration * (i + 1)) {
      for (int j = 0; j < 2; j++) {
        current_animation[i][j] = 0b11111111;
      }
    }
  }

  if (animation_duration >= one_element_duration * LAYERS_COUNT) {
    current_animation_start_time = millis();
  }
}

// 5.2 - show individual LEDs (voxels)
void update_animation_5_2() {
  unsigned long one_element_duration = 100;
  unsigned long one_layer_duration = one_element_duration * COLUMNS_COUNT;
  unsigned long animation_duration = millis() - current_animation_start_time;
  unsigned long animation_layer_step = animation_duration / one_layer_duration;
  unsigned long animation_step = (animation_duration / one_element_duration) - (COLUMNS_COUNT * animation_layer_step);

  if (animation_layer_step >= LAYERS_COUNT - 1 && animation_step >= COLUMNS_COUNT - 1) {
    current_animation_start_time = millis();
  }

  clear_animation();

  for (int i = 0; i < 2; i++) {
    current_animation[animation_layer_step][i] = columns[animation_step][i];
  }
}

// 5.3 - show vertical layers
void update_animation_5_3() {
  calculate_timings_and_set_vertical_plane(0);
}

// 5.4 - show vertical layers
void update_animation_5_4() {
  calculate_timings_and_set_vertical_plane(1);
}

// 5.5 - show vertical layers
void update_animation_5_5() {
  calculate_timings_and_set_vertical_plane(2);
}

// 5.6 - show vertical layers
void update_animation_5_6() {
  calculate_timings_and_set_vertical_plane(3);
}

void calculate_timings_and_set_vertical_plane(int direction) {
  unsigned long plane_show_time = 1000;
  unsigned long animation_duration = millis() - current_animation_start_time;

  clear_animation();

  set_vertical_plane(animation_duration, plane_show_time, direction);

  if (animation_duration > plane_show_time * VERTICAL_PLANES_COUNT) {
    current_animation_start_time = millis();
  }
}

void set_vertical_plane(unsigned long animation_duration, unsigned long plane_show_time, int direction) {
  for (int plane = 0; plane < VERTICAL_PLANES_COUNT; plane++) {
    if (animation_duration > plane_show_time * plane && animation_duration < plane_show_time * (plane + 1)) {
      for (int plane_element = 0; plane_element < ELEMENTS_IN_VERTICAL_PLANE_COUNT; plane_element++) {
        for (int i = 0; i < LAYERS_COUNT; i++) {
          for (int j = 0; j < 2; j++) {
            current_animation[i][j] = current_animation[i][j] | columns[cols_diff_direction[direction][plane][plane_element]][j];
          }
        }
      }
    }
  }
}

// 5.7 - show edges
void update_animation_5_7() {
  clear_animation();
  for (int col = 0; col < COLUMNS_COUNT; col++) {
    for (int i = 0; i < LAYERS_COUNT; i++) {
      for (int j = 0; j < 2; j++) {
        if (col == 0 || col == 3 || col == 12 || col == 15) {
          current_animation[i][j] = current_animation[i][j] | columns[col][j];
        }
        if (i == 0 || i == 5) {
          if (col == 1 || col == 2 || col == 4 || col == 7 || col == 8 || col == 11 || col == 13 || col == 14) {
            current_animation[i][j] = current_animation[i][j] | columns[col][j];
          }
        }
      }
    }
  }
}

// 5.8. measurement mode
void update_animation_5_8() {
  turn_on_all_leds();
  for (int i = 0; i < LAYERS_COUNT; i++) {
    if (i == 0) {
      ledcWrite(layers[i][1], PWM_HIGH);
    } else {
      ledcWrite(layers[i][1], PWM_LOW);
    }
  }
}

void off_all_layers() {
  for (int i = 0; i < LAYERS_COUNT; i++) {
    ledcWrite(layers[i][1], PWM_LOW);
  }
}

void clear_animation() {
  for (int i = 0; i < LAYERS_COUNT; i++) {
    for (int j = 0; j < 2; j++) {
      current_animation[i][j] = 0b00000000;
    }
  }
}

void change_symbol_orientation() {
  int temp_direction = current_cols_direction + 1;
  if (temp_direction == 4) temp_direction = 0;
  current_cols_direction = temp_direction;
  current_animation_start_time = millis();
}

mode_submode mode_submode_index() {
  mode_submode calculated_mode_submode;
  calculated_mode_submode.mode = -1;
  calculated_mode_submode.submode = -1;
  for (int i = 0; i < MODES_COUNT; i++) {
    for (int j = 0; j < SUBMODES_COUNT; j++)

      if (modes[i][j] == 1) {
        calculated_mode_submode.mode = i;
        calculated_mode_submode.submode = j;
        return calculated_mode_submode;
      }
  }
  return calculated_mode_submode;
}

void clear_and_set_mode_submode(int mode, int submode) {
  set_all_pwm(PWM_HIGH);
  for (int i = 0; i < MODES_COUNT; i++) {
    for (int j = 0; j < SUBMODES_COUNT; j++)
      modes[i][j] = 0;
  }
  modes[mode][submode] = 1;
}

void change_mode_submode(int mode, int submode) {
  clear_and_set_mode_submode(mode, submode);
  current_animation_start_time = millis();
}

void switch_submode(int direction, int current_mode, int current_submode) {
  int new_submode = current_submode + direction;

  if (new_submode < 0 || new_submode >= submodes_count_in_modes[current_mode]) {
    new_submode = 0;
  }
  change_mode_submode(current_mode, new_submode);
}

void change_brightness_level(int direction, int step) {
  int actual_change = step;
  int new_pwm = 0;

  for (int layer = 0; layer < LAYERS_COUNT; layer++) {
    if (current_pwm[layer] < step * 2) {
      actual_change = step / 10;
    }
    new_pwm = current_pwm[layer] + direction * actual_change;
    if (new_pwm <= PWM_HIGH && new_pwm >= PWM_LOW) {
      current_pwm[layer] = new_pwm;
    }
  }
}

void send_data_to_ICs(byte rws, byte cls) {
  digitalWrite(latchPin, LOW);
  MYshiftOut(dataPin, clockPin, LSBFIRST, rws);
  MYshiftOut(dataPin, clockPin, LSBFIRST, cls);
  digitalWrite(latchPin, HIGH);
}

// used instead of standard shiftOut because ESP32 is too fast and IC don't have
// enough time to receive data
// the code taken from here:
// https://forum.arduino.cc/t/74hc595-shift-register-adjustments-for-esp32/1164766/2
void MYshiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
  uint8_t i;

  digitalWrite(clockPin, LOW);

  for (i = 0; i < 8; i++) {
    if (bitOrder == LSBFIRST) {
      digitalWrite(dataPin, val & 1);
      val >>= 1;
    } else {
      digitalWrite(dataPin, (val & 128) != 0);
      val <<= 1;
    }
    //changed to 30
    delayMicroseconds(30);
    digitalWrite(clockPin, HIGH);
    delayMicroseconds(30);
    digitalWrite(clockPin, LOW);
  }
}
