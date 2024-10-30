#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include "GyverButton.h"

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

#define BUTTON_PIN 2

#define TEST_PIN1 12
#define TEST_PIN2 11
#define TEST_PIN3 10

#define STR_SIZE 9
#define MEAS_NUM 8
#define PIN_PERMUTATION_NUM 6
#define ANALOG_PIN_NUM 3

#define LOW_LIMIT 50
#define HIGH_LIMIT 1000

typedef enum check_status_e{
  CHECK_FAIL,
  CHECK_OK,
}check_status_t;

int buttonState = 0;
char oled_str[STR_SIZE] = "";
GButton btn(BUTTON_PIN);

static const int analog_pin[] = {0,0,0,0,0,0,0,0,0,2,1,0};

void reset_transistor(){
  digitalWrite(TEST_PIN1, LOW);
  digitalWrite(TEST_PIN2, LOW);
  digitalWrite(TEST_PIN3, LOW);
}

int p_field_transistor_test(int gate, int drain, int source){
  char test_pin1,test_pin2,test_pin3;
  int gate_close_value = 0;
  int gate_open_value = 0;
  if (drain > source && drain > gate){
    test_pin1 = 'S';//'D';
    if (source > gate){
      test_pin2 = 'D';//'S';
      test_pin3 = 'G';
    } else {
      test_pin2 = 'G';
      test_pin3 = 'D';//'S';
    }
  } else {
    if (source > drain && source > gate){
      test_pin1 = 'D';//'S';
      if (drain > gate){
        test_pin2 = 'S';//'D';
        test_pin3 = 'G';
      } else {
        test_pin2 = 'G';
        test_pin3 = 'S';//'D';
      }
    } else {
      if (gate > source && gate > drain){
        test_pin1 = 'G';
        if (drain > source){
          test_pin2 = 'S';//'D';
          test_pin3 = 'D';//'S';
        } else {
          test_pin2 = 'D';//'S';
          test_pin3 = 'S';//'D';
        }
      }
    }
  }
  //check P channel
  digitalWrite(gate, HIGH);
  digitalWrite(source, LOW);
  reset_transistor();
  digitalWrite(gate, LOW);
  digitalWrite(drain, HIGH);
  delay(10);
  digitalWrite(source, LOW);
  //digitalWrite(drain, HIGH);
  gate_close_value = analogRead(analog_pin[source]);
  digitalWrite(gate, HIGH);
  digitalWrite(drain, LOW);
  delay(10);
  digitalWrite(drain, HIGH);
  gate_open_value = analogRead(analog_pin[source]);
  char debug_str[40] = "";
  sprintf(debug_str, "open=%d, close=%d",gate_open_value, gate_close_value);
  Serial.println(debug_str);
  //if (gate_open_value > gate_close_value && gate_close_value > 50 && gate_close_value < 1000) {
  //if (gate_close_value > 50 && gate_close_value < 1000 && gate_open_value > 50 && gate_open_value > gate_close_value) {//&& gate_open_value < 1000
  if (gate_open_value > gate_close_value && gate_close_value > 50 && gate_open_value > 50){
    sprintf(oled_str, "%c%c%c - P", test_pin1, test_pin2, test_pin3);//test_pin1, test_pin2, test_pin3 -> wrong result. Need to swap
    return gate_close_value;
  }
  return 0;
}

