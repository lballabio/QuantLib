#include <ql/currencies/europe.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/indexes/region.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/inflation/piecewisezeroinflationcurve.hpp>
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
#include <iostream>


#define LENGTH(a) (sizeof(a) / sizeof(a[0]))


using namespace QuantLib;

struct Coupon {
    Date maturity;
    Rate rate;
};

std::vector<ext::shared_ptr<BootstrapHelper<ZeroInflationTermStructure>>>
makeHelpers(const std::vector<Coupon>& coupons,
            std::function<ext::shared_ptr<ZeroCouponInflationSwapHelper>(const Handle<Quote>&, const Date&)>
                makeHelper) {

    std::vector<ext::shared_ptr<BootstrapHelper<ZeroInflationTermStructure>>> instruments;
    for (Coupon coupon : coupons) {
        Handle<Quote> quote(ext::shared_ptr<Quote>(new SimpleQuote(coupon.rate / 100.0)));
        ext::shared_ptr<ZeroCouponInflationSwapHelper> anInstrument = makeHelper(quote, coupon.maturity);
        instruments.push_back(anInstrument);
    }

    return instruments;
}

int main() {

    // KPI Index

    ZeroInflationIndex kpiIndex("KPI", EURegion(), false, Monthly, Period(1, Days), SEKCurrency());

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

    // YoY KPI

    boost::shared_ptr<ZeroInflationIndex> pKpiIndex =
        boost::make_shared<ZeroInflationIndex>(kpiIndex);

    YoYInflationIndex yyIndex(pKpiIndex, false);
    Rate yyRate = yyIndex.fixing(Date(1, January, 2024));

    std::cout << "------KPI YoY index------" << '\n';
    std::cout << "1 Jan 2024: " << yyRate * 100 << '%' << std::endl;
    std::cout << "1 Dec 2023: " << yyIndex.fixing(Date(1, December, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Nov 2023: " << yyIndex.fixing(Date(1, November, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Okt 2023: " << yyIndex.fixing(Date(1, October, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Sep 2023: " << yyIndex.fixing(Date(1, September, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Aug 2023: " << yyIndex.fixing(Date(1, August, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Jul 2023: " << yyIndex.fixing(Date(1, July, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Jun 2023: " << yyIndex.fixing(Date(1, June, 2023)) * 100 << '%' << '\n';
    std::cout << "1 May 2023: " << yyIndex.fixing(Date(1, May, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Apr 2023: " << yyIndex.fixing(Date(1, April, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Mar 2023: " << yyIndex.fixing(Date(1, March, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Feb 2023: " << yyIndex.fixing(Date(1, February, 2023)) * 100 << '%' << '\n';
    std::cout << "1 Jan 2023: " << yyIndex.fixing(Date(1, January, 2023)) * 100 << '%' << '\n';


    ///////////////
    // now build the ZCIS (Zero-Coupon Inflation Swap)
    // Fixed Leg 
    // SE statsobligation 2/5/7/10 år (2024 January)
    std::vector<Coupon> zcData = {{Date(1, January, 2024), 4.00},
                                  {Date(1, January, 2026), 2.4344},
                                  {Date(1, January, 2029), 2.1718},
                                  {Date(1, January, 2031), 2.1629},
                                  {Date(1, January, 2034), 2.2307}};


    Period observationLag = Period(3, Months);
    Calendar calendar = Sweden();
    BusinessDayConvention bdc = ModifiedFollowing;
    DayCounter dc = Thirty360(Thirty360::BondBasis);


    Date studyDate(15, August, 2022);
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
                                                               pKpiIndex, // Inflation leg (KPI)
                                                               CPI::AsIndex,
                                                               nominalTermStructure);
    };

    std::vector<ext::shared_ptr<BootstrapHelper<ZeroInflationTermStructure>>> helpers =
        makeHelpers(zcData, makeHelper);

    Frequency frequency = Monthly;
    Date baseDate = pKpiIndex->lastFixingDate();

    ext::shared_ptr<PiecewiseZeroInflationCurve<Linear> > pZCIS =
        ext::make_shared<PiecewiseZeroInflationCurve<Linear>>(
            studyDate,
            baseDate,
            frequency,
            dc,
            helpers
            );

    std::vector<Time> times = pZCIS->times();
    std::vector<Real> rates = pZCIS->data();
    std::vector<Date> c = pZCIS->dates();
    std::vector<std::pair<Date, Rate> > nodes = pZCIS->nodes();

    std::cout << "ZCIS baseDate(): " << pZCIS->baseDate() << '\n';
    std::cout << "ZCIS maxDate(): " << pZCIS->maxDate() << '\n';

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
    }*/

    std::cout << "---nodes---" << '\n';
    for (auto node : nodes) {
        std::cout << node.first << '\n';
        std::cout << node.second * 100 << '%' << '\n';
    }

    return 0;
}
