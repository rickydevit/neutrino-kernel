#include "time.h"

// === PRIVATE FUNCTIONS ========================

Date timestamp_to_date(Timestamp timestamp) {
    Date date = {};

    Day days = timestamp / SECONDS_PER_DAY;
    date.year = EPOCH_YEAR;
    date.month = 0;

    while (days - DAYS_PER_YEAR[IsLeapYear(date.year)] > 0) {
        days -= DAYS_PER_YEAR[IsLeapYear(date.year)];
        date.year++;
    }

    while (days - DAYS_PER_MONTH[IsLeapYear(date.year)][date.month] >= 0) {
        days -= DAYS_PER_MONTH[IsLeapYear(date.year)][date.month];
        date.month++;
    }

    date.day = days + 1;
    date.month++;

    return date;
}

Time timestamp_to_time(Timestamp timestamp) {
    return (Time){
        .second = (Seconds)(timestamp % 60),
        .minute = (Minutes)((timestamp / SECONDS_PER_MINUTE) % 60),
        .hour = (Hours)((timestamp / SECONDS_PER_HOUR) % 24)
    };
}

// === PUBLIC FUNCTIONS =========================

DateTime timestamp_to_datetime(Timestamp timestamp) {
    return (DateTime) {
        .date = timestamp_to_date(timestamp),
        .time = timestamp_to_time(timestamp)
    };
}

Timestamp datetime_to_timestamp(DateTime datetime) {
    Timestamp res = 0;

    for (Year year = EPOCH_YEAR; year < datetime.year; year++) 
        res += DAYS_PER_YEAR[IsLeapYear(year)] * SECONDS_PER_DAY;

    for (Month month = 0; month < datetime.month - 1; month++)
        res += DAYS_PER_MONTH[IsLeapYear(datetime.year)][month] * SECONDS_PER_DAY;

    res += (datetime.day - 1) * SECONDS_PER_DAY;
    res += datetime.hour * SECONDS_PER_HOUR;
    res += datetime.minute * SECONDS_PER_MINUTE;
    res += datetime.second;

    return res;
}