int n_field_transistor_test(int gate, int drain, int source){
  char test_pin1,test_pin2,test_pin3;
  int gate_close_value = 0;
  int gate_open_value = 0;
  if (drain > source && drain > gate){
    test_pin1 = 'D';
    if (source > gate){
      test_pin2 = 'S';
      test_pin3 = 'G';
    } else {
      test_pin2 = 'G';
      test_pin3 = 'S';
    }
  } else {
    if (source > drain && source > gate){
      test_pin1 = 'S';
      if (drain > gate){
        test_pin2 = 'D';
        test_pin3 = 'G';
      } else {
        test_pin2 = 'G';
        test_pin3 = 'D';
      }
    } else {
      if (gate > source && gate > drain){
        test_pin1 = 'G';
        if (drain > source){
          test_pin2 = 'D';
          test_pin3 = 'S';
        } else {
          test_pin2 = 'S';
          test_pin3 = 'D';
        }
      }
    }
  }
  //check N channel
  digitalWrite(gate, LOW);
  digitalWrite(drain, HIGH);
  reset_transistor();
  digitalWrite(gate, HIGH);
  digitalWrite(source, LOW);
  delay(10);
  digitalWrite(drain, HIGH);
  gate_close_value = analogRead(analog_pin[source]);
  digitalWrite(gate, LOW);
  digitalWrite(source, HIGH);
  delay(10);
  digitalWrite(source, LOW);
  gate_open_value = analogRead(analog_pin[source]);
  char debug_str[40] = "";
  sprintf(debug_str, "open=%d, close=%d",gate_open_value, gate_close_value);
  Serial.println(debug_str);
  //if (gate_close_value > 50 && gate_close_value < 1000 && gate_open_value > 50 && gate_open_value < 1000 && gate_open_value > gate_close_value) { //not a safe condition: gate_close_value > 20 && gate_open_value < 1000    gate_open_value > gate_close_value &&
  if (gate_open_value > gate_close_value && gate_close_value > 50 && gate_open_value > 50){
    if (gate_open_value < 1000) {
      sprintf(oled_str, "%c%c%c - N",test_pin2, test_pin1, test_pin3);
    } else {
      sprintf(oled_str, "%c%c%c - N",test_pin1, test_pin2, test_pin3);
    }
    
    return gate_close_value;
  } 
  return 0;
}

check_status_t bipolar_transistor_test(int emitter, int base, int collector){
  char test_pin1,test_pin2,test_pin3;
  if (emitter > collector && emitter > base){
    test_pin1 = 'E';
    if (collector > base){
      test_pin2 = 'C';
      test_pin3 = 'B';
    } else {
      test_pin2 = 'B';
      test_pin3 = 'C';
    }
  } else {
    if (collector > emitter && collector > base){
      test_pin1 = 'C';
      if (emitter > base){
        test_pin2 = 'E';
        test_pin3 = 'B';
      } else {
        test_pin2 = 'B';
        test_pin3 = 'E';
      }
    } else {
      if (base > collector && base > emitter){
        test_pin1 = 'B';
        if (collector > emitter){
          test_pin2 = 'C';
          test_pin3 = 'E';
        } else {
          test_pin2 = 'E';
          test_pin3 = 'C';
        }
      }
    }
  }
  reset_transistor();
  //check pnp
  digitalWrite(base, LOW);
  digitalWrite(emitter, HIGH);
  digitalWrite(collector, HIGH);
  delay(10);
  int base_open_emitter_value = analogRead(analog_pin[base]);
  //digitalWrite(emitter, LOW);
  //digitalWrite(collector, HIGH);
  //delay(10);
  int base_open_collector_value = analogRead(analog_pin[base]);
  digitalWrite(base, HIGH);
  digitalWrite(emitter, LOW);
  digitalWrite(collector, LOW);
  delay(10);
  int base_close_emitter_value = analogRead(analog_pin[emitter]);
  int base_close_collector_value = analogRead(analog_pin[collector]);
  char debug_str[40] = "";
  sprintf(debug_str, "boE=%d, boC=%d, bcE=%d, bcC=%d,", base_open_emitter_value, base_open_collector_value, base_close_emitter_value, base_close_collector_value);
  Serial.println(debug_str);
  if (base_open_emitter_value > 700 &&  base_open_collector_value > 700 &&
      base_open_emitter_value < 1000 &&  base_open_collector_value < 1000 && 
      base_close_emitter_value < 50 && base_close_collector_value > 1000) {
    sprintf(oled_str, "%c%c%c-pnp",test_pin1, test_pin2, test_pin3);
    return CHECK_OK;
  } else {
    //check npn
    digitalWrite(base, HIGH);
    digitalWrite(emitter, LOW);
    digitalWrite(collector, LOW);
    base_open_emitter_value = analogRead(analog_pin[collector]);
    base_open_collector_value = analogRead(analog_pin[emitter]);
    digitalWrite(base, LOW);
    digitalWrite(emitter, HIGH);
    base_close_emitter_value = analogRead(analog_pin[base]);
    //digitalWrite(emitter, LOW);
    digitalWrite(collector, HIGH);
    base_close_collector_value = analogRead(analog_pin[base]);
    char debug_str[40] = "";
    sprintf(debug_str, "boE=%d, boC=%d, bcE=%d, bcC=%d,", base_open_emitter_value, base_open_collector_value, base_close_emitter_value, base_close_collector_value);
    Serial.println(debug_str);
    if (base_open_emitter_value - base_open_collector_value > 100 && 
        base_open_emitter_value > 50 &&  base_open_collector_value > 50 && 
        base_close_emitter_value > 1000 && base_close_collector_value > 1000) {
      sprintf(oled_str, "%c%c%c-npn",test_pin1, test_pin2, test_pin3);
      return CHECK_OK;
    }
  }
  return CHECK_FAIL;
}


