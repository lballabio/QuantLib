from __future__ import annotations

from quantlib.time.frequency import Frequency
from quantlib.time.timeunit import TimeUnit


class Period:
    def __init__(self, n: int | Frequency = 0, units: TimeUnit = TimeUnit.Days):
        if isinstance(n, Frequency):
            self._init_from_frequency(n)
        else:
            self._length = n
            self._units = units

    def _init_from_frequency(self, f: Frequency) -> None:
        if f == Frequency.NoFrequency:
            self._units = TimeUnit.Days
            self._length = 0
        elif f == Frequency.Once:
            self._units = TimeUnit.Years
            self._length = 0
        elif f == Frequency.Annual:
            self._units = TimeUnit.Years
            self._length = 1
        elif f in (
            Frequency.Semiannual,
            Frequency.EveryFourthMonth,
            Frequency.Quarterly,
            Frequency.Bimonthly,
            Frequency.Monthly,
        ):
            self._units = TimeUnit.Months
            self._length = 12 // int(f)
        elif f in (Frequency.EveryFourthWeek, Frequency.Biweekly, Frequency.Weekly):
            self._units = TimeUnit.Weeks
            self._length = 52 // int(f)
        elif f == Frequency.Daily:
            self._units = TimeUnit.Days
            self._length = 1
        elif f == Frequency.OtherFrequency:
            raise ValueError("unknown frequency")
        else:
            raise ValueError(f"unknown frequency ({int(f)})")

    def length(self) -> int:
        return self._length

    def units(self) -> TimeUnit:
        return self._units

    def frequency(self) -> Frequency:
        length = abs(self._length)

        if length == 0:
            if self._units == TimeUnit.Years:
                return Frequency.Once
            return Frequency.NoFrequency

        if self._units == TimeUnit.Years:
            return Frequency.Annual if length == 1 else Frequency.OtherFrequency
        elif self._units == TimeUnit.Months:
            if 12 % length == 0 and length <= 12:
                return Frequency(12 // length)
            return Frequency.OtherFrequency
        elif self._units == TimeUnit.Weeks:
            if length == 1:
                return Frequency.Weekly
            elif length == 2:
                return Frequency.Biweekly
            elif length == 4:
                return Frequency.EveryFourthWeek
            return Frequency.OtherFrequency
        elif self._units == TimeUnit.Days:
            return Frequency.Daily if length == 1 else Frequency.OtherFrequency
        else:
            raise ValueError(f"unknown time unit ({int(self._units)})")

    def normalize(self) -> None:
        if self._length == 0:
            self._units = TimeUnit.Days
        elif self._units == TimeUnit.Months:
            if self._length % 12 == 0:
                self._length //= 12
                self._units = TimeUnit.Years
        elif self._units == TimeUnit.Days:
            if self._length % 7 == 0:
                self._length //= 7
                self._units = TimeUnit.Weeks

    def normalized(self) -> Period:
        p = Period(self._length, self._units)
        p.normalize()
        return p

    def __iadd__(self, other: Period) -> Period:
        if self._length == 0:
            self._length = other._length
            self._units = other._units
        elif self._units == other._units:
            self._length += other._length
        else:
            self._iadd_different_units(other)
        return self

    def _iadd_different_units(self, other: Period) -> None:
        if self._units == TimeUnit.Years:
            if other._units == TimeUnit.Months:
                self._units = TimeUnit.Months
                self._length = self._length * 12 + other._length
            elif other._units in (TimeUnit.Weeks, TimeUnit.Days):
                if other._length != 0:
                    raise ValueError(f"impossible addition between {self} and {other}")
            else:
                raise ValueError(f"unknown time unit ({int(other._units)})")
        elif self._units == TimeUnit.Months:
            if other._units == TimeUnit.Years:
                self._length += other._length * 12
            elif other._units in (TimeUnit.Weeks, TimeUnit.Days):
                if other._length != 0:
                    raise ValueError(f"impossible addition between {self} and {other}")
            else:
                raise ValueError(f"unknown time unit ({int(other._units)})")
        elif self._units == TimeUnit.Weeks:
            if other._units == TimeUnit.Days:
                self._units = TimeUnit.Days
                self._length = self._length * 7 + other._length
            elif other._units in (TimeUnit.Years, TimeUnit.Months):
                if other._length != 0:
                    raise ValueError(f"impossible addition between {self} and {other}")
            else:
                raise ValueError(f"unknown time unit ({int(other._units)})")
        elif self._units == TimeUnit.Days:
            if other._units == TimeUnit.Weeks:
                self._length += other._length * 7
            elif other._units in (TimeUnit.Years, TimeUnit.Months):
                if other._length != 0:
                    raise ValueError(f"impossible addition between {self} and {other}")
            else:
                raise ValueError(f"unknown time unit ({int(other._units)})")
        else:
            raise ValueError(f"unknown time unit ({int(self._units)})")

    def __isub__(self, other: Period) -> Period:
        self += -other
        return self

    def __imul__(self, n: int) -> Period:
        self._length *= n
        return self

    def __itruediv__(self, n: int) -> Period:
        if n == 0:
            raise ValueError("cannot be divided by zero")
        if self._length % n == 0:
            self._length //= n
        else:
            units = self._units
            length = self._length
            if units == TimeUnit.Years:
                length *= 12
                units = TimeUnit.Months
            elif units == TimeUnit.Weeks:
                length *= 7
                units = TimeUnit.Days
            if length % n != 0:
                raise ValueError(f"{self} cannot be divided by {n}")
            self._length = length // n
            self._units = units
        return self

    def __neg__(self) -> Period:
        return Period(-self._length, self._units)

    def __add__(self, other: Period) -> Period:
        result = Period(self._length, self._units)
        result += other
        return result

    def __sub__(self, other: Period) -> Period:
        return self + (-other)

    def __mul__(self, n: int) -> Period:
        return Period(n * self._length, self._units)

    def __rmul__(self, n: int) -> Period:
        return Period(n * self._length, self._units)

    def __truediv__(self, n: int) -> Period:
        result = Period(self._length, self._units)
        result /= n
        return result

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Period):
            return NotImplemented
        return not (self < other or other < self)

    def __ne__(self, other: object) -> bool:
        if not isinstance(other, Period):
            return NotImplemented
        return not (self == other)

    def __lt__(self, other: Period) -> bool:
        # special cases
        if self._length == 0:
            return other._length > 0
        if other._length == 0:
            return self._length < 0

        # exact comparisons
        if self._units == other._units:
            return self._length < other._length
        if self._units == TimeUnit.Months and other._units == TimeUnit.Years:
            return self._length < 12 * other._length
        if self._units == TimeUnit.Years and other._units == TimeUnit.Months:
            return 12 * self._length < other._length
        if self._units == TimeUnit.Days and other._units == TimeUnit.Weeks:
            return self._length < 7 * other._length
        if self._units == TimeUnit.Weeks and other._units == TimeUnit.Days:
            return 7 * self._length < other._length

        # inexact comparisons
        p1_min, p1_max = _days_min_max(self)
        p2_min, p2_max = _days_min_max(other)

        if p1_max < p2_min:
            return True
        elif p1_min > p2_max:
            return False
        else:
            raise ValueError(f"undecidable comparison between {self} and {other}")

    def __le__(self, other: Period) -> bool:
        return not (other < self)

    def __gt__(self, other: Period) -> bool:
        return other < self

    def __ge__(self, other: Period) -> bool:
        return not (self < other)

    def __str__(self) -> str:
        _unit_short = {
            TimeUnit.Days: "D",
            TimeUnit.Weeks: "W",
            TimeUnit.Months: "M",
            TimeUnit.Years: "Y",
            TimeUnit.Hours: "h",
            TimeUnit.Minutes: "m",
            TimeUnit.Seconds: "s",
        }
        return f"{self._length}{_unit_short.get(self._units, '?')}"

    def __repr__(self) -> str:
        return f"Period({self._length}, {self._units!r})"


