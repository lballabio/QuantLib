from __future__ import annotations

from quantlib.time.businessdayconvention import BusinessDayConvention
from quantlib.time.calendar import Calendar
from quantlib.time.calendars.nullcalendar import NullCalendar
from quantlib.time.date import Date
from quantlib.time.dategenerationrule import DateGeneration
from quantlib.time.frequency import Frequency
from quantlib.time.period import Period
from quantlib.time.timeunit import TimeUnit
from quantlib.time.weekday import Weekday


def allowsEndOfMonth(tenor: Period) -> bool:
    return (tenor.units() == TimeUnit.Months or tenor.units() == TimeUnit.Years) and tenor >= Period(1, TimeUnit.Months)


class Schedule:
    def __init__(
        self,
        dates: list[Date] | None = None,
        calendar: Calendar | None = None,
        convention: BusinessDayConvention = BusinessDayConvention.Unadjusted,
        terminationDateConvention: BusinessDayConvention | None = None,
        tenor: Period | None = None,
        rule: DateGeneration.Rule | None = None,
        endOfMonth: bool | None = None,
        isRegular: list[bool] | None = None,
        # Rule-based constructor args
        effectiveDate: Date | None = None,
        terminationDate: Date | None = None,
        firstDate: Date | None = None,
        nextToLastDate: Date | None = None,
    ):
        self._calendar = calendar or NullCalendar()
        self._convention = convention
        self._terminationDateConvention = terminationDateConvention
        self._tenor = tenor
        self._rule = rule
        self._endOfMonth = endOfMonth
        self._firstDate = firstDate or Date()
        self._nextToLastDate = nextToLastDate or Date()
        self._dates: list[Date] = []
        self._isRegular: list[bool] = []

        if dates is not None:
            # List-based constructor
            self._dates = list(dates)
            self._isRegular = list(isRegular) if isRegular else []
            if tenor and not allowsEndOfMonth(tenor):
                self._endOfMonth = False
        elif effectiveDate is not None and terminationDate is not None and tenor is not None and rule is not None:
            # Rule-based constructor
            if endOfMonth is None:
                self._endOfMonth = False
            elif not allowsEndOfMonth(tenor):
                self._endOfMonth = False

            self._generate(effectiveDate, terminationDate, tenor, rule)
        # else: empty schedule

    def _generate(
        self,
        effectiveDate: Date,
        terminationDate: Date,
        tenor: Period,
        rule: DateGeneration.Rule,
    ) -> None:
        if not terminationDate:
            raise ValueError("null termination date")
        if not effectiveDate:
            raise ValueError("null effective date")
        if effectiveDate >= terminationDate:
            raise ValueError(
                f"effective date ({effectiveDate}) later than or equal to termination date ({terminationDate})"
            )

        if tenor.length() == 0:
            self._rule = DateGeneration.Rule.Zero
            rule = DateGeneration.Rule.Zero

        nullCalendar = NullCalendar()
        eom = self._endOfMonth or False

        if rule == DateGeneration.Rule.Zero:
            self._tenor = Period(0, TimeUnit.Years)
            self._dates.append(effectiveDate)
            self._dates.append(terminationDate)
            self._isRegular.append(True)

        elif rule == DateGeneration.Rule.Backward:
            self._dates.append(terminationDate)

            seed = terminationDate
            periods = 1
            if self._nextToLastDate and self._nextToLastDate.serialNumber() != 0:
                self._dates.append(self._nextToLastDate)
                temp = nullCalendar.advance(seed, -periods, tenor.units(), self._convention, eom)
                self._isRegular.append(temp == self._nextToLastDate)
                seed = self._nextToLastDate

            exitDate = effectiveDate
            if self._firstDate and self._firstDate.serialNumber() != 0:
                exitDate = self._firstDate

            while True:
                temp = nullCalendar.advance(
                    seed,
                    period=Period(-periods * tenor.length(), tenor.units()),
                    convention=self._convention,
                    endOfMonth=eom,
                )
                if temp < exitDate:
                    if (
                        self._firstDate
                        and self._firstDate.serialNumber() != 0
                        and self._calendar.adjust(self._dates[-1], self._convention)
                        != self._calendar.adjust(self._firstDate, self._convention)
                    ):
                        self._dates.append(self._firstDate)
                        self._isRegular.append(False)
                    break
                else:
                    if self._calendar.adjust(self._dates[-1], self._convention) != self._calendar.adjust(
                        temp, self._convention
                    ):
                        self._dates.append(temp)
                        self._isRegular.append(True)
                    periods += 1

            if self._calendar.adjust(self._dates[-1], self._convention) != self._calendar.adjust(
                effectiveDate, self._convention
            ):
                self._dates.append(effectiveDate)
                self._isRegular.append(False)

            self._dates.reverse()
            self._isRegular.reverse()

        elif rule == DateGeneration.Rule.Forward:
            self._dates.append(effectiveDate)

            seed = effectiveDate
            periods = 1
            if self._firstDate and self._firstDate.serialNumber() != 0:
                self._dates.append(self._firstDate)
                temp = nullCalendar.advance(
                    seed,
                    period=Period(periods * tenor.length(), tenor.units()),
                    convention=self._convention,
                    endOfMonth=eom,
                )
                self._isRegular.append(temp == self._firstDate)
                seed = self._firstDate

            exitDate = terminationDate
            if self._nextToLastDate and self._nextToLastDate.serialNumber() != 0:
                exitDate = self._nextToLastDate

            while True:
                temp = nullCalendar.advance(
                    seed,
                    period=Period(periods * tenor.length(), tenor.units()),
                    convention=self._convention,
                    endOfMonth=eom,
                )
                if temp > exitDate:
                    if (
                        self._nextToLastDate
                        and self._nextToLastDate.serialNumber() != 0
                        and self._calendar.adjust(self._dates[-1], self._convention)
                        != self._calendar.adjust(self._nextToLastDate, self._convention)
                    ):
                        self._dates.append(self._nextToLastDate)
                        self._isRegular.append(False)
                    break
                else:
                    if self._calendar.adjust(self._dates[-1], self._convention) != self._calendar.adjust(
                        temp, self._convention
                    ):
                        self._dates.append(temp)
                        self._isRegular.append(True)
                    periods += 1

            term_conv = self._terminationDateConvention or self._convention
            if self._calendar.adjust(self._dates[-1], term_conv) != self._calendar.adjust(terminationDate, term_conv):
                self._dates.append(terminationDate)
                self._isRegular.append(False)

        elif rule == DateGeneration.Rule.ThirdWednesday:
            # Forward generation, then adjust intermediate dates to 3rd Wednesday
            self._dates.append(effectiveDate)
            seed = effectiveDate
            periods = 1
            exitDate = terminationDate

            while True:
                temp = nullCalendar.advance(
                    seed,
                    period=Period(periods * tenor.length(), tenor.units()),
                    convention=self._convention,
                    endOfMonth=False,
                )
                if temp > exitDate:
                    break
                else:
                    if self._calendar.adjust(self._dates[-1], self._convention) != self._calendar.adjust(
                        temp, self._convention
                    ):
                        self._dates.append(temp)
                        self._isRegular.append(True)
                    periods += 1

            if self._calendar.adjust(self._dates[-1], self._convention) != self._calendar.adjust(
                terminationDate, self._convention
            ):
                self._dates.append(terminationDate)
                self._isRegular.append(False)

            # Adjust intermediate dates to 3rd Wednesday
            for i in range(1, len(self._dates) - 1):
                self._dates[i] = Date.nthWeekday(3, Weekday.Wednesday, self._dates[i].month(), self._dates[i].year())

        else:
            raise ValueError(f"date generation rule {rule} not yet implemented")

        # Adjustments
        if self._convention != BusinessDayConvention.Unadjusted and rule != DateGeneration.Rule.ThirdWednesday:
            self._dates[0] = self._calendar.adjust(self._dates[0], self._convention)

        term_conv = self._terminationDateConvention or self._convention
        if term_conv != BusinessDayConvention.Unadjusted:
            self._dates[-1] = self._calendar.adjust(self._dates[-1], term_conv)

        if eom and self._calendar.isEndOfMonth(seed):
            for i in range(1, len(self._dates) - 1):
                self._dates[i] = self._calendar.adjust(Date.endOfMonth(self._dates[i]), self._convention)
        else:
            for i in range(1, len(self._dates) - 1):
                self._dates[i] = self._calendar.adjust(self._dates[i], self._convention)

        # Safety: remove duplicate next-to-last
        if len(self._dates) >= 2 and self._dates[-2] >= self._dates[-1]:
            if len(self._isRegular) >= 2:
                self._isRegular[-2] = self._dates[-2] == self._dates[-1]
            self._dates[-2] = self._dates[-1]
            self._dates.pop()
            self._isRegular.pop()

        if len(self._dates) >= 2 and self._dates[1] <= self._dates[0]:
            self._isRegular[1] = self._dates[1] == self._dates[0]
            self._dates[1] = self._dates[0]
            self._dates.pop(0)
            self._isRegular.pop(0)

        if len(self._dates) <= 1:
            raise ValueError(f"degenerate single date ({self._dates[0] if self._dates else 'empty'}) schedule")

    def size(self) -> int:
        return len(self._dates)

    def empty(self) -> bool:
        return len(self._dates) == 0

    def dates(self) -> list[Date]:
        return self._dates

    def date(self, i: int) -> Date:
        return self._dates[i]

    def __getitem__(self, i: int) -> Date:
        return self._dates[i]

    def startDate(self) -> Date:
        if not self._dates:
            raise ValueError("empty Schedule: no start date")
        return self._dates[0]

    def endDate(self) -> Date:
        if not self._dates:
            raise ValueError("empty Schedule: no end date")
        return self._dates[-1]

    def calendar(self) -> Calendar:
        return self._calendar

    def businessDayConvention(self) -> BusinessDayConvention:
        return self._convention

    def hasTenor(self) -> bool:
        return self._tenor is not None

    def tenor(self) -> Period:
        if self._tenor is None:
            raise ValueError("full interface (tenor) not available")
        return self._tenor

    def hasRule(self) -> bool:
        return self._rule is not None

    def rule(self) -> DateGeneration.Rule:
        if self._rule is None:
            raise ValueError("full interface (rule) not available")
        return self._rule

    def hasEndOfMonth(self) -> bool:
        return self._endOfMonth is not None

    def endOfMonth(self) -> bool:
        if self._endOfMonth is None:
            raise ValueError("full interface (end of month) not available")
        return self._endOfMonth

    def hasIsRegular(self) -> bool:
        return len(self._isRegular) > 0

    def isRegular(self, i: int | None = None) -> bool | list[bool]:
        if i is None:
            if not self._isRegular:
                raise ValueError("full interface (isRegular) not available")
            return self._isRegular
        if not self._isRegular:
            raise ValueError("full interface (isRegular) not available")
        if i < 1 or i > len(self._isRegular):
            raise ValueError(f"index ({i}) must be in [1, {len(self._isRegular)}]")
        return self._isRegular[i - 1]

    def hasTerminationDateBusinessDayConvention(self) -> bool:
        return self._terminationDateConvention is not None

    def terminationDateBusinessDayConvention(self) -> BusinessDayConvention:
        if self._terminationDateConvention is None:
            raise ValueError("full interface (termination date bdc) not available")
        return self._terminationDateConvention

    def __len__(self) -> int:
        return len(self._dates)

    def __iter__(self):
        return iter(self._dates)


