#ifndef _DUCKS_DEADLINE_HPP_
#define _DUCKS_DEADLINE_HPP_

#include <stdint.h>
#include <stdlib.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <Windows.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#ifdef _WIN32
static inline int gettimeofday(struct timeval *tv,void *tz)
{
    // Define a structure to receive the current Windows filetime
    FILETIME ft;

    // Initialize the present time to 0 and the timezone to UTC
    unsigned __int64 tmpres = 0;
    static int tzflag = 0;

    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft);

        // The GetSystemTimeAsFileTime returns the number of 100 nanosecond
        // intervals since Jan 1, 1601 in a structure. Copy the high bits t
        // the 64 bit tmpres, shift it left by 32 then or in the low 32 bit
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        // Convert to microseconds by dividing by 10
        tmpres /= 10;
        // The Unix epoch starts on Jan 1 1970.  Need to subtract the diffe
        // in seconds from Jan 1 1601.
        tmpres -= DELTA_EPOCH_IN_MICROSECS;

        // Finally change microseconds to seconds and place in the seconds
        // The modulus picks up the microseconds.
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
    }
    return 0;
}
#endif

namespace ducks
{

///encapsulates a time
class Deadline
{
public:
    /// Creates a deadline pTime milliseconds into the future
    explicit Deadline(int64_t pTime=0)
        : mTime(now() + pTime)
    {
    }

    int64_t remainingMs() const
    {
        return mTime - now();
    }

private:
    /// returns a the current time in milliseconds since the epoch
    static int64_t now()
    {
        struct timeval lTime;
        gettimeofday(&lTime, NULL);
        return (int64_t(lTime.tv_sec) * 1000L + lTime.tv_usec / 1000);
    }

    int64_t mTime;
};

} /*namespace ducks*/

#endif
