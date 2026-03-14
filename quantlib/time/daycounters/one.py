from quantlib.time.date import Date
from quantlib.time.daycounter import DayCounter


class OneDayCounter(DayCounter):
    class _Impl(DayCounter.Impl):
        def name(self) -> str:
            return "1/1"

        def dayCount(self, d1: Date, d2: Date) -> int:
            return 1 if d1 != d2 else 0

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return 1.0 if d1 != d2 else 0.0

    def __init__(self):
        super().__init__(OneDayCounter._Impl())
