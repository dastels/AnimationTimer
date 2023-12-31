#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <Bounce2.h>
#include "logging.h"
#include "serial_handler.h"
#include "null_handler.h"

// logger config
#define LOG_TO_SERIAL (true)
#define LOG_LEVEL ("DEBUG")

// fames/sec modes
uint8_t modes[] = {6, 12, 24, 30};
uint8_t number_of_modes = 4;
int mode;// default to 24 frames/sec
#define DEFAULT_MODE (2)

// state variables
uint8_t seconds;
uint8_t frames;
long time;
long frame_time;
long frame_start_time;

enum State {IDLE, MODE, RUN, LOWPOWER};
char *StateNames[] = {"IDLE", "MODE", "RUN", "LOWPOWER"};
enum State current_state;

long idle_timeout_start;
#define IDLE_TIMEOUT (60000)      // one minute lo power timeout when on battery

// button debouncers
Bounce run_button = Bounce();
Bounce mode_button = Bounce();

// low power flashing indicator
long low_power_indicator_toggle_time;          // time to toggle low power indicator
long low_power_toggle_on_time = 100;           // time the low power indicator is on
long low_power_toggle_off_time = 1000;          // time the low power indicator is off
bool low_power_indicator_state = false; // low power indicator state

// the display
Adafruit_7segment display = Adafruit_7segment();

Logger *logger;


boolean is_usb_connected(void)
{
     float val = (float(analogRead(A2)) / 1024.0) * 6.6;
     return val > 4.0;
}


bool initialize_serial()
{
     int position = 0;
     int delta = 1;
     long timeout = millis() + 2000;
     Serial.begin(115200);
     while (!Serial) {
          display.writeDigitRaw(position, 0b01000000);
          display.writeDisplay();
          delay(50);
          display.writeDigitRaw(position, 0b00000000);
          display.writeDisplay();
          position += delta;
          if (position == 2) {
               position += delta;
          } else if (position == 0 || position == 4) {
               delta *= -1;
          }
          if (millis() > timeout) {
               return false;
          }
     }
     return true;
}


void update_time_display(uint8_t s, uint8_t f)
{
     logger->debug("Updating display: %d.%d", s, f);

     display.writeDigitNum(0, s / 10, false);
     display.writeDigitNum(1, s % 10, true);
     display.drawColon(false);
     display.writeDigitNum(3, f / 10, false);
     display.writeDigitNum(4, f % 10, false);
     display.writeDisplay();
}


void update_frames_display()
{
     display.clear();
     uint8_t frames = modes[mode];
     if (frames > 9) {
          display.writeDigitNum(3, frames / 10, false);
     }
     display.writeDigitNum(4, frames % 10, false);
     display.writeDisplay();
}

void change_state(enum State new_state)
{
     current_state = new_state;
     logger->debug("State now %s", StateNames[new_state]);
}


void setup()
{
     // initialize the display
     display.begin(0x70);
     // set up logging
     LoggingHandler *handler = new NullHandler();
     if ( LOG_TO_SERIAL) {
          if (initialize_serial()) {
               handler = new SerialHandler();
          }
     }
     logger = Logger::get_logger(handler);
     logger->set_level(log_level_for(LOG_LEVEL));

     // set up button debouncers
     mode_button.attach(0, INPUT_PULLUP);
     run_button.attach(1, INPUT_PULLUP);
     //set state
     mode = DEFAULT_MODE;
     frame_time = 1000/modes[mode];
     change_state(IDLE);
     logger->debug("USB %s connected", is_usb_connected() ? "is" : "is not");
     idle_timeout_start = millis();
     if (modes[mode] > 9) {
          display.writeDigitNum(3, modes[mode] / 10, false);
     }
     display.writeDigitNum(4, modes[mode] % 10, false);
     display.writeDisplay();

}


void loop()
{
     run_button.update();
     mode_button.update();

     if (current_state == LOWPOWER) {
          if (mode_button.fell() || run_button.fell()) { // get out of low power mode
               update_frames_display();
               idle_timeout_start = millis();
               change_state(IDLE);
          } else if (millis() > low_power_indicator_toggle_time) { // toggle the indicator if it's time
               low_power_indicator_state = !low_power_indicator_state;
               display.writeDigitRaw(1, low_power_indicator_state ? 0b10000000 : 0b00000000);
               display.writeDisplay();
               low_power_indicator_toggle_time = millis() + (low_power_indicator_state ? low_power_toggle_on_time : low_power_toggle_off_time);
          }
     } else if (current_state == IDLE) {
          if (idle_timeout_start && (millis() > idle_timeout_start + IDLE_TIMEOUT) && !is_usb_connected()) {
               logger->debug("Idle timeout - turning off LEDs");
               display.clear();
               display.writeDigitRaw(1, 0b10000000);
               display.writeDisplay();
               low_power_indicator_toggle_time = millis() + low_power_toggle_on_time;
               low_power_indicator_state = true;
               change_state(LOWPOWER);
          } else if (mode_button.fell()) {
               idle_timeout_start = 0;
               logger->debug("Idle timeout disabled");
               change_state(MODE);
               mode++;
               mode %= number_of_modes;
               logger->debug("F/s now %d", modes[mode]);
               frame_time = 1000 / modes[mode];
               update_frames_display();
          } else if (run_button.fell()) {
               idle_timeout_start = 0;
               logger->debug("Idle timeout disabled");
               change_state(RUN);
               frame_start_time = millis();
               seconds = 0;
               frames = 0;
               update_time_display(seconds, frames);
          }
     } else if (current_state == MODE) {
          if (mode_button.rose()) {
               idle_timeout_start = millis();
               logger->debug("Idle timeout enabled");
               change_state(IDLE);
               // display.clear();
               // display.writeDisplay();
          }
     } else if (current_state == RUN) {
          if (!run_button.read()) { // run button held down
               if (millis() >= frame_start_time + frame_time) { // time to advance count?
                    frame_start_time = millis();
                    frames++;
                    if (frames == modes[mode]) {
                         frames = 0;
                         seconds++;
                    }
                    update_time_display(seconds, frames);
               }
          } else if (run_button.rose()) {
               idle_timeout_start = millis();
               logger->debug("Idle timeout enabled");
               change_state(IDLE);
          }
     }
}
