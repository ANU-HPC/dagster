// wallclock.h -- monotonic wall time in clock() units (CLOCKS_PER_SEC ticks/s).
// The master previously used clock(), i.e. its own CPU time. A master that sleeps while
// waiting for workers barely accumulates CPU time, so checkpoints (-v N) never fired and
// per-message statistics read near zero. Drop-in replacement for clock() in timing code.
#ifndef WALLCLOCK_H
#define WALLCLOCK_H

#include <ctime>

static inline clock_t wall_clock() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (clock_t)((long)ts.tv_sec * (long)CLOCKS_PER_SEC
                   + ts.tv_nsec / (1000000000L / (long)CLOCKS_PER_SEC));
}

#endif
