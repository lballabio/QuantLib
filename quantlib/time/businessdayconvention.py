from enum import IntEnum


class BusinessDayConvention(IntEnum):
    Following = 0
    ModifiedFollowing = 1
    Preceding = 2
    ModifiedPreceding = 3
    Unadjusted = 4
    HalfMonthModifiedFollowing = 5
    Nearest = 6
