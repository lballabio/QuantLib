
from QuantLib import *
import time

gmt = time.gmtime(time.time())
today =  Date(gmt[2],gmt[1],gmt[0])

currency =  EUR()
depoDayCount = Actual360()
curveDayCount =  Actual365()
depoCalendar =  TARGET()
deposits = [
	DepositRate(depoCalendar.roll(today.plusWeeks(1)), 0.0426, depoDayCount),
	DepositRate(depoCalendar.roll(today.plusMonths(1)),0.04381,depoDayCount),
	DepositRate(depoCalendar.roll(today.plusMonths(2)),0.04423,depoDayCount),
	DepositRate(depoCalendar.roll(today.plusMonths(3)),0.04486,depoDayCount),
	DepositRate(depoCalendar.roll(today.plusMonths(6)),0.0464, depoDayCount),
	DepositRate(depoCalendar.roll(today.plusMonths(9)),0.0481, depoDayCount),
	DepositRate(depoCalendar.roll(today.plusYears(1)), 0.04925,depoDayCount)
]

curve = PiecewiseConstantForwards(currency,curveDayCount,today,deposits)

flatCurve =  FlatForward(currency,curveDayCount,today,0.05)


days = 380
settlement = curve.settlementDate()

time      = map(lambda i : i/365.0, range(days))
dates     = map(settlement.plusDays,range(days))

discounts = map(lambda x: curve.discount(x, 1),  dates)
zeros     = map(lambda x: curve.zeroYield(x, 1), dates)
forwards  = map(lambda x: curve.forward(x, 1),   dates)


print ' time                dates          disc    zero   forward'
for i in range(days):
    print '%5.3f %20s %13.10f %7.3f %9.3f' % \
        (time[i], dates[i], discounts[i], zeros[i]*100, forwards[i]*100)
    