//check_status_t check_transistor_effect(int pin1, int pin2, int pin3){
//  if (check_bipolar_effect_transistor(pin1, pin2, pin3) == CHECK_FAIL){
//    if (check_field_effect_transistor(pin1, pin2, pin3) == CHECK_FAIL){
//      return CHECK_FAIL;
//    }
//  }
//  return CHECK_OK;
//}

check_status_t check_field_effect_transistor(){
  static const struct{
    int test_pin1;
    int test_pin2;
    int test_pin3;
  }pin_perm[PIN_PERMUTATION_NUM] = {
    {TEST_PIN1, TEST_PIN2, TEST_PIN3},
    {TEST_PIN1, TEST_PIN3, TEST_PIN2},
    {TEST_PIN2, TEST_PIN1, TEST_PIN3},
    {TEST_PIN2, TEST_PIN3, TEST_PIN1},
    {TEST_PIN3, TEST_PIN2, TEST_PIN1},
    {TEST_PIN3, TEST_PIN1, TEST_PIN2}
  };
  int p_result_array[PIN_PERMUTATION_NUM] = {0};
  int n_result_array[PIN_PERMUTATION_NUM] = {0};
  int p_pin = 0;
  int n_pin = 0;
  for (int i = 0; i < PIN_PERMUTATION_NUM; i++) {
    p_result_array[i] = p_field_transistor_test(pin_perm[i].test_pin1, pin_perm[i].test_pin2, pin_perm[i].test_pin3);
    n_result_array[i] = n_field_transistor_test(pin_perm[i].test_pin1, pin_perm[i].test_pin2, pin_perm[i].test_pin3);
    char debug_str[40] = "";
    sprintf(debug_str, "%p_gc=%d, n_gc=%d", i, p_result_array[i], n_result_array[i]);
    Serial.println(debug_str);
  }
  int p_gate_close_value = 0;
  int n_gate_close_value = 0;
  for (int i = 0; i < PIN_PERMUTATION_NUM; i++){
    if(p_result_array[i] > 50 && p_result_array[i] < 1000){
      p_gate_close_value = p_result_array[i];
      p_pin = i;
    }
  }
  for (int i = 0; i < PIN_PERMUTATION_NUM; i++){
    if(n_result_array[i] > 50 && n_result_array[i] < 1000){
      n_gate_close_value = n_result_array[i];
      n_pin = i;
    }
  }
  if (p_gate_close_value < n_gate_close_value) {
    n_field_transistor_test(pin_perm[n_pin].test_pin1, pin_perm[n_pin].test_pin2, pin_perm[n_pin].test_pin3);
    return CHECK_OK;
  } else {
    p_field_transistor_test(pin_perm[p_pin].test_pin1, pin_perm[p_pin].test_pin2, pin_perm[p_pin].test_pin3);
    return CHECK_OK;
  }
  return CHECK_FAIL;
}

check_status_t check_bipolar_effect_transistor(){
  if (bipolar_transistor_test(TEST_PIN1, TEST_PIN2, TEST_PIN3) == CHECK_FAIL){
    if (bipolar_transistor_test(TEST_PIN1, TEST_PIN3, TEST_PIN2) == CHECK_FAIL){
      if (bipolar_transistor_test(TEST_PIN2, TEST_PIN1, TEST_PIN3) == CHECK_FAIL){
        if (bipolar_transistor_test(TEST_PIN2, TEST_PIN3, TEST_PIN1) == CHECK_FAIL){
          if (bipolar_transistor_test(TEST_PIN3, TEST_PIN2, TEST_PIN1) == CHECK_FAIL){
            if (bipolar_transistor_test(TEST_PIN3, TEST_PIN1, TEST_PIN2) == CHECK_FAIL){
              return CHECK_FAIL;
            }
          }
        }
      }
    }
  }
  return CHECK_OK;
}

