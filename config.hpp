#pragma once

// Logging
#define LOG_FILE "/dev/kmsg" // File path to write logs to

// Functionality
#define SLEEP_TIME       500  // Amount of time in milliseconds to sleep before checking iowait again
#define MAX_MEMORY_USAGE 0.95 // Maximum percent of memory being used allowed before acting
#define IOWAIT_THRES     0.25 // Maximum percent of CPU time dedicated to IO allowed before acting

// Timers (in seconds)
#define CACHE_TIMEOUT 10
#define SWAP_TIMEOUT  10
#define KILL_TIMEOUT  5