from quantlib.time.calendar import Calendar
from quantlib.time.date import Date
from quantlib.time.month import Month


class TARGET(Calendar):
    """TARGET calendar (Trans-European Automated Real-time Gross Settlement Express Transfer)."""

    class _Impl(Calendar.WesternImpl):
        def name(self) -> str:
            return "TARGET"

        def isBusinessDay(self, d: Date) -> bool:
            w = d.weekday()
            day = d.dayOfMonth()
            dd = d.dayOfYear()
            m = d.month()
            y = d.year()
            em = self.easterMonday(y)

            if (
                self.isWeekend(w)
                or (day == 1 and m == Month.January)
                or (dd == em - 3 and y >= 2000)  # Good Friday
                or (dd == em and y >= 2000)  # Easter Monday
                or (day == 1 and m == Month.May and y >= 2000)  # Labour Day
                or (day == 25 and m == Month.December)  # Christmas
                or (day == 26 and m == Month.December and y >= 2000)  # Day of Goodwill
                or (day == 31 and m == Month.December and y in (1998, 1999, 2001))
            ):
                return False
            return True

    def __init__(self):
        super().__init__(TARGET._Impl())
