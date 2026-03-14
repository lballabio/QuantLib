from quantlib.time.calendar import Calendar
from quantlib.time.date import Date
from quantlib.time.weekday import Weekday


class NullCalendar(Calendar):
    """Calendar with no holidays — every day is a business day."""

    class _Impl(Calendar.Impl):
        def name(self) -> str:
            return "Null"

        def isWeekend(self, w: Weekday) -> bool:
            return False

        def isBusinessDay(self, d: Date) -> bool:
            return True

    def __init__(self):
        super().__init__(NullCalendar._Impl())
