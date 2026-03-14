from __future__ import annotations

import datetime as _dt

from quantlib.time.month import Month
from quantlib.time.period import Period
from quantlib.time.timeunit import TimeUnit
from quantlib.time.weekday import Weekday


class Date:
    """Concrete date class storing an Excel/Applix serial number internally.

    Serial number 1 = January 1st, 1900. Valid range: [367, 109574],
    i.e., January 1st, 1901 to December 31st, 2199.
    """

    _MIN_SERIAL = 367  # Jan 1, 1901
    _MAX_SERIAL = 109574  # Dec 31, 2199

    _MONTH_LENGTH = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    _MONTH_LEAP_LENGTH = [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

    _MONTH_OFFSET = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365]
    _MONTH_LEAP_OFFSET = [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366]

    # fmt: off
    _YEAR_IS_LEAP = [
        # 1900 is leap in agreement with Excel's bug
        # 1900-1909
        True,False,False,False,True,False,False,False,True,False,
        # 1910-1919
        False,False,True,False,False,False,True,False,False,False,
        # 1920-1929
        True,False,False,False,True,False,False,False,True,False,
        # 1930-1939
        False,False,True,False,False,False,True,False,False,False,
        # 1940-1949
        True,False,False,False,True,False,False,False,True,False,
        # 1950-1959
        False,False,True,False,False,False,True,False,False,False,
        # 1960-1969
        True,False,False,False,True,False,False,False,True,False,
        # 1970-1979
        False,False,True,False,False,False,True,False,False,False,
        # 1980-1989
        True,False,False,False,True,False,False,False,True,False,
        # 1990-1999
        False,False,True,False,False,False,True,False,False,False,
        # 2000-2009
        True,False,False,False,True,False,False,False,True,False,
        # 2010-2019
        False,False,True,False,False,False,True,False,False,False,
        # 2020-2029
        True,False,False,False,True,False,False,False,True,False,
        # 2030-2039
        False,False,True,False,False,False,True,False,False,False,
        # 2040-2049
        True,False,False,False,True,False,False,False,True,False,
        # 2050-2059
        False,False,True,False,False,False,True,False,False,False,
        # 2060-2069
        True,False,False,False,True,False,False,False,True,False,
        # 2070-2079
        False,False,True,False,False,False,True,False,False,False,
        # 2080-2089
        True,False,False,False,True,False,False,False,True,False,
        # 2090-2099
        False,False,True,False,False,False,True,False,False,False,
        # 2100-2109
        False,False,False,False,True,False,False,False,True,False,
        # 2110-2119
        False,False,True,False,False,False,True,False,False,False,
        # 2120-2129
        True,False,False,False,True,False,False,False,True,False,
        # 2130-2139
        False,False,True,False,False,False,True,False,False,False,
        # 2140-2149
        True,False,False,False,True,False,False,False,True,False,
        # 2150-2159
        False,False,True,False,False,False,True,False,False,False,
        # 2160-2169
        True,False,False,False,True,False,False,False,True,False,
        # 2170-2179
        False,False,True,False,False,False,True,False,False,False,
        # 2180-2189
        True,False,False,False,True,False,False,False,True,False,
        # 2190-2199
        False,False,True,False,False,False,True,False,False,False,
        # 2200
        False,
    ]

    _YEAR_OFFSET = [
        # 1900-1909
        0,366,731,1096,1461,1827,2192,2557,2922,3288,
        # 1910-1919
        3653,4018,4383,4749,5114,5479,5844,6210,6575,6940,
        # 1920-1929
        7305,7671,8036,8401,8766,9132,9497,9862,10227,10593,
        # 1930-1939
        10958,11323,11688,12054,12419,12784,13149,13515,13880,14245,
        # 1940-1949
        14610,14976,15341,15706,16071,16437,16802,17167,17532,17898,
        # 1950-1959
        18263,18628,18993,19359,19724,20089,20454,20820,21185,21550,
        # 1960-1969
        21915,22281,22646,23011,23376,23742,24107,24472,24837,25203,
        # 1970-1979
        25568,25933,26298,26664,27029,27394,27759,28125,28490,28855,
        # 1980-1989
        29220,29586,29951,30316,30681,31047,31412,31777,32142,32508,
        # 1990-1999
        32873,33238,33603,33969,34334,34699,35064,35430,35795,36160,
        # 2000-2009
        36525,36891,37256,37621,37986,38352,38717,39082,39447,39813,
        # 2010-2019
        40178,40543,40908,41274,41639,42004,42369,42735,43100,43465,
        # 2020-2029
        43830,44196,44561,44926,45291,45657,46022,46387,46752,47118,
        # 2030-2039
        47483,47848,48213,48579,48944,49309,49674,50040,50405,50770,
        # 2040-2049
        51135,51501,51866,52231,52596,52962,53327,53692,54057,54423,
        # 2050-2059
        54788,55153,55518,55884,56249,56614,56979,57345,57710,58075,
        # 2060-2069
        58440,58806,59171,59536,59901,60267,60632,60997,61362,61728,
        # 2070-2079
        62093,62458,62823,63189,63554,63919,64284,64650,65015,65380,
        # 2080-2089
        65745,66111,66476,66841,67206,67572,67937,68302,68667,69033,
        # 2090-2099
        69398,69763,70128,70494,70859,71224,71589,71955,72320,72685,
        # 2100-2109
        73050,73415,73780,74145,74510,74876,75241,75606,75971,76337,
        # 2110-2119
        76702,77067,77432,77798,78163,78528,78893,79259,79624,79989,
        # 2120-2129
        80354,80720,81085,81450,81815,82181,82546,82911,83276,83642,
        # 2130-2139
        84007,84372,84737,85103,85468,85833,86198,86564,86929,87294,
        # 2140-2149
        87659,88025,88390,88755,89120,89486,89851,90216,90581,90947,
        # 2150-2159
        91312,91677,92042,92408,92773,93138,93503,93869,94234,94599,
        # 2160-2169
        94964,95330,95695,96060,96425,96791,97156,97521,97886,98252,
        # 2170-2179
        98617,98982,99347,99713,100078,100443,100808,101174,101539,101904,
        # 2180-2189
        102269,102635,103000,103365,103730,104096,104461,104826,105191,105557,
        # 2190-2199
        105922,106287,106652,107018,107383,107748,108113,108479,108844,109209,
        # 2200
        109574,
    ]
    # fmt: on

    def __init__(self, *args: int | Month):
        if len(args) == 0:
            self._serial = 0
        elif len(args) == 1:
            serial = int(args[0])
            if serial != 0:
                Date._check_serial_number(serial)
            self._serial = serial
        elif len(args) == 3:
            d, m, y = int(args[0]), int(args[1]), int(args[2])
            if y <= 1900 or y >= 2200:
                raise ValueError(f"year {y} out of bound. It must be in [1901,2199]")
            if m < 1 or m > 12:
                raise ValueError(f"month {m} outside January-December range [1,12]")
            leap = Date.isLeap(y)
            length = Date._month_length(m, leap)
            if d < 1 or d > length:
                raise ValueError(f"day outside month ({m}) day-range [1,{length}]")
            offset = Date._month_offset(m, leap)
            self._serial = d + offset + Date._year_offset(y)
        else:
            raise TypeError(f"Date() takes 0, 1, or 3 arguments ({len(args)} given)")

    def serialNumber(self) -> int:
        return self._serial

    def weekday(self) -> Weekday:
        w = self._serial % 7
        return Weekday(7 if w == 0 else w)

    def year(self) -> int:
        y = (self._serial // 365) + 1900
        if self._serial <= Date._year_offset(y):
            y -= 1
        return y

    def month(self) -> Month:
        d = self.dayOfYear()
        m = d // 30 + 1
        leap = Date.isLeap(self.year())
        while d <= Date._month_offset(m, leap):
            m -= 1
        while d > Date._month_offset(m + 1, leap):
            m += 1
        return Month(m)

    def dayOfMonth(self) -> int:
        return self.dayOfYear() - Date._month_offset(int(self.month()), Date.isLeap(self.year()))

    def dayOfYear(self) -> int:
        return self._serial - Date._year_offset(self.year())

    @staticmethod
    def isLeap(y: int) -> bool:
        if y < 1900 or y > 2200:
            raise ValueError("year outside valid range")
        return Date._YEAR_IS_LEAP[y - 1900]

    @staticmethod
    def minDate() -> Date:
        return Date(Date._MIN_SERIAL)

    @staticmethod
    def maxDate() -> Date:
        return Date(Date._MAX_SERIAL)

    @staticmethod
    def todaysDate() -> Date:
        today = _dt.date.today()
        return Date(today.day, Month(today.month), today.year)

    @staticmethod
    def _check_serial_number(serial: int) -> None:
        if serial < Date._MIN_SERIAL or serial > Date._MAX_SERIAL:
            raise ValueError(
                f"Date's serial number ({serial}) outside allowed range [{Date._MIN_SERIAL}-{Date._MAX_SERIAL}]"
            )

    @staticmethod
    def _month_length(m: int, leap: bool) -> int:
        return Date._MONTH_LEAP_LENGTH[m - 1] if leap else Date._MONTH_LENGTH[m - 1]

    @staticmethod
    def _month_offset(m: int, leap: bool) -> int:
        return Date._MONTH_LEAP_OFFSET[m - 1] if leap else Date._MONTH_OFFSET[m - 1]

    @staticmethod
    def _year_offset(y: int) -> int:
        return Date._YEAR_OFFSET[y - 1900]

    # Comparison operators
    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Date):
            return NotImplemented
        return self._serial == other._serial

    def __ne__(self, other: object) -> bool:
        if not isinstance(other, Date):
            return NotImplemented
        return self._serial != other._serial

    def __lt__(self, other: Date) -> bool:
        return self._serial < other._serial

    def __le__(self, other: Date) -> bool:
        return self._serial <= other._serial

    def __gt__(self, other: Date) -> bool:
        return self._serial > other._serial

    def __ge__(self, other: Date) -> bool:
        return self._serial >= other._serial

    # Date algebra
    def __add__(self, other: int | Period) -> Date:
        if isinstance(other, int):
            return Date(self._serial + other)
        if isinstance(other, Period):
            return Date._advance(self, other.length(), other.units())
        return NotImplemented

    def __radd__(self, other: int) -> Date:
        return self.__add__(other)

    def __sub__(self, other: int | Period | Date) -> Date | int:
        if isinstance(other, int):
            return Date(self._serial - other)
        if isinstance(other, Period):
            return Date._advance(self, -other.length(), other.units())
        if isinstance(other, Date):
            return self._serial - other._serial
        return NotImplemented

    def __iadd__(self, other: int | Period) -> Date:
        if isinstance(other, int):
            serial = self._serial + other
            Date._check_serial_number(serial)
            self._serial = serial
        elif isinstance(other, Period):
            self._serial = Date._advance(self, other.length(), other.units())._serial
        else:
            return NotImplemented
        return self

    def __isub__(self, other: int | Period) -> Date:
        if isinstance(other, int):
            serial = self._serial - other
            Date._check_serial_number(serial)
            self._serial = serial
        elif isinstance(other, Period):
            self._serial = Date._advance(self, -other.length(), other.units())._serial
        else:
            return NotImplemented
        return self

    @staticmethod
    def _advance(date: Date, n: int, units: TimeUnit) -> Date:
        if units == TimeUnit.Days:
            return Date(date._serial + n)
        elif units == TimeUnit.Weeks:
            return Date(date._serial + 7 * n)
        elif units == TimeUnit.Months:
            d = date.dayOfMonth()
            m = int(date.month()) + n
            y = date.year()
            while m > 12:
                m -= 12
                y += 1
            while m < 1:
                m += 12
                y -= 1
            if y < 1901 or y > 2199:
                raise ValueError(f"year {y} out of bounds. It must be in [1901,2199]")
            length = Date._month_length(m, Date.isLeap(y))
            if d > length:
                d = length
            return Date(d, Month(m), y)
        elif units == TimeUnit.Years:
            d = date.dayOfMonth()
            m = date.month()
            y = date.year() + n
            if y < 1901 or y > 2199:
                raise ValueError(f"year {y} out of bounds. It must be in [1901,2199]")
            if d == 29 and m == Month.February and not Date.isLeap(y):
                d = 28
            return Date(d, m, y)
        else:
            raise ValueError("undefined time units")

    @staticmethod
    def startOfMonth(d: Date) -> Date:
        return Date(1, d.month(), d.year())

    @staticmethod
    def isStartOfMonth(d: Date) -> bool:
        return d.dayOfMonth() == 1

    @staticmethod
    def endOfMonth(d: Date) -> Date:
        m = d.month()
        y = d.year()
        return Date(Date._month_length(int(m), Date.isLeap(y)), m, y)

    @staticmethod
    def isEndOfMonth(d: Date) -> bool:
        return d.dayOfMonth() == Date._month_length(int(d.month()), Date.isLeap(d.year()))

    @staticmethod
    def nextWeekday(d: Date, dayOfWeek: Weekday) -> Date:
        wd = int(d.weekday())
        dow = int(dayOfWeek)
        return d + ((7 if wd > dow else 0) - wd + dow)

    @staticmethod
    def nthWeekday(nth: int, dayOfWeek: Weekday, m: Month, y: int) -> Date:
        if nth < 1:
            raise ValueError("zeroth day of week in a given (month, year) is undefined")
        if nth > 5:
            raise ValueError("no more than 5 weekday in a given (month, year)")
        first = int(Date(1, m, y).weekday())
        dow = int(dayOfWeek)
        skip = nth - (1 if dow >= first else 0)
        return Date((1 + dow + skip * 7) - first, m, y)

    def __bool__(self) -> bool:
        return self._serial != 0

    def __hash__(self) -> int:
        return hash(self._serial)

    def __repr__(self) -> str:
        if self._serial == 0:
            return "Date()"
        return f"Date({self.dayOfMonth()}, {self.month()!r}, {self.year()})"

    def __str__(self) -> str:
        if self._serial == 0:
            return "null date"
        return f"{self.month().name} {self.dayOfMonth()}, {self.year()}"

    def iso_date(self) -> str:
        """ISO format: yyyy-mm-dd"""
        if self._serial == 0:
            return "null date"
        return f"{self.year()}-{int(self.month()):02d}-{self.dayOfMonth():02d}"

    def short_date(self) -> str:
        """Short format: mm/dd/yyyy"""
        if self._serial == 0:
            return "null date"
        return f"{int(self.month()):02d}/{self.dayOfMonth():02d}/{self.year()}"


def daysBetween(d1: Date, d2: Date) -> float:
    return float(d2.serialNumber() - d1.serialNumber())
