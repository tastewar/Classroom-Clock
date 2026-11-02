#ifndef TIMEDEFS_H
#define TIMEDEFS_H

// Standard local time is GMT-6. DST adds one hour between the configured
// spring and fall dates. Each transition takes effect at midnight local time.
// Set these month/day values for each school year.
#define GMT_OFFSET_HOURS -6
#define DST_BEGIN_MONTH 3
#define DST_BEGIN_DAY 14
#define DST_END_MONTH 11
#define DST_END_DAY 1

// Default bell offset in seconds. A value saved through the serial menu
// overrides this default. Positive means the clock runs behind wall time.
#define BELL_SECONDS_OFFSET 16

#endif
