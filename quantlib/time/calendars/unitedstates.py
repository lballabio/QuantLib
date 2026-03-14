from __future__ import annotations

from enum import IntEnum

from quantlib.time.calendar import Calendar
from quantlib.time.date import Date
from quantlib.time.month import Month
from quantlib.time.weekday import Weekday


def _is_washington_birthday(d: int, m: int, y: int, w: Weekday) -> bool:
    if y >= 1971:
        return 15 <= d <= 21 and w == Weekday.Monday and m == int(Month.February)
    return (d == 22 or (d == 23 and w == Weekday.Monday) or (d == 21 and w == Weekday.Friday)) and m == int(
        Month.February
    )


def _is_memorial_day(d: int, m: int, y: int, w: Weekday) -> bool:
    if y >= 1971:
        return d >= 25 and w == Weekday.Monday and m == int(Month.May)
    return (d == 30 or (d == 31 and w == Weekday.Monday) or (d == 29 and w == Weekday.Friday)) and m == int(Month.May)


def _is_labor_day(d: int, m: int, w: Weekday) -> bool:
    return d <= 7 and w == Weekday.Monday and m == int(Month.September)


def _is_columbus_day(d: int, m: int, y: int, w: Weekday) -> bool:
    return 8 <= d <= 14 and w == Weekday.Monday and m == int(Month.October) and y >= 1971


def _is_veterans_day(d: int, m: int, y: int, w: Weekday) -> bool:
    if y <= 1970 or y >= 1978:
        return (d == 11 or (d == 12 and w == Weekday.Monday) or (d == 10 and w == Weekday.Friday)) and m == int(
            Month.November
        )
    return 22 <= d <= 28 and w == Weekday.Monday and m == int(Month.October)


def _is_juneteenth(d: int, m: int, y: int, w: Weekday, move_to_friday: bool = True) -> bool:
    return (
        (d == 19 or (d == 20 and w == Weekday.Monday) or (d == 18 and w == Weekday.Friday and move_to_friday))
        and m == int(Month.June)
        and y >= 2022
    )


class UnitedStates(Calendar):
    class Market(IntEnum):
        Settlement = 0
        NYSE = 1
        GovernmentBond = 2
        NERC = 3
        FederalReserve = 4

    class _SettlementImpl(Calendar.WesternImpl):
        def name(self) -> str:
            return "US settlement"

        def isBusinessDay(self, date: Date) -> bool:
            w = date.weekday()
            d = date.dayOfMonth()
            m = int(date.month())
            y = date.year()

            if (
                self.isWeekend(w)
                or ((d == 1 or (d == 2 and w == Weekday.Monday)) and m == int(Month.January))
                or (d == 31 and w == Weekday.Friday and m == int(Month.December))
                or (15 <= d <= 21 and w == Weekday.Monday and m == int(Month.January) and y >= 1983)
                or _is_washington_birthday(d, m, y, w)
                or _is_memorial_day(d, m, y, w)
                or _is_juneteenth(d, m, y, w)
                or (
                    (d == 4 or (d == 5 and w == Weekday.Monday) or (d == 3 and w == Weekday.Friday))
                    and m == int(Month.July)
                )
                or _is_labor_day(d, m, w)
                or _is_columbus_day(d, m, y, w)
                or _is_veterans_day(d, m, y, w)
                or (22 <= d <= 28 and w == Weekday.Thursday and m == int(Month.November))
                or (
                    (d == 25 or (d == 26 and w == Weekday.Monday) or (d == 24 and w == Weekday.Friday))
                    and m == int(Month.December)
                )
            ):
                return False
            return True

    class _NyseImpl(Calendar.WesternImpl):
        def name(self) -> str:
            return "New York stock exchange"

        def isBusinessDay(self, date: Date) -> bool:
            w = date.weekday()
            d = date.dayOfMonth()
            dd = date.dayOfYear()
            m = int(date.month())
            y = date.year()
            em = self.easterMonday(y)

            if (
                self.isWeekend(w)
                or ((d == 1 or (d == 2 and w == Weekday.Monday)) and m == int(Month.January))
                or _is_washington_birthday(d, m, y, w)
                or (dd == em - 3)  # Good Friday
                or _is_memorial_day(d, m, y, w)
                or _is_juneteenth(d, m, y, w)
                or (
                    (d == 4 or (d == 5 and w == Weekday.Monday) or (d == 3 and w == Weekday.Friday))
                    and m == int(Month.July)
                )
                or _is_labor_day(d, m, w)
                or (22 <= d <= 28 and w == Weekday.Thursday and m == int(Month.November))
                or (
                    (d == 25 or (d == 26 and w == Weekday.Monday) or (d == 24 and w == Weekday.Friday))
                    and m == int(Month.December)
                )
            ):
                return False

            # Martin Luther King's birthday (third Monday in January, since 1998 for NYSE)
            if 15 <= d <= 21 and w == Weekday.Monday and m == int(Month.January) and y >= 1998:
                return False

            return True

    def __init__(self, market: UnitedStates.Market = Market.Settlement):
        if market == UnitedStates.Market.Settlement:
            impl: Calendar.Impl = UnitedStates._SettlementImpl()
        elif market == UnitedStates.Market.NYSE:
            impl = UnitedStates._NyseImpl()
        else:
            raise ValueError(f"US calendar market {market} not yet implemented")
        super().__init__(impl)
