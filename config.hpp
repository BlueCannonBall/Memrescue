#pragma once

// Logging
#define LOG_FILE "/dev/kmsg" // File path to write logs to

// Functionality
#define IOWAIT_SLEEP_TIME      500  // Amount of time in milliseconds to sleep before checking iowait again
#define MEMORY_SLEEP_TIME      100  // Amount of time in milliseconds to sleep before checking memory usage again
#define MAX_MEMORY_USAGE       0.95 // Maximum percent of memory being used allowed before acting
#define IOWAIT_THRES           0.25 // Maximum percent of CPU time dedicated to IO allowed before acting
#define OOM_KILLER_SAMPLE_SIZE 3    // The number of processes that are sampled at a time when killing processes

// Timers (in milliseconds)
#define CACHE_TIMEOUT 10000
#define SWAP_TIMEOUT  10000
