from quantlib.time.calendar import Calendar
from quantlib.time.date import Date


class WeekendsOnly(Calendar):
    """Calendar with no bank holidays except weekends (Sat/Sun)."""

    class _Impl(Calendar.WesternImpl):
        def name(self) -> str:
            return "weekends only"

        def isBusinessDay(self, d: Date) -> bool:
            return not self.isWeekend(d.weekday())

    def __init__(self):
        super().__init__(WeekendsOnly._Impl())
