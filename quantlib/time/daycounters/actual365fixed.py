from __future__ import annotations

from enum import IntEnum

from quantlib.time.date import Date, daysBetween
from quantlib.time.daycounter import DayCounter
from quantlib.time.month import Month


class Actual365Fixed(DayCounter):
    class Convention(IntEnum):
        Standard = 0
        Canadian = 1
        NoLeap = 2

    class _Impl(DayCounter.Impl):
        def name(self) -> str:
            return "Actual/365 (Fixed)"

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return daysBetween(d1, d2) / 365.0

    class _CA_Impl(DayCounter.Impl):
        def name(self) -> str:
            return "Actual/365 (Fixed) Canadian Bond"

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            if d1 == d2:
                return 0.0
            if not refPeriodStart or not refPeriodEnd:
                raise ValueError("invalid refPeriodStart or refPeriodEnd")
            dcs = daysBetween(d1, d2)
            dcc = daysBetween(refPeriodStart, refPeriodEnd)
            months = round(12 * dcc / 365)
            if months == 0:
                raise ValueError("invalid reference period for Act/365 Canadian; must be longer than a month")
            frequency = 12 // months
            if frequency == 0:
                raise ValueError("invalid reference period for Act/365 Canadian; must not be longer than a year")
            if dcs < 365 / frequency:
                return dcs / 365.0
            return 1.0 / frequency - (dcc - dcs) / 365.0

    class _NL_Impl(DayCounter.Impl):
        _MONTH_OFFSET = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334]

        def name(self) -> str:
            return "Actual/365 (No Leap)"

        def dayCount(self, d1: Date, d2: Date) -> int:
            s1 = d1.dayOfMonth() + self._MONTH_OFFSET[int(d1.month()) - 1] + d1.year() * 365
            s2 = d2.dayOfMonth() + self._MONTH_OFFSET[int(d2.month()) - 1] + d2.year() * 365
            if d1.month() == Month.February and d1.dayOfMonth() == 29:
                s1 -= 1
            if d2.month() == Month.February and d2.dayOfMonth() == 29:
                s2 -= 1
            return s2 - s1

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return self.dayCount(d1, d2) / 365.0

    def __init__(self, convention: Actual365Fixed.Convention = Convention.Standard):
        if convention == Actual365Fixed.Convention.Standard:
            impl: DayCounter.Impl = Actual365Fixed._Impl()
        elif convention == Actual365Fixed.Convention.Canadian:
            impl = Actual365Fixed._CA_Impl()
        elif convention == Actual365Fixed.Convention.NoLeap:
            impl = Actual365Fixed._NL_Impl()
        else:
            raise ValueError(f"unknown Actual/365 (Fixed) convention: {convention}")
        super().__init__(impl)
