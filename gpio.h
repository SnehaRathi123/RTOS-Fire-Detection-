#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define GPIO_BASE_PATH "/sys/class/gpio"
#define GPIO_DIRECTION_FILE "direction"
#define GPIO_VALUE_FILE "value"

// GPIO Pin Definitions
#define GPIO_PINS_COUNT 5
extern int gpio_pins[];  // Array of GPIO input pins (66, 67, 69, 68, 45)
extern int gpio_output_pin;  // GPIO pin for controlling buzzer (GPIO 44)

// Task priorities (higher number = higher priority)
#define HIGH_PRIORITY 90
#define MEDIUM_PRIORITY 60
#define LOW_PRIORITY 30

// GPIO Functions
/**
 * @brief Exports a GPIO pin for use.
 * 
 * @param pin GPIO pin number to export.
 * @return 0 on success, -1 on failure.
 */
int gpio_export(int pin);

/**
 * @brief Sets the direction of a GPIO pin (in or out).
 * 
 * @param pin GPIO pin number to set.
 * @param direction Direction of the pin: "in" or "out".
 * @return 0 on success, -1 on failure.
 */
int gpio_set_direction(int pin, const char *direction);

/**
 * @brief Reads the value (0 or 1) from a GPIO pin.
 * 
 * @param pin GPIO pin number to read.
 * @return Value of the pin (0 or 1), or -1 on failure.
 */
int gpio_read(int pin);

/**
 * @brief Writes a value (0 or 1) to a GPIO pin.
 * 
 * @param pin GPIO pin number to write to.
 * @param value Value to write (0 or 1).
 * @return 0 on success, -1 on failure.
 */
int gpio_write(int pin, int value);

// Thread Priority Function
/**
 * @brief Sets the priority for a given thread.
 * 
 * @param thread The thread to set the priority for.
 * @param priority The priority level to set for the thread.
 */
void set_thread_priority(pthread_t thread, int priority);

// Task Functions
/**
 * @brief Monitors GPIO pins for HIGH/LOW states.
 * This task has the highest priority.
 * 
 * @param arg Arguments passed to the task (not used).
 * @return NULL (end of task).
 */
void* sensor_monitoring_task(void* arg);

/**
 * @brief Controls the buzzer based on GPIO pin states.
 * This task has medium priority.
 * 
 * @param arg Arguments passed to the task (not used).
 * @return NULL (end of task).
 */
void* buzzer_control_task(void* arg);

/**
 * @brief Placeholder task for DHT22 sensor.
 * This task has the lowest priority and has no implementation for now.
 * 
 * @param arg Arguments passed to the task (not used).
 * @return NULL (end of task).
 */
void* dht22_task(void* arg);

// Main function to start the tasks and initialize GPIO
/**
 * @brief Main function to initialize the GPIO pins and start the tasks.
 * 
 * @return 0 on success, -1 on failure.
 */
int main(void);

#endif // GPIO_CONTROL_H
