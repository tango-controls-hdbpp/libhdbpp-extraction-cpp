#include "datetimeutils.h"
#include <string.h>
#include <stdio.h>
#include <math.h> /* round */

#define TIMESTAMPLEN    32

DateTimeUtils::DateTimeUtils()
{
}

time_t DateTimeUtils::toTime_t(const char* timestamp_str) const
{
    struct tm mtm;
    memset(&mtm, 0, sizeof(struct tm));
    strptime(timestamp_str, "%Y-%m-%d %H:%M:%S", &mtm);
    return mktime(&mtm);
}

struct timeval DateTimeUtils::toTimeval(const char* timestamp_str) const
{
   // struct tm mtm;
    struct timeval tv;
    time_t t;
    struct tm *mtm;
    t = time(NULL);
    mtm = localtime(&t);
    // memset(&mtm, 0, sizeof(struct tm));
    // mtm.tm_isdst = 1;

    strptime(timestamp_str, "%Y-%m-%d %H:%M:%S", mtm);
    /* get usecs if specified */
    tv.tv_usec = 0;
    tv.tv_sec = mktime(mtm);
    return tv;
}

/** \brief Writes into dest (which must be previously allocated)
 *  the timestamp decoded from tv
 *
 * @param the timestamp represented by a timeval struct
 * @param the destination buffer, which must be a valid char, long enough to reasonably
 *        contain a timestamp. Its length must be specified in destlen
 * @param destlen the length of the dest buffer
 *
 * @return true if conversion was possible, false otherwise.
 */
bool DateTimeUtils::toString(const struct timeval* tv, char *dest, size_t destlen)
{
    int strfret;
    struct tm result;
    char timestamp[TIMESTAMPLEN];
    if(localtime_r(&(tv->tv_sec), &result) == NULL)
        return false;
    strfret = strftime(timestamp, TIMESTAMPLEN, "%Y-%m-%d %H:%M:%S", &result);
    /* concat milliseconds */
    snprintf(dest, destlen, "%s.%03.0f", timestamp, round(tv->tv_usec/1e3));
    return strfret != 0;
}

/** \brief Converts a string timestamp in the format YYYY-MM-dd hh:mm:ss to a double
 *         representing a unix timestamp in the format seconds.microseconds.
 *
 * @param timestamp_str a string  in the format "YYYY-MM-dd hh:mm:ss"
 *
 * @return a unix timestamp in the format seconds.microseconds.
 */
double DateTimeUtils::toDouble(const char *timestamp_str) const
{
    struct timeval tv = toTimeval(timestamp_str);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

/** \brief Writes into dest, which must be long at least destlen bytes, the timestamp decoded from
 *         the date/time represented by a double.
 *
 * @param tsmicro a double whose integer part is the UNIX timestamp and the decimal part is the milliseconds.
 * @param dest a char buffer long at least destlen
 * @param destlen the number of bytes available to save the timestamp.
 *
 * @see toString(const struct timeval* tv, char *dest, size_t destlen)
 *
 * @return true if no error occurred.
 */
bool DateTimeUtils::toString(double tsmicro, char *dest, size_t destlen)
{
    struct timeval tv;
    tv.tv_sec = (time_t) tsmicro;
    tv.tv_usec = (tsmicro - tv.tv_sec) * 1e6;
    return toString(&tv, dest, destlen);
}

/** \brief Writes into dest, which must be long at least destlen bytes, the timestamp decoded from
 *         the date/time represented by a time_t type.
 *
 * @param tp a time_t representing the UNIX timestamp.
 * @param dest a char buffer long at least destlen
 * @param destlen the number of bytes available to save the timestamp.
 *
 * @see toString(const struct timeval* tv, char *dest, size_t destlen)
 *
 * @return true if no error occurred.
 *
 * \note Milliseconds are set to 0.
 */
bool DateTimeUtils::toString(const time_t tp, char *dest, size_t destlen)
{
    double dtp = (double) tp;
    return toString(dtp, dest, destlen);
}


