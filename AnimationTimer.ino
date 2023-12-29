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

enum State {NONE, MODE, RUN, LOWPOWER};
enum State current_state;

long idle_timeout_start;
#define IDLE_TIMEOUT (60000)      // one minute lo power timeout when on battery

// button debouncers
Bounce run_button = Bounce();
Bounce mode_button = Bounce();

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


void update_display(uint8_t s, uint8_t f)
{
     logger->debug("Updating display: %d.%d", s, f);

     display.writeDigitNum(0, s / 10, false);
     display.writeDigitNum(1, s % 10, true);
     display.drawColon(false);
     display.writeDigitNum(3, f / 10, false);
     display.writeDigitNum(4, f % 10, false);
     display.writeDisplay();
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
     current_state = NONE;
     logger->debug("State now NONE");
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

     if (current_state == NONE && idle_timeout_start && (millis() > idle_timeout_start + IDLE_TIMEOUT) && !is_usb_connected()) {
          logger->debug("Idle timeout - turning off LEDs");
          display.clear();
          display.writeDisplay();
          current_state = LOWPOWER;
          logger->debug("State now LOWPOWER");
     }

     // mode button pressed while in NONE state
     if ((current_state == NONE || current_state == LOWPOWER) && mode_button.fell()) {
          idle_timeout_start = 0;
          logger->debug("Idle timeout disabled");
          current_state = MODE;
          logger->debug("State now MODE");
          display.clear();
          mode++;
          mode %= number_of_modes;
          logger->debug("F/s now %d", modes[mode]);
          frame_time = 1000 / modes[mode];
          if (modes[mode] > 9) {
               display.writeDigitNum(3, modes[mode] / 10, false);
          }
          display.writeDigitNum(4, modes[mode] % 10, false);
          display.writeDisplay();
     }

     // mode_button released while in MODE state
     if (current_state == MODE && mode_button.rose()) {
          idle_timeout_start = millis();
          logger->debug("Idle timeout enabled");
          current_state = NONE;
          logger->debug("State now NONE");
          // display.clear();
          // display.writeDisplay();
     }

     // run button pressed while in NONE state
     if ((current_state == NONE || current_state == LOWPOWER) && run_button.fell()) { // run button first pressed
          idle_timeout_start = 0;
          logger->debug("Idle timeout disabled");
          current_state = RUN;
          logger->debug("State now RUN");
          frame_start_time = millis();
          seconds = 0;
          frames = 0;
          update_display(seconds, frames);
     }

     // run button held while in RUN state
     if (current_state == RUN && !run_button.read()) { // run button held down
          if (millis() >= frame_start_time + frame_time) { // time to advance count?
               frame_start_time = millis();
               frames++;
               if (frames == modes[mode]) {
                    frames = 0;
                    seconds++;
               }
               update_display(seconds, frames);
          }
     }

     // run button released while in RUN mode
     if (current_state == RUN && run_button.rose()) {
          idle_timeout_start = millis();
          logger->debug("Idle timeout enabled");
          current_state = NONE;
          logger->debug("State now NONE");
     }
}
