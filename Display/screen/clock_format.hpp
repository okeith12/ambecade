#ifndef UI_CLOCK_FORMAT_HPP
#define UI_CLOCK_FORMAT_HPP

#include <cstdio>

namespace ui {
namespace clock_format {

// Weekday abbreviation for tm_wday (0 = Sunday .. 6 = Saturday).
inline const char* weekday(int wday)
{
    static const char* const names[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
    return (wday >= 0 && wday < 7) ? names[wday] : "---";
}

// Month abbreviation for tm_mon (0 = January .. 11 = December).
inline const char* month(int mon)
{
    static const char* const names[12] = {
        "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
    };
    return (mon >= 0 && mon < 12) ? names[mon] : "---";
}

// Writes a zero-padded 24-hour "HH:MM" into buf.
inline void hhmm(int hour, int minute, char* buf, int cap)
{
    std::snprintf(buf, cap, "%02d:%02d", hour, minute);
}

// Writes a "MON JUL 12" style date into buf.
inline void date_str(int wday, int mon, int mday, char* buf, int cap)
{
    std::snprintf(buf, cap, "%s %s %d", weekday(wday), month(mon), mday);
}

}  // namespace clock_format
}  // namespace ui

#endif  // UI_CLOCK_FORMAT_HPP
