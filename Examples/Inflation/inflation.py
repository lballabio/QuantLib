import QuantLib as ql
from datetime import date
import matplotlib.pyplot as plt


study_date = ql.Date(1, ql.August, 2022)
calendar = ql.Sweden()
convention = ql.ModifiedFollowing
daycounter = ql.Actual360()
dc360 = ql.Actual360()


class Coupon:
    def __init__(self, date, rate):
        self.maturity = date
        self.rate = rate


def nominal_term_structure() -> ql.YieldTermStructureHandle:
    nominal_term_structure = ql.FlatForward(study_date, 0.05, ql.Actual360())
    handle_nts = ql.YieldTermStructureHandle(nominal_term_structure)
    return handle_nts


def create_zero_coupon_inflation_swap_helpers(coupons, index):
    helpers = []
    
    for coupon in coupons:
        quote = ql.SimpleQuote(coupon.rate / 100.0)
        quote_handle = ql.QuoteHandle(quote)
        
        nts = nominal_term_structure()
        observation_lag =ql.Period(1, ql.Months)
        
        helper = ql.ZeroCouponInflationSwapHelper(
            quote_handle,
            observation_lag,
            coupon.maturity,
            calendar,
            convention,
            daycounter,
            index,
            ql.CPI.AsIndex,
            nts
        )
        
        helpers.append(helper)

    return helpers


def get_kpi_index() -> ql.ZeroInflationIndex:
    kpi_index = ql.ZeroInflationIndex("KPI", ql.CustomRegion("Europe", "EU"), False, ql.Monthly, ql.Period(1, ql.Days), ql.SEKCurrency())
    kpi_data = [338.09, 339.01, 339.54, 340.37, 341.04, 341.32, 342.23, 343.99, 345.74, 346.44,
            348.03, 352.47, 350.56, 353.56, 359.80, 362.02, 365.82, 370.95, 371.28, 377.81,
            383.21, 384.04, 387.93, 395.96, 391.50, 395.82, 398.08, 399.93, 401.19, 405.49,
            405.67, 405.97, 408.05, 409.07, 410.35, 413.34, 412.74]
    from_date = ql.Date(1, 1, 2021)
    to_date = ql.Date(1, 1, 2024)
    kpi_schedule = ql.Schedule(from_date, 
                               to_date, 
                               ql.Period(ql.Monthly), 
                               calendar, 
                               convention, 
                               ql.Unadjusted,
            ql.DateGeneration.Backward,False)
    for i in range(len(kpi_data)):
        kpi_index.addFixing(kpi_schedule[i], kpi_data[i])
    return kpi_index
    

zcSwap = [
    Coupon(ql.Date(1, 1, 2026), 3.145),
    Coupon(ql.Date(1, 1, 2027), 3.145),
    Coupon(ql.Date(1, 1, 2029), 3.145),
    Coupon(ql.Date(1, 1, 2031), 3.145),
    Coupon(ql.Date(1, 1, 2034), 3.145)
]

#########################
inflation_index = get_kpi_index()
helper = create_zero_coupon_inflation_swap_helpers(zcSwap, inflation_index)

frequency = ql.Monthly
base_rate = 0.0
period_lag = ql.Period("1M")

bootstrap_curve = ql.PiecewiseZeroInflation(
    study_date,
    calendar,
    daycounter,
    period_lag,
    frequency,
    base_rate,
    helper
)

## Plot curve
start_date = ql.Date(1, 1, 2025)
end_date = ql.Date(1, 2, 2033)
calendar = ql. Sweden()


schedule = ql.MakeSchedule(start_date, end_date, ql.Period('1M'), rule=ql.DateGeneration.Forward)

x = []
y = []

for day in  schedule:
    print(day)
    _y =bootstrap_curve.zeroRate(day)
    print(_y)
    y.append(_y)
    year_fraction = dc360.yearFraction(start_date, day)
    print(year_fraction)
    x.append(year_fraction)



plt.plot(x, y)
plt.show()
