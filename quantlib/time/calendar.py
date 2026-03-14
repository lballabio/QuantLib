from __future__ import annotations

from abc import ABC, abstractmethod

from quantlib.time.businessdayconvention import BusinessDayConvention
from quantlib.time.date import Date
from quantlib.time.period import Period
from quantlib.time.timeunit import TimeUnit
from quantlib.time.weekday import Weekday


class Calendar:
    """Calendar base class using the Bridge pattern."""

    class Impl(ABC):
        def __init__(self) -> None:
            self.added_holidays: set[Date] = set()
            self.removed_holidays: set[Date] = set()

        @abstractmethod
        def name(self) -> str: ...

        @abstractmethod
        def isBusinessDay(self, d: Date) -> bool: ...

        @abstractmethod
        def isWeekend(self, w: Weekday) -> bool: ...

    class WesternImpl(Impl):
        """Partial implementation for Western calendars (Sat/Sun weekends, Easter)."""

        # fmt: off
        _EASTER_MONDAY = [
                  98,  90, 103,  95, 114, 106,  91, 111, 102,   # 1901-1909
             87, 107,  99,  83, 103,  95, 115,  99,  91, 111,   # 1910-1919
             96,  87, 107,  92, 112, 103,  95, 108, 100,  91,   # 1920-1929
            111,  96,  88, 107,  92, 112, 104,  88, 108, 100,   # 1930-1939
             85, 104,  96, 116, 101,  92, 112,  97,  89, 108,   # 1940-1949
            100,  85, 105,  96, 109, 101,  93, 112,  97,  89,   # 1950-1959
            109,  93, 113, 105,  90, 109, 101,  86, 106,  97,   # 1960-1969
             89, 102,  94, 113, 105,  90, 110, 101,  86, 106,   # 1970-1979
             98, 110, 102,  94, 114,  98,  90, 110,  95,  86,   # 1980-1989
            106,  91, 111, 102,  94, 107,  99,  90, 103,  95,   # 1990-1999
            115, 106,  91, 111, 103,  87, 107,  99,  84, 103,   # 2000-2009
             95, 115, 100,  91, 111,  96,  88, 107,  92, 112,   # 2010-2019
            104,  95, 108, 100,  92, 111,  96,  88, 108,  92,   # 2020-2029
            112, 104,  89, 108, 100,  85, 105,  96, 116, 101,   # 2030-2039
             93, 112,  97,  89, 109, 100,  85, 105,  97, 109,   # 2040-2049
            101,  93, 113,  97,  89, 109,  94, 113, 105,  90,   # 2050-2059
            110, 101,  86, 106,  98,  89, 102,  94, 114, 105,   # 2060-2069
             90, 110, 102,  86, 106,  98, 111, 102,  94, 114,   # 2070-2079
             99,  90, 110,  95,  87, 106,  91, 111, 103,  94,   # 2080-2089
            107,  99,  91, 103,  95, 115, 107,  91, 111, 103,   # 2090-2099
             88, 108, 100,  85, 105,  96, 109, 101,  93, 112,   # 2100-2109
             97,  89, 109,  93, 113, 105,  90, 109, 101,  86,   # 2110-2119
            106,  97,  89, 102,  94, 113, 105,  90, 110, 101,   # 2120-2129
             86, 106,  98, 110, 102,  94, 114,  98,  90, 110,   # 2130-2139
             95,  86, 106,  91, 111, 102,  94, 107,  99,  90,   # 2140-2149
            103,  95, 115, 106,  91, 111, 103,  87, 107,  99,   # 2150-2159
             84, 103,  95, 115, 100,  91, 111,  96,  88, 107,   # 2160-2169
             92, 112, 104,  95, 108, 100,  92, 111,  96,  88,   # 2170-2179
            108,  92, 112, 104,  89, 108, 100,  85, 105,  96,   # 2180-2189
            116, 101,  93, 112,  97,  89, 109, 100,  85, 105,   # 2190-2199
        ]
        # fmt: on

        def isWeekend(self, w: Weekday) -> bool:
            return w == Weekday.Saturday or w == Weekday.Sunday

        @staticmethod
        def easterMonday(year: int) -> int:
            """Day of year for Easter Monday."""
            return Calendar.WesternImpl._EASTER_MONDAY[year - 1901]

    class OrthodoxImpl(Impl):
        """Partial implementation for Orthodox calendars."""

        # fmt: off
        _EASTER_MONDAY = [
                 105, 118, 110, 102, 121, 106, 126, 118, 102,   # 1901-1909
            122, 114,  99, 118, 110,  95, 115, 106, 126, 111,   # 1910-1919
            103, 122, 107,  99, 119, 110, 123, 115, 107, 126,   # 1920-1929
            111, 103, 123, 107,  99, 119, 104, 123, 115, 100,   # 1930-1939
            120, 111,  96, 116, 108, 127, 112, 104, 124, 115,   # 1940-1949
            100, 120, 112,  96, 116, 108, 128, 112, 104, 124,   # 1950-1959
            109, 100, 120, 105, 125, 116, 101, 121, 113, 104,   # 1960-1969
            117, 109, 101, 120, 105, 125, 117, 101, 121, 113,   # 1970-1979
             98, 117, 109, 129, 114, 105, 125, 110, 102, 121,   # 1980-1989
            106,  98, 118, 109, 122, 114, 106, 118, 110, 102,   # 1990-1999
            122, 106, 126, 118, 103, 122, 114,  99, 119, 110,   # 2000-2009
             95, 115, 107, 126, 111, 103, 123, 107,  99, 119,   # 2010-2019
            111, 123, 115, 107, 127, 111, 103, 123, 108,  99,   # 2020-2029
            119, 104, 124, 115, 100, 120, 112,  96, 116, 108,   # 2030-2039
            128, 112, 104, 124, 116, 100, 120, 112,  97, 116,   # 2040-2049
            108, 128, 113, 104, 124, 109, 101, 120, 105, 125,   # 2050-2059
            117, 101, 121, 113, 105, 117, 109, 101, 121, 105,   # 2060-2069
            125, 110, 102, 121, 113,  98, 118, 109, 129, 114,   # 2070-2079
            106, 125, 110, 102, 122, 106,  98, 118, 110, 122,   # 2080-2089
            114,  99, 119, 110, 102, 115, 107, 126, 118, 103,   # 2090-2099
            123, 115, 100, 120, 112,  96, 116, 108, 128, 112,   # 2100-2109
            104, 124, 109, 100, 120, 105, 125, 116, 108, 121,   # 2110-2119
            113, 104, 124, 109, 101, 120, 105, 125, 117, 101,   # 2120-2129
            121, 113,  98, 117, 109, 129, 114, 105, 125, 110,   # 2130-2139
            102, 121, 113,  98, 118, 109, 129, 114, 106, 125,   # 2140-2149
            110, 102, 122, 106, 126, 118, 103, 122, 114,  99,   # 2150-2159
            119, 110, 102, 115, 107, 126, 111, 103, 123, 114,   # 2160-2169
             99, 119, 111, 130, 115, 107, 127, 111, 103, 123,   # 2170-2179
            108,  99, 119, 104, 124, 115, 100, 120, 112, 103,   # 2180-2189
            116, 108, 128, 119, 104, 124, 116, 100, 120, 112,   # 2190-2199
        ]
        # fmt: on

        def isWeekend(self, w: Weekday) -> bool:
            return w == Weekday.Saturday or w == Weekday.Sunday

        @staticmethod
        def easterMonday(year: int) -> int:
            return Calendar.OrthodoxImpl._EASTER_MONDAY[year - 1901]

    def __init__(self, impl: Calendar.Impl | None = None):
        self._impl = impl

    def empty(self) -> bool:
        return self._impl is None

    def name(self) -> str:
        if self._impl is None:
            raise RuntimeError("no calendar implementation provided")
        return self._impl.name()

    def isBusinessDay(self, d: Date) -> bool:
        if self._impl is None:
            raise RuntimeError("no calendar implementation provided")
        if d in self._impl.added_holidays:
            return False
        if d in self._impl.removed_holidays:
            return True
        return self._impl.isBusinessDay(d)

    def isHoliday(self, d: Date) -> bool:
        return not self.isBusinessDay(d)

    def isWeekend(self, w: Weekday) -> bool:
        if self._impl is None:
            raise RuntimeError("no calendar implementation provided")
        return self._impl.isWeekend(w)

    def isStartOfMonth(self, d: Date) -> bool:
        return d <= self.startOfMonth(d)

    def startOfMonth(self, d: Date) -> Date:
        return self.adjust(Date.startOfMonth(d), BusinessDayConvention.Following)

    def isEndOfMonth(self, d: Date) -> bool:
        return d >= self.endOfMonth(d)

    def endOfMonth(self, d: Date) -> Date:
        return self.adjust(Date.endOfMonth(d), BusinessDayConvention.Preceding)

    def addHoliday(self, d: Date) -> None:
        if self._impl is None:
            raise RuntimeError("no calendar implementation provided")
        self._impl.removed_holidays.discard(d)
        if self._impl.isBusinessDay(d):
            self._impl.added_holidays.add(d)

    def removeHoliday(self, d: Date) -> None:
        if self._impl is None:
            raise RuntimeError("no calendar implementation provided")
        self._impl.added_holidays.discard(d)
        if not self._impl.isBusinessDay(d):
            self._impl.removed_holidays.add(d)

    def resetAddedAndRemovedHolidays(self) -> None:
        if self._impl is not None:
            self._impl.added_holidays.clear()
            self._impl.removed_holidays.clear()

    def adjust(self, d: Date, c: BusinessDayConvention = BusinessDayConvention.Following) -> Date:
        if not d:
            raise ValueError("null date")
        if c == BusinessDayConvention.Unadjusted:
            return d

        d1 = Date(d.serialNumber())
        if c in (
            BusinessDayConvention.Following,
            BusinessDayConvention.ModifiedFollowing,
            BusinessDayConvention.HalfMonthModifiedFollowing,
        ):
            while self.isHoliday(d1):
                d1 = Date(d1.serialNumber() + 1)
            if c in (BusinessDayConvention.ModifiedFollowing, BusinessDayConvention.HalfMonthModifiedFollowing):
                if d1.month() != d.month():
                    return self.adjust(d, BusinessDayConvention.Preceding)
                if c == BusinessDayConvention.HalfMonthModifiedFollowing:
                    if d.dayOfMonth() <= 15 and d1.dayOfMonth() > 15:
                        return self.adjust(d, BusinessDayConvention.Preceding)
        elif c in (BusinessDayConvention.Preceding, BusinessDayConvention.ModifiedPreceding):
            while self.isHoliday(d1):
                d1 = Date(d1.serialNumber() - 1)
            if c == BusinessDayConvention.ModifiedPreceding and d1.month() != d.month():
                return self.adjust(d, BusinessDayConvention.Following)
        elif c == BusinessDayConvention.Nearest:
            d2 = Date(d.serialNumber())
            while self.isHoliday(d1) and self.isHoliday(d2):
                d1 = Date(d1.serialNumber() + 1)
                d2 = Date(d2.serialNumber() - 1)
            if self.isHoliday(d1):
                return d2
            return d1
        else:
            raise ValueError("unknown business-day convention")
        return d1

    def advance(
        self,
        d: Date,
        n: int | None = None,
        unit: TimeUnit | None = None,
        convention: BusinessDayConvention = BusinessDayConvention.Following,
        endOfMonth: bool = False,
        period: Period | None = None,
    ) -> Date:
        if not d:
            raise ValueError("null date")

        if period is not None:
            n = period.length()
            unit = period.units()

        if n is None or unit is None:
            raise ValueError("must provide either (n, unit) or period")

        if n == 0:
            return self.adjust(d, convention)

        if unit == TimeUnit.Days:
            d1 = Date(d.serialNumber())
            if n > 0:
                remaining = n
                while remaining > 0:
                    d1 = Date(d1.serialNumber() + 1)
                    while self.isHoliday(d1):
                        d1 = Date(d1.serialNumber() + 1)
                    remaining -= 1
            else:
                remaining = n
                while remaining < 0:
                    d1 = Date(d1.serialNumber() - 1)
                    while self.isHoliday(d1):
                        d1 = Date(d1.serialNumber() - 1)
                    remaining += 1
            return d1

        if unit == TimeUnit.Weeks:
            d1 = Date._advance(d, n * 7, TimeUnit.Days)
            return self.adjust(d1, convention)

        # Months or Years
        d1 = Date._advance(d, n, unit)
        if endOfMonth:
            if convention == BusinessDayConvention.Unadjusted:
                if Date.isEndOfMonth(d):
                    return Date.endOfMonth(d1)
            else:
                if self.isEndOfMonth(d):
                    return self.endOfMonth(d1)
        return self.adjust(d1, convention)

    def businessDaysBetween(
        self, from_date: Date, to_date: Date, includeFirst: bool = True, includeLast: bool = False
    ) -> int:
        if from_date == to_date:
            return int(includeFirst and includeLast and self.isBusinessDay(from_date))
        if from_date > to_date:
            return -self.businessDaysBetween(to_date, from_date, includeLast, includeFirst)

        count = int(includeLast and self.isBusinessDay(to_date))
        serial = from_date.serialNumber() if includeFirst else from_date.serialNumber() + 1
        end = to_date.serialNumber()
        while serial < end:
            if self.isBusinessDay(Date(serial)):
                count += 1
            serial += 1
        return count

    def holidayList(self, from_date: Date, to_date: Date, includeWeekEnds: bool = False) -> list[Date]:
        result = []
        serial = from_date.serialNumber()
        end = to_date.serialNumber()
        while serial <= end:
            d = Date(serial)
            if self.isHoliday(d) and (includeWeekEnds or not self.isWeekend(d.weekday())):
                result.append(d)
            serial += 1
        return result

    def businessDayList(self, from_date: Date, to_date: Date) -> list[Date]:
        result = []
        serial = from_date.serialNumber()
        end = to_date.serialNumber()
        while serial <= end:
            d = Date(serial)
            if self.isBusinessDay(d):
                result.append(d)
            serial += 1
        return result

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Calendar):
            return NotImplemented
        if self.empty() and other.empty():
            return True
        if self.empty() or other.empty():
            return False
        return self.name() == other.name()

    def __ne__(self, other: object) -> bool:
        if not isinstance(other, Calendar):
            return NotImplemented
        return not self.__eq__(other)

    def __str__(self) -> str:
        return self.name() if not self.empty() else "null calendar"