def _days_min_max(p: Period) -> tuple[int, int]:
    length = p.length()
    units = p.units()
    if units == TimeUnit.Days:
        return (length, length)
    elif units == TimeUnit.Weeks:
        return (7 * length, 7 * length)
    elif units == TimeUnit.Months:
        return (28 * length, 31 * length)
    elif units == TimeUnit.Years:
        return (365 * length, 366 * length)
    else:
        raise ValueError(f"unknown time unit ({int(units)})")


def years(p: Period) -> float:
    if p.length() == 0:
        return 0.0
    if p.units() == TimeUnit.Days:
        raise ValueError("cannot convert Days into Years")
    if p.units() == TimeUnit.Weeks:
        raise ValueError("cannot convert Weeks into Years")
    if p.units() == TimeUnit.Months:
        return p.length() / 12.0
    if p.units() == TimeUnit.Years:
        return float(p.length())
    raise ValueError(f"unknown time unit ({int(p.units())})")


def months(p: Period) -> float:
    if p.length() == 0:
        return 0.0
    if p.units() == TimeUnit.Days:
        raise ValueError("cannot convert Days into Months")
    if p.units() == TimeUnit.Weeks:
        raise ValueError("cannot convert Weeks into Months")
    if p.units() == TimeUnit.Months:
        return float(p.length())
    if p.units() == TimeUnit.Years:
        return p.length() * 12.0
    raise ValueError(f"unknown time unit ({int(p.units())})")


def weeks(p: Period) -> float:
    if p.length() == 0:
        return 0.0
    if p.units() == TimeUnit.Days:
        return p.length() / 7.0
    if p.units() == TimeUnit.Weeks:
        return float(p.length())
    if p.units() == TimeUnit.Months:
        raise ValueError("cannot convert Months into Weeks")
    if p.units() == TimeUnit.Years:
        raise ValueError("cannot convert Years into Weeks")
    raise ValueError(f"unknown time unit ({int(p.units())})")


def days(p: Period) -> float:
    if p.length() == 0:
        return 0.0
    if p.units() == TimeUnit.Days:
        return float(p.length())
    if p.units() == TimeUnit.Weeks:
        return p.length() * 7.0
    if p.units() == TimeUnit.Months:
        raise ValueError("cannot convert Months into Days")
    if p.units() == TimeUnit.Years:
        raise ValueError("cannot convert Years into Days")
    raise ValueError(f"unknown time unit ({int(p.units())})")
