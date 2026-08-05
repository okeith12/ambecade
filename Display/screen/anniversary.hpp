#ifndef UI_ANNIVERSARY_HPP
#define UI_ANNIVERSARY_HPP

#include <cstdio>
#include "clock_format.hpp"

namespace ui {
namespace anniversary {

/* Days since 1970-01-01 for a proleptic Gregorian date (Howard Hinnant's
   algorithm). Month is 1-12, day is 1-31. Pure integer math: leap years fall
   out correctly, so a countdown across a Feb 29 is exact. */
constexpr long days_from_civil(int y, unsigned m, unsigned d)
{
    y -= (m <= 2);
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153u * (m + (m > 2 ? -3 : 9)) + 2u) / 5u + d - 1u;
    const unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
    return static_cast<long>(era) * 146097L + static_cast<long>(doe) - 719468L;
}

/* Whole days from the current date to the next occurrence of a yearly
   anniversary (month 1-12, day 1-31). Returns 0 on the day itself, and rolls
   over to next year once the date has passed this year. */
inline int days_until(int cur_year, int cur_month, int cur_day,
                      int ann_month, int ann_day)
{
    const long today = days_from_civil(cur_year, static_cast<unsigned>(cur_month),
                                       static_cast<unsigned>(cur_day));
    long target = days_from_civil(cur_year, static_cast<unsigned>(ann_month),
                                  static_cast<unsigned>(ann_day));
    if (target < today) {
        target = days_from_civil(cur_year + 1, static_cast<unsigned>(ann_month),
                                 static_cast<unsigned>(ann_day));
    }
    return static_cast<int>(target - today);
}

// Writes the anniversary date as "AUG 16" into buf (month is 1-12).
inline void date_label(int month, int day, char* buf, int cap)
{
    std::snprintf(buf, static_cast<size_t>(cap), "%s %d",
                  ui::clock_format::month(month - 1), day);
}

// Writes the countdown as "TODAY", "1 DAY", or "N DAYS" into buf.
inline void countdown_label(int days, char* buf, int cap)
{
    if (days <= 0) {
        std::snprintf(buf, static_cast<size_t>(cap), "TODAY");
    } else if (days == 1) {
        std::snprintf(buf, static_cast<size_t>(cap), "1 DAY");
    } else {
        std::snprintf(buf, static_cast<size_t>(cap), "%d DAYS", days);
    }
}

}  // namespace anniversary
}  // namespace ui

#endif  // UI_ANNIVERSARY_HPP
