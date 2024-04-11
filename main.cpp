#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "i2c_display.hpp" 
//defining pins
#define LED_PIN 0 
#define BUTTON_PIN 26 

class Counter_I2C {
public:
//methods
    Counter_I2C();
    void initialize();
    static void irqCallback(uint gpio, uint32_t events);
    void run();
    static unsigned int getCounter();

private:
//variables
    static unsigned int counter;
    static unsigned long lastPressTime;
    static bool counting; 
    static const unsigned long debounceTime;
};

//init variables
unsigned int Counter_I2C::counter = 0;
unsigned long Counter_I2C::lastPressTime = 0;
bool Counter_I2C::counting = false; 
const unsigned long Counter_I2C::debounceTime = 200;

Counter_I2C::Counter_I2C() {}

void Counter_I2C::initialize() {

    // init LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // init button 
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &Counter_I2C::irqCallback);

    // I2C display
    i2cdisplay::init(2, 3);
}
//debounce function for button
void Counter_I2C::irqCallback(uint gpio, uint32_t events) {
    unsigned long currentTime = to_ms_since_boot(get_absolute_time());
    if ((currentTime - lastPressTime) > debounceTime) {
        lastPressTime = currentTime;
        // toggle counting state
        counting = !counting; 
    }
}

void Counter_I2C::run() {
    while (true) {
        if (counting) {
            // Increment counter 
            counter++;
            i2cdisplay::clear();
            i2cdisplay::setCursor(0, 0);
            //displaying count
            i2cdisplay::print_character("Counting:");
            
            i2cdisplay::setCursor(1, 0);
            char buffer[16];
            //displaying seconds
            snprintf(buffer, sizeof(buffer), "%u seconds", counter);
            i2cdisplay::print_character(buffer);
            
            // Blink LED 
            gpio_put(LED_PIN, 1);
            sleep_ms(500);
            gpio_put(LED_PIN, 0);
            sleep_ms(500); 

        } else {
            //LED off when button toggle
            gpio_put(LED_PIN, 0);
            sleep_ms(100); 
        }
    }
}

unsigned int Counter_I2C::getCounter() {
    return counter;
}

int main() {
  //inst 
    Counter_I2C counter_i2c;
    counter_i2c.initialize();
    counter_i2c.run();
    return 0;
}
