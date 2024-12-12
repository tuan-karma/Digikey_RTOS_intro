/**
 * ESP32 Multicore Spinlock Demo
 * 
 * Demonstration of crictical sections and ISRs with multicore processor
 */

#include <Arduino.h>

// Using dual-core of ESP32
static const BaseType_t pro_cpu = 1;
static const BaseType_t app_cpu = 1;

// Settings 
static const TickType_t time_hog = 1; // Time (ms) hogging the CPU task 1
static const TickType_t task_0_delay = 30; // Time (ms) Task 0 blocks itself
static const TickType_t task_1_delay = 100; // Time (ms) Task 1 blocks itself