class MakeSchedule:
    def __init__(self):
        self._effectiveDate: Date | None = None
        self._terminationDate: Date | None = None
        self._tenor: Period | None = None
        self._calendar: Calendar | None = None
        self._convention: BusinessDayConvention | None = None
        self._terminationDateConvention: BusinessDayConvention | None = None
        self._rule: DateGeneration.Rule = DateGeneration.Rule.Backward
        self._endOfMonth: bool = False
        self._firstDate: Date | None = None
        self._nextToLastDate: Date | None = None

    def set_from(self, effectiveDate: Date) -> MakeSchedule:
        self._effectiveDate = effectiveDate
        return self

    # alias for Pythonic usage
    from_date = set_from

    def to(self, terminationDate: Date) -> MakeSchedule:
        self._terminationDate = terminationDate
        return self

    def withTenor(self, tenor: Period) -> MakeSchedule:
        self._tenor = tenor
        return self

    def withFrequency(self, frequency: Frequency) -> MakeSchedule:
        self._tenor = Period(frequency)
        return self

    def withCalendar(self, calendar: Calendar) -> MakeSchedule:
        self._calendar = calendar
        return self

    def withConvention(self, convention: BusinessDayConvention) -> MakeSchedule:
        self._convention = convention
        return self

    def withTerminationDateConvention(self, convention: BusinessDayConvention) -> MakeSchedule:
        self._terminationDateConvention = convention
        return self

    def withRule(self, rule: DateGeneration.Rule) -> MakeSchedule:
        self._rule = rule
        return self

    def forwards(self) -> MakeSchedule:
        self._rule = DateGeneration.Rule.Forward
        return self

    def backwards(self) -> MakeSchedule:
        self._rule = DateGeneration.Rule.Backward
        return self

    def endOfMonth(self, flag: bool = True) -> MakeSchedule:
        self._endOfMonth = flag
        return self

    def withFirstDate(self, d: Date) -> MakeSchedule:
        self._firstDate = d
        return self

    def withNextToLastDate(self, d: Date) -> MakeSchedule:
        self._nextToLastDate = d
        return self

    def build(self) -> Schedule:
        if self._effectiveDate is None:
            raise ValueError("effective date not provided")
        if self._terminationDate is None:
            raise ValueError("termination date not provided")
        if self._tenor is None:
            raise ValueError("tenor/frequency not provided")

        convention = self._convention
        if convention is None:
            if self._calendar is not None and not self._calendar.empty():
                convention = BusinessDayConvention.Following
            else:
                convention = BusinessDayConvention.Unadjusted

        term_conv = self._terminationDateConvention if self._terminationDateConvention is not None else convention

        calendar = self._calendar if self._calendar is not None else NullCalendar()

        return Schedule(
            effectiveDate=self._effectiveDate,
            terminationDate=self._terminationDate,
            tenor=self._tenor,
            calendar=calendar,
            convention=convention,
            terminationDateConvention=term_conv,
            rule=self._rule,
            endOfMonth=self._endOfMonth,
            firstDate=self._firstDate,
            nextToLastDate=self._nextToLastDate,
        )
