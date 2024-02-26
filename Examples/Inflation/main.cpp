#include <ql/currencies/europe.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/indexes/region.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/inflation/piecewisezeroinflationcurve.hpp>
#include <ql/termstructures/inflation/piecewiseyoyinflationcurve.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/sweden.hpp>
#include <ql/time/date.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/frequency.hpp>
#include <ql/time/period.hpp>
#include <ql/time/schedule.hpp>
#include <ql/types.hpp>

#include <matplot/matplot.h>

#include <vector>
#include <iostream>


#define LENGTH(a) (sizeof(a) / sizeof(a[0]))


using namespace QuantLib;

// Coupon SWAP data
struct Coupon {
    Date maturity;
    Rate rate;
};

ext::shared_ptr<YieldTermStructure> nominalTermStructure(Date studyDate) {
    return ext::shared_ptr<YieldTermStructure>(
            new FlatForward(studyDate, 0.05, Actual360()));
}

template <class T>
std::vector<ext::shared_ptr<BootstrapHelper<T>>>
makeHelpers(const std::vector<Coupon>& coupons,
            std::function<ext::shared_ptr<BootstrapHelper<T>>(const Handle<Quote>&, const Date&)>
                makeHelper) {

    std::vector<ext::shared_ptr<BootstrapHelper<T>>> instruments;
    for (Coupon coupon : coupons) {
        Handle<Quote> quote(ext::shared_ptr<Quote>(new SimpleQuote(coupon.rate / 100.0)));
        ext::shared_ptr<BootstrapHelper<T>> anInstrument = makeHelper(quote, coupon.maturity);
        instruments.push_back(anInstrument);
    }

    return instruments;
}

