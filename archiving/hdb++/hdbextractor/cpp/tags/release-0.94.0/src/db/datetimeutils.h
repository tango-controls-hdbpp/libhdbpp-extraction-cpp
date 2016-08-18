#ifndef DATETIMEUTILS_H
#define DATETIMEUTILS_H

#include <sys/time.h>
#include <time.h>

/** \brief This class contains utility methods to convert different date/time representations
 *  among each other. char buffers, time_t timestamps, timestamps represented as double (seconds/milliseconds)
 *  and struct timeval representation are supported.
 *
 * The usage is simple. Instantiate a DateTimeUtils object and use its conversion utilities according to
 * your needs.
 */
class DateTimeUtils
{
public:
    DateTimeUtils();

    time_t toTime_t(const char* timestamp_str) const;

    struct timeval toTimeval(const char* timestamp_str) const;

    double toDouble(const char *timestamp_str) const;

    bool toString(const struct timeval* tv, char *dest, size_t destlen);

    bool toString(const time_t tp, char *dest, size_t destlen);

    bool toString(double tsmicro, char *dest, size_t destlen);
};

#endif // DATETIMEUTILS_H