check_status_t check_transistor_effect(){
  int result_array[MEAS_NUM][ANALOG_PIN_NUM] = {0};
  //int correct_value_num[MEAS_NUM] = {0};
  //bool correct_value_fl = false;
  for (int meas_i = 0; meas_i < MEAS_NUM; meas_i++) {
    int correct_value_num = 0;
    digitalWrite(TEST_PIN1, (meas_i & 1));
    digitalWrite(TEST_PIN2, (meas_i & 2) >> 1);
    digitalWrite(TEST_PIN3, (meas_i & 4) >> 2);
    delay(10);
    for (int analog_pin_i = 0; analog_pin_i < ANALOG_PIN_NUM; analog_pin_i++){
      result_array[meas_i][analog_pin_i] = analogRead(analog_pin_i);;
      if (result_array[meas_i][analog_pin_i] > LOW_LIMIT && result_array[meas_i][analog_pin_i] < HIGH_LIMIT) {
        correct_value_num++;
        //correct_value_num[meas_i]++;
        //correct_value_fl = true;
      }
    }
    //char debug_str[40] = "";
    //sprintf(debug_str, "%d 1pin=%d, 2pin=%d, 3pin=%d", meas_i, result_array[meas_i][0], result_array[meas_i][1], result_array[meas_i][2]);
    //Serial.println(debug_str);

    if (correct_value_num == 3) {
      /*
      if (check_bipolar_effect_transistor(TEST_PIN1, TEST_PIN2, TEST_PIN3) == CHECK_FAIL){
        if (check_bipolar_effect_transistor(TEST_PIN1, TEST_PIN3, TEST_PIN2) == CHECK_FAIL){
          if (check_bipolar_effect_transistor(TEST_PIN2, TEST_PIN1, TEST_PIN3) == CHECK_FAIL){
            if (check_bipolar_effect_transistor(TEST_PIN2, TEST_PIN3, TEST_PIN1) == CHECK_FAIL){
              if (check_bipolar_effect_transistor(TEST_PIN3, TEST_PIN2, TEST_PIN1) == CHECK_FAIL){
                if (check_bipolar_effect_transistor(TEST_PIN3, TEST_PIN1, TEST_PIN2) == CHECK_FAIL){
                  sprintf(oled_str, "fail");
                }
              }
            }
          }
        }
      }
      */
      return check_bipolar_effect_transistor();
    } else {
      if (correct_value_num == 2) {
        //сделать чтобы не прерывался тест если пройден. После идет сравнение gate_close_value. если тип P, но gate_close_value < чем в типе N, то это N
        /*
        if (check_field_effect_transistor(TEST_PIN1, TEST_PIN2, TEST_PIN3) == CHECK_FAIL){
          if (check_field_effect_transistor(TEST_PIN1, TEST_PIN3, TEST_PIN2) == CHECK_FAIL){
            if (check_field_effect_transistor(TEST_PIN2, TEST_PIN1, TEST_PIN3) == CHECK_FAIL){
              if (check_field_effect_transistor(TEST_PIN2, TEST_PIN3, TEST_PIN1) == CHECK_FAIL){
                if (check_field_effect_transistor(TEST_PIN3, TEST_PIN2, TEST_PIN1) == CHECK_FAIL){
                  if (check_field_effect_transistor(TEST_PIN3, TEST_PIN1, TEST_PIN2) == CHECK_FAIL){
                    sprintf(oled_str, "fail");
                  }
                }
              }
            }
          }
        }
        */
        return check_field_effect_transistor();
      }
    }
  }
  /*
  if (correct_value_fl) {
    for (int meas_i = 0; meas_i < MEAS_NUM; meas_i++) {
      if (correct_value_num[meas_i] == 1){
        return check_field_effect_transistor(meas_i);
      } 
      if (correct_value_num[meas_i] == 2) {
        return check_bipolar_effect_transistor(meas_i);
      }
    }
  }
  */
  sprintf(oled_str, "fail");
  return;
  //return CHECK_FAIL;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TEST_PIN1, OUTPUT);
  pinMode(TEST_PIN2, OUTPUT);
  pinMode(TEST_PIN3, OUTPUT);
  btn.setDebounce(50);
  btn.setTimeout(300);
  btn.setClickTimeout(600);
  btn.setType(HIGH_PULL);
  btn.setDirection(NORM_OPEN);

  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  u8g2.begin();
  u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(8,29,"MYBOTIC");
  u8g2.sendBuffer();
}

