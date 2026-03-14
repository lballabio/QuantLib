from __future__ import annotations

from enum import IntEnum

from quantlib.time.date import Date, daysBetween
from quantlib.time.daycounter import DayCounter
from quantlib.time.month import Month
from quantlib.time.timeunit import TimeUnit


class ActualActual(DayCounter):
    class Convention(IntEnum):
        ISMA = 0
        Bond = 0
        ISDA = 1
        Historical = 1
        Actual365 = 1
        AFB = 2
        Euro = 2

    class _ISDA_Impl(DayCounter.Impl):
        def name(self) -> str:
            return "Actual/Actual (ISDA)"

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            if d1 == d2:
                return 0.0
            if d1 > d2:
                return -self.yearFraction(d2, d1, Date(), Date())

            y1 = d1.year()
            y2 = d2.year()
            dib1 = 366.0 if Date.isLeap(y1) else 365.0
            dib2 = 366.0 if Date.isLeap(y2) else 365.0

            total = float(y2 - y1 - 1)
            total += daysBetween(d1, Date(1, Month.January, y1 + 1)) / dib1
            total += daysBetween(Date(1, Month.January, y2), d2) / dib2
            return total

    class _Old_ISMA_Impl(DayCounter.Impl):
        def name(self) -> str:
            return "Actual/Actual (ISMA)"

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            if d1 == d2:
                return 0.0
            if d1 > d2:
                return -self.yearFraction(d2, d1, refPeriodStart, refPeriodEnd)

            ref_start = refPeriodStart if refPeriodStart and refPeriodStart.serialNumber() != 0 else d1
            ref_end = refPeriodEnd if refPeriodEnd and refPeriodEnd.serialNumber() != 0 else d2

            if ref_end <= ref_start or ref_end <= d1:
                raise ValueError(
                    f"invalid reference period: d1: {d1}, d2: {d2}, "
                    f"reference period start: {ref_start}, reference period end: {ref_end}"
                )

            months = round(12 * float(ref_end.serialNumber() - ref_start.serialNumber()) / 365)
            if months == 0:
                ref_start = d1
                ref_end = Date._advance(d1, 1, TimeUnit.Years)
                months = 12

            period = float(months) / 12.0

            if d2 <= ref_end:
                if d1 >= ref_start:
                    return period * daysBetween(d1, d2) / daysBetween(ref_start, ref_end)
                else:
                    previous_ref = Date._advance(ref_start, -months, TimeUnit.Months)
                    if d2 > ref_start:
                        return self.yearFraction(d1, ref_start, previous_ref, ref_start) + self.yearFraction(
                            ref_start, d2, ref_start, ref_end
                        )
                    else:
                        return self.yearFraction(d1, d2, previous_ref, ref_start)
            else:
                if ref_start > d1:
                    raise ValueError("invalid dates: d1 < refPeriodStart < refPeriodEnd < d2")

                total = self.yearFraction(d1, ref_end, ref_start, ref_end)
                i = 0
                while True:
                    new_ref_start = Date._advance(ref_end, months * i, TimeUnit.Months)
                    new_ref_end = Date._advance(ref_end, months * (i + 1), TimeUnit.Months)
                    if d2 < new_ref_end:
                        break
                    total += period
                    i += 1
                total += self.yearFraction(new_ref_start, d2, new_ref_start, new_ref_end)
                return total

    class _AFB_Impl(DayCounter.Impl):
        def name(self) -> str:
            return "Actual/Actual (AFB)"

        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float:
            if d1 == d2:
                return 0.0
            if d1 > d2:
                return -self.yearFraction(d2, d1, Date(), Date())

            new_d2: Date = d2
            total = 0.0
            temp: Date = d2
            while temp > d1:
                temp = Date._advance(new_d2, -1, TimeUnit.Years)
                if temp.dayOfMonth() == 28 and temp.month() == Month.February and Date.isLeap(temp.year()):
                    temp = Date(temp.serialNumber() + 1)
                if temp >= d1:
                    total += 1.0
                    new_d2 = temp

            den = 365.0
            if Date.isLeap(new_d2.year()):
                feb29 = Date(29, Month.February, new_d2.year())
                if new_d2 > feb29 and d1 <= feb29:
                    den += 1.0
            elif Date.isLeap(d1.year()):
                feb29 = Date(29, Month.February, d1.year())
                if new_d2 > feb29 and d1 <= feb29:
                    den += 1.0

            return total + daysBetween(d1, new_d2) / den

    def __init__(self, convention: ActualActual.Convention = Convention.ISDA):
        if convention in (ActualActual.Convention.ISDA,):
            impl: DayCounter.Impl = ActualActual._ISDA_Impl()
        elif convention in (ActualActual.Convention.ISMA,):
            impl = ActualActual._Old_ISMA_Impl()
        elif convention in (ActualActual.Convention.AFB,):
            impl = ActualActual._AFB_Impl()
        else:
            raise ValueError(f"unknown act/act convention: {convention}")
        super().__init__(impl)
