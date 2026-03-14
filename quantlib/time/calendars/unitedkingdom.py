from __future__ import annotations

from enum import IntEnum

from quantlib.time.calendar import Calendar
from quantlib.time.date import Date
from quantlib.time.month import Month
from quantlib.time.weekday import Weekday


class UnitedKingdom(Calendar):
    class Market(IntEnum):
        Settlement = 0
        Exchange = 1
        Metals = 2

    class _SettlementImpl(Calendar.WesternImpl):
        def name(self) -> str:
            return "UK settlement"

        def isBusinessDay(self, date: Date) -> bool:
            w = date.weekday()
            d = date.dayOfMonth()
            dd = date.dayOfYear()
            m = date.month()
            y = date.year()
            em = self.easterMonday(y)

            if (
                self.isWeekend(w)
                # New Year's Day (possibly moved to Monday)
                or ((d == 1 or ((d == 2 or d == 3) and w == Weekday.Monday)) and m == Month.January)
                # Good Friday
                or (dd == em - 3)
                # Easter Monday
                or (dd == em)
                # first Monday of May (Early May Bank Holiday)
                or (d <= 7 and w == Weekday.Monday and m == Month.May)
                # last Monday of May (Spring Bank Holiday)
                or (d >= 25 and w == Weekday.Monday and m == Month.May and y != 2002 and y != 2012 and y != 2022)
                # Spring Bank Holiday special cases
                or ((d == 3 or d == 4) and m == Month.June and y == 2002)
                or ((d == 4 or d == 5) and m == Month.June and y == 2012)
                or ((d == 2 or d == 3) and m == Month.June and y == 2022)
                # Summer Bank Holiday (last Monday of August)
                or (d >= 25 and w == Weekday.Monday and m == Month.August)
                # September 19, 2022 - Queen Elizabeth II funeral
                or (d == 19 and m == Month.September and y == 2022)
                # Christmas (possibly moved to Monday or Tuesday)
                or ((d == 25 or (d == 27 and (w == Weekday.Monday or w == Weekday.Tuesday))) and m == Month.December)
                # Boxing Day (possibly moved to Monday or Tuesday)
                or ((d == 26 or (d == 28 and (w == Weekday.Monday or w == Weekday.Tuesday))) and m == Month.December)
                # May 8, 2020 - 75th anniversary of VE day
                or (d == 8 and m == Month.May and y == 2020)
                # May 8, 2023 - Coronation of King Charles III
                or (d == 8 and m == Month.May and y == 2023)
            ):
                return False
            return True

    def __init__(self, market: UnitedKingdom.Market = Market.Settlement):
        if market == UnitedKingdom.Market.Settlement:
            impl: Calendar.Impl = UnitedKingdom._SettlementImpl()
        else:
            raise ValueError(f"UK calendar market {market} not yet implemented")
        super().__init__(impl)