int main() {

    // KPI Index

    ZeroInflationIndex kpiIndex("KPI", EURegion(), false, Monthly, Period(1, Days), SEKCurrency());
    boost::shared_ptr<ZeroInflationIndex> pKpiIndex =
        boost::make_shared<ZeroInflationIndex>(kpiIndex);

    Real kpi[] = {338.09, 339.01, 339.54, 340.37, 341.04, 341.32, 342.23, 343.99, 345.74, 346.44,
                  348.03, 352.47, 350.56, 353.56, 359.80, 362.02, 365.82, 370.95, 371.28, 377.81,
                  383.21, 384.04, 387.93, 395.96, 391.50, 395.82, 398.08, 399.93, 401.19, 405.49,
                  405.67, 405.97, 408.05, 409.07, 410.35, 413.34, 412.74};

    Date from(1, January, 2021);
    Date to(1, January, 2024);
    Schedule kpiSchedule = MakeSchedule().from(from).to(to).withFrequency(Monthly);

    for (Size i = 0; i < LENGTH(kpi); ++i) {
        kpiIndex.addFixing(kpiSchedule[i], kpi[i]);
    }

    std::cout << "kpiSchedule.size(): " << kpiSchedule.size() << '\n';
    TimeSeries<Real> ts = kpiIndex.timeSeries();
    std::cout << "ts.size(): " << ts.size() << '\n';

    std::cout << "------KPI Zero index------" << '\n';
    std::cout << "1 Jan 2024: " << kpiIndex.fixing(Date(1, January, 2024)) << std::endl;
    std::cout << "1 Dec 2023: " << kpiIndex.fixing(Date(1, December, 2023)) << std::endl;
    std::cout << "1 Nov 2023: " << kpiIndex.fixing(Date(1, November, 2023)) << std::endl;

    // Common to YoY and ZC
    Actual360 dc360;
    // Use the same for both YoY and ZC inflation swap curves
    Period observationLag = Period(3, Months);
    Calendar calendar = Sweden();
    BusinessDayConvention bdc = ModifiedFollowing;
    DayCounter dc = Thirty360(Thirty360::BondBasis);
    Frequency frequency = Monthly;
    Date baseDate = pKpiIndex->lastFixingDate();

    // Evaluation date, AsOfDate
    Date studyDate(15, August, 2022);

    ///////////////
    // YoY Inflation Swap
    // YoY KPI

    YoYInflationIndex yyIndex(pKpiIndex, false);
    boost::shared_ptr<YoYInflationIndex> pYoyIndex =
        boost::make_shared<YoYInflationIndex>(yyIndex);
        Rate yyRate = yyIndex.fixing(Date(1, January, 2024));
    std::cout << "1 Jan 2024: " << yyRate * 100 << '%' << std::endl;

    // YoY Swaps (first 2 years to get monthly granularity)
    std::vector<Coupon> yoySwap = {
        { Date(1, February, 2024), 5.45 },
        { Date(1, March,    2024), 5.35 },
        { Date(1, April,    2024), 5.23 },
        { Date(1, May,      2024), 5.21 },
        { Date(1, June,     2024), 5.14 },
        { Date(1, July,     2024), 5.00 },
        { Date(1, August,   2024), 4.80 },
        { Date(1, September,2024), 4.55 },
        { Date(1, October,  2024), 4.45 },
        { Date(1, November, 2024), 4.37 },
        { Date(1, December, 2024), 4.25 },
        { Date(1, January,  2025), 3.135 },
        { Date(1, February, 2025), 3.155 },
        { Date(1, March,    2025), 3.145 },
        { Date(1, April,    2025), 3.145 },
        { Date(1, May,      2025), 3.145 },
        { Date(1, June,     2025), 3.145 },
        { Date(1, July,     2025), 3.145 },
        { Date(1, August,   2025), 3.145 },
        { Date(1, September,2025), 3.145 },
        { Date(1, October,  2025), 3.145 },
        { Date(1, November, 2025), 3.145 },
        { Date(1, December, 2025), 3.145 },
    };


    // TODO
    ext::shared_ptr<YieldTermStructure> yoyNominalTS = nominalTermStructure(studyDate);

    auto makeHelperYoY = [&](const Handle<Quote>& quote, const Date& maturity) {
        return ext::make_shared<YearOnYearInflationSwapHelper>(
                    quote,
                    observationLag,
                    maturity,
                    calendar,
                    bdc,
                    dc,
                    pYoyIndex,
                    Handle<YieldTermStructure>(yoyNominalTS));
    };

    auto helpersYoY = makeHelpers<YoYInflationTermStructure>(yoySwap, makeHelperYoY);

    Rate baseYoYRate = yyIndex.fixing(Date(1, January, 2024));
    std::cout << "baseYoYRate: " << baseYoYRate << '\n';

    auto pYoYIS = ext::make_shared<PiecewiseYoYInflationCurve<Linear>>(
        studyDate, baseDate, baseYoYRate, frequency, true, dc, helpersYoY);


    // ZCIS //
    ///////////////
    // now build the ZCIS (Zero-Coupon Inflation Swap)
    std::vector<Coupon> zcSwap = {{Date(1, January, 2026), 3.145},
                                  {Date(1, January, 2027), 3.145},
                                  {Date(1, January, 2029), 3.145},
                                  {Date(1, January, 2031), 3.145},
                                  {Date(1, January, 2034), 3.145}};


    boost::shared_ptr<YieldTermStructure> yieldTermStructure(
        new FlatForward(studyDate, 0.05, Actual360()));
    Handle<YieldTermStructure> nominalTermStructure(yieldTermStructure);

    auto makeHelper = [&](const Handle<Quote>& quote, const Date& maturity) {
        return ext::make_shared<ZeroCouponInflationSwapHelper>(quote,
                                                               observationLag,
                                                               maturity,
                                                               calendar,
                                                               bdc,
                                                               dc,
                                                               pKpiIndex,
                                                               CPI::AsIndex,
                                                               nominalTermStructure);
    };

    std::vector<ext::shared_ptr<BootstrapHelper<ZeroInflationTermStructure>>> helpers =
        makeHelpers<ZeroInflationTermStructure>(zcSwap, makeHelper);

    

    ext::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pZCIS =
        ext::make_shared<PiecewiseZeroInflationCurve<Linear>>(
            studyDate,
            baseDate,
            frequency,
            dc,
            helpers
            );


    std::vector<Time> times = pZCIS->times();  // year fractions
    std::vector<Real> rates = pZCIS->data();
    std::vector<Date> c = pZCIS->dates();
    std::vector<std::pair<Date, Rate> > nodes = pZCIS->nodes();

    std::cout << "ZCIS baseDate(): " << pZCIS->baseDate() << '\n';
    std::cout << "ZCIS maxDate(): " << pZCIS->maxDate() << '\n';

    //////////////////////
    // Plot Inflation curve


    Date startDay(1, January, 2024); // Must be after or equal base date
    //Date toDayYoY(1, December, 2025);
    
    //Date startDayZc(1, January, 2026);
    Date toDay(1, September, 2033);
    Schedule printSchedule = MakeSchedule().from(startDay).to(toDay).withFrequency(Monthly);

    int monthsYoy = 21;
    int monthsZc = 12*7;
    int months = monthsYoy + monthsZc;

    std::vector<Time> x(months);
    std::vector<Rate> y(months);


    for (int i = 0; i < monthsYoy; ++i) {
        Time t = dc360.yearFraction(startDay, printSchedule[i]);
        std::cout << "Date to print YoY: " << printSchedule[i] << '\n';
        std::cout << "Yearfraction: " << t << '\n';
        Real yyr = pYoYIS->yoyRate(printSchedule[i]);
        std::cout << "YoY rate: " << yyr << '\n';

        x.emplace_back(t);
        y.emplace_back(yyr);
    }
    
    
    for (int i = monthsYoy; i < months; ++i) {
        Time t = dc360.yearFraction(startDay, printSchedule[i]);
        std::cout << "Date to print ZC: " << printSchedule[i] << '\n';
        std::cout << "Yearfraction: " << t << '\n';
        Real zr = pZCIS->zeroRate(printSchedule[i]);
        std::cout << "Zero rate: " << zr << '\n';
        
        x.emplace_back(t);
        y.emplace_back(zr);
    }



    ////// plot ////////
    using namespace matplot;
    plot(x, y, "-o");
    show();
    
    //std::cout << pZCIS->zeroRate(studyDate) << '\n';
    /*
    std::cout << "---times---" << '\n';
    for (auto time : times) {
        std::cout << time << '\n';
    }

    std::cout << "---dates---" << '\n';
    for (auto date : dates) {
        std::cout << date << '\n';
    }

    std::cout << "---rates---" << '\n';
    for (auto rate : rates) {
        std::cout << rate << '\n';
    }

    std::cout << "---nodes---" << '\n';
    for (auto node : nodes) {
        std::cout << node.first << '\n';
        std::cout << node.second * 100 << '%' << '\n';
    }*/

    return 0;
}
