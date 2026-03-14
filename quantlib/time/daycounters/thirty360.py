from __future__ import annotations

from enum import IntEnum

from quantlib.time.date import Date
from quantlib.time.daycounter import DayCounter


def _is_last_of_february(d: int, m: int, y: int) -> bool:
    return m == 2 and d == 28 + (1 if Date.isLeap(y) else 0)


class Thirty360(DayCounter):
    class Convention(IntEnum):
        USA = 0
        BondBasis = 1
        European = 2
        EurobondBasis = 2
        Italian = 3
        German = 4
        ISMA = 1
        ISDA = 4
        NASD = 5

    class _US_Impl(DayCounter.Impl):
        def name(self) -> str:
            return "30/360 (US)"

        def dayCount(self, d1: Date, d2: Date) -> int:
            dd1, dd2 = d1.dayOfMonth(), d2.dayOfMonth()
            mm1, mm2 = int(d1.month()), int(d2.month())
            yy1, yy2 = d1.year(), d2.year()

            if _is_last_of_february(dd1, mm1, yy1):
                if _is_last_of_february(dd2, mm2, yy2):
                    dd2 = 30
                dd1 = 30
            if dd2 == 31 and dd1 >= 30:
                dd2 = 30
            if dd1 == 31:
                dd1 = 30
            return 360 * (yy2 - yy1) + 30 * (mm2 - mm1) + (dd2 - dd1)

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return self.dayCount(d1, d2) / 360.0

    class _ISMA_Impl(DayCounter.Impl):
        def name(self) -> str:
            return "30/360 (Bond Basis)"

        def dayCount(self, d1: Date, d2: Date) -> int:
            dd1, dd2 = d1.dayOfMonth(), d2.dayOfMonth()
            mm1, mm2 = int(d1.month()), int(d2.month())
            yy1, yy2 = d1.year(), d2.year()

            if dd1 == 31:
                dd1 = 30
            if dd2 == 31 and dd1 == 30:
                dd2 = 30
            return 360 * (yy2 - yy1) + 30 * (mm2 - mm1) + (dd2 - dd1)

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return self.dayCount(d1, d2) / 360.0

    class _EU_Impl(DayCounter.Impl):
        def name(self) -> str:
            return "30E/360 (Eurobond Basis)"

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
            return self.dayCount(d1, d2) / 360.0

    class _IT_Impl(DayCounter.Impl):
        def name(self) -> str:
            return "30/360 (Italian)"

        def dayCount(self, d1: Date, d2: Date) -> int:
            dd1, dd2 = d1.dayOfMonth(), d2.dayOfMonth()
            mm1, mm2 = int(d1.month()), int(d2.month())
            yy1, yy2 = d1.year(), d2.year()

            if dd1 == 31:
                dd1 = 30
            if dd2 == 31:
                dd2 = 30
            if mm1 == 2 and dd1 > 27:
                dd1 = 30
            if mm2 == 2 and dd2 > 27:
                dd2 = 30
            return 360 * (yy2 - yy1) + 30 * (mm2 - mm1) + (dd2 - dd1)

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return self.dayCount(d1, d2) / 360.0

    class _ISDA_Impl(DayCounter.Impl):
        def __init__(self, terminationDate: Date):
            self._terminationDate = terminationDate

        def name(self) -> str:
            return "30E/360 (ISDA)"

        def dayCount(self, d1: Date, d2: Date) -> int:
            dd1, dd2 = d1.dayOfMonth(), d2.dayOfMonth()
            mm1, mm2 = int(d1.month()), int(d2.month())
            yy1, yy2 = d1.year(), d2.year()

            if dd1 == 31:
                dd1 = 30
            if dd2 == 31:
                dd2 = 30
            if _is_last_of_february(dd1, mm1, yy1):
                dd1 = 30
            if d2 != self._terminationDate and _is_last_of_february(dd2, mm2, yy2):
                dd2 = 30
            return 360 * (yy2 - yy1) + 30 * (mm2 - mm1) + (dd2 - dd1)

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return self.dayCount(d1, d2) / 360.0

    class _NASD_Impl(DayCounter.Impl):
        def name(self) -> str:
            return "30/360 (NASD)"

        def dayCount(self, d1: Date, d2: Date) -> int:
            dd1, dd2 = d1.dayOfMonth(), d2.dayOfMonth()
            mm1, mm2 = int(d1.month()), int(d2.month())
            yy1, yy2 = d1.year(), d2.year()

            if dd1 == 31:
                dd1 = 30
            if dd2 == 31 and dd1 >= 30:
                dd2 = 30
            if dd2 == 31 and dd1 < 30:
                dd2 = 1
                mm2 += 1
            return 360 * (yy2 - yy1) + 30 * (mm2 - mm1) + (dd2 - dd1)

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            return self.dayCount(d1, d2) / 360.0

    def __init__(self, convention: Thirty360.Convention = Convention.USA, terminationDate: Date | None = None):
        if convention == Thirty360.Convention.USA:
            impl: DayCounter.Impl = Thirty360._US_Impl()
        elif convention in (Thirty360.Convention.BondBasis, Thirty360.Convention.ISMA):
            impl = Thirty360._ISMA_Impl()
        elif convention in (Thirty360.Convention.European, Thirty360.Convention.EurobondBasis):
            impl = Thirty360._EU_Impl()
        elif convention == Thirty360.Convention.Italian:
            impl = Thirty360._IT_Impl()
        elif convention in (Thirty360.Convention.German, Thirty360.Convention.ISDA):
            impl = Thirty360._ISDA_Impl(terminationDate or Date())
        elif convention == Thirty360.Convention.NASD:
            impl = Thirty360._NASD_Impl()
        else:
            raise ValueError(f"unknown 30/360 convention: {convention}")
        super().__init__(impl)
