from quantlib.time.date import Date
from quantlib.time.daycounter import DayCounter


class Thirty365(DayCounter):
    class _Impl(DayCounter.Impl):
        def name(self) -> str:
            return "30/365"

        def dayCount(self, d1: Date, d2: Date) -> int:
            dd1, dd2 = d1.dayOfMonth(), d2.dayOfMonth()
            mm1, mm2 = int(d1.month()), int(d2.month())
            yy1, yy2 = d1.year(), d2.year()

            if dd1 == 31:
                dd1 = 30
            if dd2 == 31:
                dd2 = 30
            return 360 * (yy2 - yy1) + 30 * (mm2 - mm1) + (dd2 - dd1)

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return self.dayCount(d1, d2) / 365.0

    def __init__(self):
        super().__init__(Thirty365._Impl())
