from quantlib.time.date import Date, daysBetween
from quantlib.time.daycounter import DayCounter


class Actual364(DayCounter):
    class _Impl(DayCounter.Impl):
        def name(self) -> str:
            return "Actual/364"

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return daysBetween(d1, d2) / 364.0

    def __init__(self):
        super().__init__(Actual364._Impl())
