from enum import IntEnum


class DateGeneration:
    class Rule(IntEnum):
        Backward = 0
        Forward = 1
        Zero = 2
        ThirdWednesday = 3
        ThirdWednesdayInclusive = 4
        Twentieth = 5
        TwentiethIMM = 6
        OldCDS = 7
        CDS = 8
        CDS2015 = 9
