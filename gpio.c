#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "gpio.h"
#define GPIO_PATH "/sys/class/gpio"

// Function to export a GPIO pin
int gpio_export(int pin) {
    int fd, len;
    char buf[64];

    fd = open(GPIO_BASE_PATH "/export", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open export for writing!");
        return -1;
    }

    len = snprintf(buf, sizeof(buf), "%d", pin);
    write(fd, buf, len);
    close(fd);

    return 0;
}

// Function to set the direction of a GPIO pin (in or out)
int gpio_set_direction(int pin, const char *direction) {
    while(gpio_export(pin) != 0) {
        sleep(1);
    }
    int fd;
    char buf[64];
    snprintf(buf, sizeof(buf), GPIO_BASE_PATH "/gpio%d/%s", pin, GPIO_DIRECTION_FILE);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
       // perror("Failed to open direction file for writing!");
        return -1;
    }

    write(fd, direction, strlen(direction));
    close(fd);

    return 0;
}

// Function to read a GPIO pin value (0 or 1)
int gpio_read(int pin) {
    int fd;
    char buf[64];
    char value;

    snprintf(buf, sizeof(buf), GPIO_BASE_PATH "/gpio%d/%s", pin, GPIO_VALUE_FILE);

    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open value file for reading!");
        return -1;
    }

    if (read(fd, &value, 1) != 1) {
        perror("Failed to read value!");
        close(fd);
        return -1;
    }

    close(fd);

    return (value == '1') ? 1 : 0;
}

// Function to write a value to a GPIO pin (0 or 1)
int gpio_write(int pin, int value) {
    FILE *fp;
    char path[100];
    snprintf(path, sizeof(path), GPIO_PATH "/gpio%d/value", pin);
    
    fp = fopen(path, "w");
    if (fp == NULL) {
        perror("Failed to write GPIO value");
        return -1;
    }
    fprintf(fp, "%d", value);
    (void)fclose(fp);
    return 0;
}
