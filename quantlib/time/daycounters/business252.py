from __future__ import annotations

from quantlib.time.date import Date
from quantlib.time.daycounter import DayCounter
from quantlib.time.weekday import Weekday


class Business252(DayCounter):
    """Business/252 day counter.

    Counts business days (excluding weekends) between dates.
    When a Calendar class is available, this should accept a calendar
    to also exclude holidays. For now, only excludes weekends.
    """

    class _Impl(DayCounter.Impl):
        def name(self) -> str:
            return "Business/252"

        def dayCount(self, d1: Date, d2: Date) -> int:
            count = 0
            serial = d1.serialNumber()
            end = d2.serialNumber()
            while serial < end:
                serial += 1
                d = Date(serial)
                wd = d.weekday()
                if wd != Weekday.Saturday and wd != Weekday.Sunday:
                    count += 1
            return count

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return self.dayCount(d1, d2) / 252.0

    def __init__(self):
        super().__init__(Business252._Impl())
