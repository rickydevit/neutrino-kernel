#pragma once
#include <stdint.h>
#include <neutrino/macros.h>

#define SECONDS_PER_MINUTE  60
#define SECONDS_PER_HOUR    SECONDS_PER_MINUTE*60
#define SECONDS_PER_DAY     SECONDS_PER_HOUR*24

#define EPOCH_YEAR 1970
#define IsLeapYear(year) ((!((year) % 4) && (year) % 100) || !((year) % 400))

static const int DAYS_PER_YEAR[2] = {365, 366};
static const int DAYS_PER_MONTH[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
};

typedef uint64_t Timestamp;

typedef uint64_t Seconds;
typedef uint64_t Minutes;
typedef uint64_t Hours;

typedef uint64_t Day;
typedef uint64_t Month;
typedef uint64_t Year;

typedef struct __time {
    Seconds second;
    Minutes minute;
    Hours hour;
} packed Time;

typedef struct __date {
    Day day;
    Month month;
    Year year;
} packed Date;

typedef union __datetime {
    struct {
        Day day;
        Month month;
        Year year;
        Seconds second;
        Minutes minute;
        Hours hour; 
    };

    struct {
        Date date;
        Time time;
    };
} packed DateTime;

Timestamp datetime_to_timestamp(DateTime datetime);
DateTime timestamp_to_datetime(Timestamp timestamp);
