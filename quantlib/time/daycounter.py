from __future__ import annotations

from abc import ABC, abstractmethod

from quantlib.time.date import Date


class DayCounter:
    """Day counter base class using the Bridge pattern.

    Subclasses provide an _Impl that defines name(), dayCount(), and yearFraction().
    """

    class Impl(ABC):
        @abstractmethod
        def name(self) -> str: ...

        def dayCount(self, d1: Date, d2: Date) -> int:
            return d2.serialNumber() - d1.serialNumber()

        @abstractmethod
        def yearFraction(self, d1: Date, d2: Date, refPeriodStart: Date, refPeriodEnd: Date) -> float: ...

    def __init__(self, impl: DayCounter.Impl | None = None):
        self._impl = impl

    def empty(self) -> bool:
        return self._impl is None

    def name(self) -> str:
        if self._impl is None:
            raise RuntimeError("no day counter implementation provided")
        return self._impl.name()

    def dayCount(self, d1: Date, d2: Date) -> int:
        if self._impl is None:
            raise RuntimeError("no day counter implementation provided")
        return self._impl.dayCount(d1, d2)

    def yearFraction(
        self, d1: Date, d2: Date, refPeriodStart: Date | None = None, refPeriodEnd: Date | None = None
    ) -> float:
        if self._impl is None:
            raise RuntimeError("no day counter implementation provided")
        return self._impl.yearFraction(d1, d2, refPeriodStart or Date(), refPeriodEnd or Date())

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, DayCounter):
            return NotImplemented
        if self.empty() and other.empty():
            return True
        if self.empty() or other.empty():
            return False
        return self.name() == other.name()

    def __ne__(self, other: object) -> bool:
        if not isinstance(other, DayCounter):
            return NotImplemented
        return not self.__eq__(other)

    def __str__(self) -> str:
        return self.name() if not self.empty() else "null day counter"

    def __repr__(self) -> str:
        return f"DayCounter({self.name()!r})" if not self.empty() else "DayCounter()"
