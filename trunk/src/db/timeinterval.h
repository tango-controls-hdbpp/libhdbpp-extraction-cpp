#ifndef TIMEINTERVAL_H
#define TIMEINTERVAL_H

#include <time.h>

#define MAXTIMESTAMPLEN 64

/** \brief This class stores a time interval, made up of a start and stop timestamp.
 *
 * This class stores a couple of date time quantities. They are internally stored
 * in a char buffer, as they are fetched from the database.
 *
 * Conversion utilities and different constructors are provided, in order to handle
 * the start and end of the time
 * interval as time_t, struct timeval and double (seconds.microseconds) representations.
 *
 */
class TimeInterval
{
public:
    TimeInterval();

    TimeInterval(const char *sta, const char *sto);

    TimeInterval(time_t start_tt, time_t stop_tt);

    TimeInterval(const struct timeval* start_tv, const struct timeval* stop_tv);

    TimeInterval(double start_d, double stop_d);

    TimeInterval(const TimeInterval& other);

    TimeInterval & operator=(const TimeInterval& other);

    bool operator==(const TimeInterval &other) const;

    bool operator!=(const TimeInterval &other) const;

    bool intersects(const TimeInterval &other) const;

    bool contains(const TimeInterval &other) const;

    time_t start_time_t() const;
    time_t stop_time_t() const;

    struct timeval start_timeval() const;
    struct timeval stop_timeval() const;

    const char *start() const;

    const char *stop() const;

private:
    char m_start[MAXTIMESTAMPLEN];
    char m_stop[MAXTIMESTAMPLEN];
};

#endif // TIMEINTERVAL_H
