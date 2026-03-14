from quantlib.time.date import Date, daysBetween
from quantlib.time.daycounter import DayCounter


class Actual360(DayCounter):
    class _Impl(DayCounter.Impl):
        def __init__(self, includeLastDay: bool = False):
            self._includeLastDay = includeLastDay

        def name(self) -> str:
            return "Actual/360 (inc)" if self._includeLastDay else "Actual/360"

        def dayCount(self, d1: Date, d2: Date) -> int:
            return (d2.serialNumber() - d1.serialNumber()) + (1 if self._includeLastDay else 0)

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return (daysBetween(d1, d2) + (1.0 if self._includeLastDay else 0.0)) / 360.0

    def __init__(self, includeLastDay: bool = False):
        super().__init__(Actual360._Impl(includeLastDay))
