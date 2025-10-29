#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>

#include "gpio.h"
#include "common_dht_read.h"

// GPIO Pin Definitions
#define GPIO_PINS_COUNT 5
int gpio_pins[] = {66, 67, 69, 68, 45};
int gpio_output_pin = 26;  // Pin for controlling the buzzer
float temperature, humidity;

// Task priorities (higher number = higher priority)
#define HIGH_PRIORITY 90
#define MEDIUM_PRIORITY 60
#define LOW_PRIORITY 30

// Task Function Declarations
void* sensor_monitoring_task(void* arg);
void* buzzer_control_task(void* arg);
void* dht22_task(void* arg);
void* display_task(void* arg);
void* control_task(void* arg);

// Global variables
bool any_pin_high = false; // Flag indicating if any GPIO pin is HIGH
bool fire_detected = false; // Flag for fire detection
bool terminate_program = false; // Signal termination flag

pthread_t sensor_thread, buzzer_thread, dht22_thread, display_thread, control_thread;

void cleanup() {
    printf("\nCleaning up resources...\n");
    gpio_write(gpio_output_pin, 0); // Ensure buzzer is off
    printf("Cleanup completed. Exiting program.\n");
}

void signal_handler(int signum) {
    printf("\nSignal %d received. Terminating program.\n", signum);
    terminate_program = true;
    pthread_cancel(sensor_thread);
    pthread_cancel(buzzer_thread);
    pthread_cancel(dht22_thread);
    pthread_cancel(display_thread);
    pthread_cancel(control_thread);
    cleanup();
    exit(0);
}

void read_dht(float *t, float *h) {
    int gpio_base = 1, gpio_number = 12;
    float hm=0, tm=0;

    while (hm == 0.0f || tm == 0.0f) {
        // Read sensor data for the correct value
        if (bbb_dht_read(22, gpio_base, gpio_number, &hm, &tm) == 0) {
            *t = tm;
            *h = hm;
        } else 
        {
            *t=-1;
            *h=-1;
        }

        sleep(1);
    }
}

// Function to set the thread priority
void set_thread_priority(pthread_t thread, int priority) {
    struct sched_param param;
    param.sched_priority = priority;

    int policy = SCHED_FIFO; // Real-time scheduling policy (First In, First Out)
    if (pthread_setschedparam(thread, policy, &param) != 0) {
        perror("Error setting thread priority");
    } else {
        printf("Thread priority set to %d\n", priority);
    }
}

// Task 1: Monitor GPIO pins
void* sensor_monitoring_task(void* arg) {
    (void)arg;
    while (!terminate_program) {
        any_pin_high = false;

        // Check the state of each GPIO pin
        for (int i = 0; i < GPIO_PINS_COUNT; i++) {
            int pin_state = gpio_read(gpio_pins[i]);
            if (pin_state == 1) {
                printf("GPIO%d is HIGH\n", gpio_pins[i]);
                any_pin_high = true;
                fire_detected = true;
            }
        }

        // Sleep for 1 second before checking again
        sleep(1);
    }

    return NULL;
}

// Task 2: Control Buzzer
void* buzzer_control_task(void* arg) {
    (void)arg;
    int status;
    while (!terminate_program) {
        if (fire_detected) {
            gpio_write(gpio_output_pin, 1); // Turn buzzer ON
            printf("Buzzer ON: Fire detected!\n");
            status =1;
        } else {
            if(status == 1)
            {
                gpio_write(gpio_output_pin, 0); // Turn buzzer OFF
                printf("Buzzer OFF: No fire detected.\n");
                status=0;
            }
        }
        sleep(1);
    }
    return NULL;
}

// Task 3: DHT22 Sensor Task
void* dht22_task(void* arg) {
    (void)arg;
    while (!terminate_program) {
        read_dht(&temperature, &humidity);
        printf("Temperature: %.2f, Humidity: %.2f\n", temperature, humidity);

        // If temperature exceeds threshold, set fire_detected
        if (temperature > 29.0) {
            fire_detected = true;
        } else {
            fire_detected = false;
        }

        sleep(5);
    }
    return NULL;
}

// Task 4: Display Task
void* display_task(void* arg) {
    (void)arg;
    printf("Displaying System Status:\n");
    while (!terminate_program) {
        
        printf("Temperature: %.2f\t", temperature);
        printf("Humidity: %.2f\t", humidity);
        printf("Fire Detected: %s\n", fire_detected ? "Yes" : "No");
        sleep(2);
    }
    return NULL;
}


// Test cases to simulate scenarios
void test_cases() {
    printf("\nRunning Test Cases:\n");

    // Simulate fire detection
    printf("\nTest Case 1: Simulate Fire Detection\n");
    fire_detected = true;
    sleep(3);

    // Simulate no fire detection
    printf("\nTest Case 2: Simulate No Fire\n");
    fire_detected = false;
    sleep(3);

    // Simulate high temperature
    printf("\nTest Case 3: Simulate High Temperature\n");
    temperature = 30.0;
    sleep(3);

    // Simulate normal temperature
    printf("\nTest Case 4: Simulate Normal Temperature\n");
    temperature = 25.0;
    sleep(3);

    printf("Test Cases Completed.\n\n");
}

// Main function
int main() {
    signal(SIGINT, signal_handler); // Register signal handler for termination

    for (int i = 0; i < GPIO_PINS_COUNT; i++) {
        while (gpio_set_direction(gpio_pins[i], "in") != 0);
    }

    while (gpio_set_direction(gpio_output_pin, "out") != 0);

    // Create threads for tasks
    if (pthread_create(&sensor_thread, NULL, sensor_monitoring_task, NULL) != 0) {
        perror("Failed to create sensor monitoring thread");
        return -1;
    }
    set_thread_priority(sensor_thread, MEDIUM_PRIORITY);

    if (pthread_create(&buzzer_thread, NULL, buzzer_control_task, NULL) != 0) {
        perror("Failed to create buzzer control thread");
        return -1;
    }
    set_thread_priority(buzzer_thread, LOW_PRIORITY);

    if (pthread_create(&dht22_thread, NULL, dht22_task, NULL) != 0) {
        perror("Failed to create DHT22 thread");
        return -1;
    }
    set_thread_priority(dht22_thread, HIGH_PRIORITY);

    if (pthread_create(&display_thread, NULL, display_task, NULL) != 0) {
        perror("Failed to create display thread");
        return -1;
    }
    set_thread_priority(display_thread, LOW_PRIORITY);

    // Run test cases
    //test_cases();
    
    // while (!terminate_program) {
    //     read_dht(&temperature, &humidity);
    //     printf("Temperature: %.2f, Humidity: %.2f\n", temperature, humidity);

    //     // If temperature exceeds threshold, set fire_detected
    //     if (temperature > 29.0) {
    //         fire_detected = true;
    //     } else {
    //         fire_detected = false;
    //     }

    //     sleep(5);
    // }
    // Join threads to prevent main from exiting prematurely
    pthread_join(sensor_thread, NULL);
    pthread_join(buzzer_thread, NULL);
    pthread_join(dht22_thread, NULL);
    pthread_join(display_thread, NULL);


    cleanup();
    return 0;
}