void loop() {
  btn.tick();
  if (btn.isRelease()) {   
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("task execution...");
    u8g2.clearBuffer(); 
    u8g2.drawStr(8,29,"testing");
    u8g2.sendBuffer();
    delay(10);

    //check_transistor_effect();
    if (check_transistor_effect() == CHECK_FAIL) {
      sprintf(oled_str, "fail");
    }
    //if (check_transistor_effect(TEST_PIN1, TEST_PIN2, TEST_PIN3) == CHECK_FAIL){
    //  if (check_transistor_effect(TEST_PIN1, TEST_PIN3, TEST_PIN2) == CHECK_FAIL){
    //    if (check_transistor_effect(TEST_PIN2, TEST_PIN1, TEST_PIN3) == CHECK_FAIL){
    //      if (check_transistor_effect(TEST_PIN2, TEST_PIN3, TEST_PIN1) == CHECK_FAIL){
    //        if (check_transistor_effect(TEST_PIN3, TEST_PIN2, TEST_PIN1) == CHECK_FAIL){
    //          if (check_transistor_effect(TEST_PIN3, TEST_PIN1, TEST_PIN2) == CHECK_FAIL){
    //            sprintf(oled_str, "fail");
    //          }
    //        }
    //      }
    //    }
    //  }
    //}

    //if (check_bipolar_effect_transistor(TEST_PIN1, TEST_PIN2, TEST_PIN3) == CHECK_FAIL){
    //  if (check_bipolar_effect_transistor(TEST_PIN1, TEST_PIN3, TEST_PIN2) == CHECK_FAIL){
    //    if (check_bipolar_effect_transistor(TEST_PIN2, TEST_PIN1, TEST_PIN3) == CHECK_FAIL){
    //      if (check_bipolar_effect_transistor(TEST_PIN2, TEST_PIN3, TEST_PIN1) == CHECK_FAIL){
    //        if (check_bipolar_effect_transistor(TEST_PIN3, TEST_PIN2, TEST_PIN1) == CHECK_FAIL){
    //          if (check_bipolar_effect_transistor(TEST_PIN3, TEST_PIN1, TEST_PIN2) == CHECK_FAIL){
    //            if (check_field_effect_transistor(TEST_PIN1, TEST_PIN2, TEST_PIN3) == CHECK_FAIL){
    //              if (check_field_effect_transistor(TEST_PIN1, TEST_PIN3, TEST_PIN2) == CHECK_FAIL){
    //                if (check_field_effect_transistor(TEST_PIN2, TEST_PIN1, TEST_PIN3) == CHECK_FAIL){
    //                  if (check_field_effect_transistor(TEST_PIN2, TEST_PIN3, TEST_PIN1) == CHECK_FAIL){
    //                    if (check_field_effect_transistor(TEST_PIN3, TEST_PIN2, TEST_PIN1) == CHECK_FAIL){
    //                      if (check_field_effect_transistor(TEST_PIN3, TEST_PIN1, TEST_PIN2) == CHECK_FAIL){
    //                        sprintf(oled_str, "fail");
    //                      }
    //                    }
    //                  }
    //                }
    //              }
    //            }
    //          }
    //        }
    //      }
    //    }
    //  }
    //}
    //reset_transistor();
    Serial.println(oled_str);
    u8g2.clearBuffer(); 
    u8g2.drawStr(0,29,oled_str);
    u8g2.sendBuffer();
    delay(10);
    for (int i = 0; i < STR_SIZE; i++){
      oled_str[i] = ' ';
    }
    digitalWrite(LED_BUILTIN, LOW);
  }
}
