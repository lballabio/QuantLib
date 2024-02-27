import QuantLib as ql
from datetime import date


study_date = ql.Date(1, ql.January, 2023)
calendar = ql.Sweden()
convention = ql.ModifiedFollowing
daycounter = ql.Actual360()


class Coupon:
    def __init__(self, date, rate):
        self.maturity = date
        self.rate = rate


def create_zero_coupon_inflation_swap_helpers(coupons, index):
    helpers = []
    
    for coupon in coupons:
        quote = ql.SimpleQuote(coupon.rate)
        quote_handle = ql.QuoteHandle(quote)
        
        
        observation_lag =ql.Period(3, ql.Months)
        nominal_term_structure = ql.FlatForward(study_date, 0.05, ql.Actual360())
        handle_nts = ql.YieldTermStructureHandle(nominal_term_structure)

        helper = ql.ZeroCouponInflationSwapHelper(
            quote_handle,
            observation_lag,
            coupon.maturity,
            calendar,
            convention,
            daycounter,
            index,
            ql.CPI.AsIndex,
            handle_nts
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
    Coupon(ql.Date(1, ql.January, 2026), 3.145),
    Coupon(ql.Date(1, ql.January, 2027), 3.145),
    Coupon(ql.Date(1, ql.January, 2029), 3.145),
    Coupon(ql.Date(1, ql.January, 2031), 3.145),
    Coupon(ql.Date(1, ql.January, 2034), 3.145)
]

#########################
inflation_index = get_kpi_index()
helper = create_zero_coupon_inflation_swap_helpers(zcSwap, inflation_index)


bootstrap_curve = ql.PiecewiseZeroInflation(
    study_date,
    ql.UnitedKingdom(),  # ?
    ql.OneDayCounter(),  # ?
    ql.Period("2M"),
    ql.Annual,  # ?
    0.0,
    helper,
    1.0e-5,
)
