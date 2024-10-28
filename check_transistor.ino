//#include <U8g2lib.h>
//#include <SPI.h>
//#include <Wire.h>
#include "GyverButton.h"

//U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0);

#define BUTTON_PIN 2

#define TEST_PIN1 12
#define TEST_PIN2 11
#define TEST_PIN3 10

#define STR_SIZE 9

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

check_status_t check_field_effect_transistor(int gate, int source, int drain){
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
  //закрываем считываем 1 открываем считываем 2 сравниваем 1 и 2
  //check N channel
  reset_transistor();
  digitalWrite(gate, HIGH);
  digitalWrite(source, LOW);
  delay(10);
  digitalWrite(drain, HIGH);
  //digitalWrite(drain, LOW);
  gate_close_value = analogRead(analog_pin[source]);
  digitalWrite(gate, LOW);
  digitalWrite(source, HIGH);
  delay(10);
  digitalWrite(source, LOW);
  gate_open_value = analogRead(analog_pin[source]);
  if (gate_open_value < 1000) {//need to analog way to separate bipolar from fied transistor
    return CHECK_FAIL;
  }
  if (gate_open_value > gate_close_value && gate_open_value - gate_close_value > 100 && gate_close_value != 0) { 
    char debug_str[40] = "";
    sprintf(debug_str, "open=%d, close=%d",gate_open_value, gate_close_value);
    Serial.println(debug_str);
    sprintf(oled_str, "%c%c%c - N",test_pin1, test_pin2, test_pin3);
    return CHECK_OK;
  } else {
    //check P channel
    reset_transistor();
    digitalWrite(gate, LOW);
    digitalWrite(source, HIGH);
    delay(10);
    digitalWrite(drain, LOW);
    //digitalWrite(drain, HIGH);
    gate_close_value = analogRead(analog_pin[drain]);
    digitalWrite(gate, HIGH);
    digitalWrite(source, LOW);
    delay(10);
    digitalWrite(source, HIGH);
    gate_open_value = analogRead(analog_pin[drain]);
    if (gate_open_value < 1000) {//need to analog way to separate bipolar from fied transistor
      return CHECK_FAIL;
    }
    if (gate_open_value > gate_close_value && gate_open_value - gate_close_value > 100 && gate_close_value != 0) {
      char debug_str[40] = "";
      sprintf(debug_str, "open=%d, close=%d",gate_open_value, gate_close_value);
      Serial.println(debug_str);
      sprintf(oled_str, "%c%c%c - P",test_pin1, test_pin2, test_pin3);
      return CHECK_OK;
    }
  }
  return CHECK_FAIL;
}

check_status_t check_bipolar_effect_transistor(int emitter, int collector, int base){
  char test_pin1,test_pin2,test_pin3;
  int emitter_value = 0;
  int collector_value = 0;
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
  emitter_value = analogRead(analog_pin[collector]);
  collector_value = analogRead(analog_pin[emitter]);
  if (emitter_value > collector_value && emitter_value - collector_value > 50 && collector_value != 0) {
    sprintf(oled_str, "%c%c%c-pnp",test_pin1, test_pin2, test_pin3);
    return CHECK_OK;
  } else {
    //check npn
    digitalWrite(base, HIGH);
    digitalWrite(emitter, LOW);
    digitalWrite(collector, LOW);
    emitter_value = analogRead(analog_pin[collector]);
    collector_value = analogRead(analog_pin[emitter]);
    if (emitter_value > collector_value && emitter_value - collector_value > 50 && collector_value != 0) {
      sprintf(oled_str, "%c%c%c-npn",test_pin1, test_pin2, test_pin3);
      return CHECK_OK;
    }
  }
  return CHECK_FAIL;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TEST_PIN1, OUTPUT);
  pinMode(TEST_PIN2, OUTPUT);
  pinMode(TEST_PIN3, OUTPUT);
  btn.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  btn.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
  btn.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  btn.setType(HIGH_PULL);
  btn.setDirection(NORM_OPEN);

  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  //u8g2.begin();
  //u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
  //u8g2.drawStr(8,29,"MYBOTIC");	// write something to the internal memory
  //u8g2.sendBuffer();					// transfer internal memory to the display
}

void loop() {
  btn.tick();
  if (btn.isRelease()) {   
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("task execution...");
    if (check_field_effect_transistor(TEST_PIN1, TEST_PIN2, TEST_PIN3) == CHECK_FAIL){
      if (check_field_effect_transistor(TEST_PIN1, TEST_PIN3, TEST_PIN2) == CHECK_FAIL){
        if (check_field_effect_transistor(TEST_PIN2, TEST_PIN1, TEST_PIN3) == CHECK_FAIL){
          if (check_field_effect_transistor(TEST_PIN2, TEST_PIN3, TEST_PIN1) == CHECK_FAIL){
            if (check_field_effect_transistor(TEST_PIN3, TEST_PIN2, TEST_PIN1) == CHECK_FAIL){
              if (check_field_effect_transistor(TEST_PIN3, TEST_PIN1, TEST_PIN2) == CHECK_FAIL){
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
              }
            }
          }
        }
      }
    }
    reset_transistor();
    Serial.println(oled_str);
    //u8g2.drawStr(x,32,oled_str);
    //u8g2.sendBuffer();
    //delay(10);
    for (int i = 0; i < STR_SIZE; i++){
      oled_str[i] = ' ';
    }
    digitalWrite(LED_BUILTIN, LOW);


  }
}
