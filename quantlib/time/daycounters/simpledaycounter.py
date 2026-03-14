from quantlib.time.date import Date, daysBetween
from quantlib.time.daycounter import DayCounter
from quantlib.time.month import Month


class SimpleDayCounter(DayCounter):
    class _Impl(DayCounter.Impl):
        def name(self) -> str:
            return "Simple"

        def dayCount(self, d1: Date, d2: Date) -> int:
            return d2.serialNumber() - d1.serialNumber()

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            day_count = daysBetween(d1, d2)
            if day_count == 0.0:
                return 0.0

            y1 = d1.year()
            y2 = d2.year()
            if y1 == y2:
                dib = 366.0 if Date.isLeap(y1) else 365.0
                return day_count / dib
            else:
                days_in_y1 = 366.0 if Date.isLeap(y1) else 365.0
                days_in_y2 = 366.0 if Date.isLeap(y2) else 365.0
                frac1 = daysBetween(d1, Date(1, Month.January, y1 + 1)) / days_in_y1
                frac2 = daysBetween(Date(1, Month.January, y2), d2) / days_in_y2
                return frac1 + frac2 + (y2 - y1 - 1)

    def __init__(self):
        super().__init__(SimpleDayCounter._Impl())
