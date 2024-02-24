#include <ql/currencies/europe.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/indexes/region.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
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

std::vector<ext::shared_ptr<ZeroCouponInflationSwapHelper>>
makeHelpers(const std::vector<Coupon>& coupons,
            std::function<ext::shared_ptr<ZeroCouponInflationSwapHelper>(const Handle<Quote>&, const Date&)>
                makeHelper) {

    std::vector<ext::shared_ptr<ZeroCouponInflationSwapHelper>> instruments;
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

    Real kpi[] = {350.56, 353.56, 359.80, 362.02, 365.82, 370.95, 371.28, 377.81, 383.21,
                  384.04, 387.93, 395.96, 391.50, 395.82, 398.08, 399.93, 401.19, 405.49,
                  405.67, 405.97, 408.05, 409.07, 410.35, 413.34, 412.74};

    Date from(1, January, 2022);
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
    std::vector<Coupon> zcData = {{Date(13, August, 2008), 2.93},  {Date(13, August, 2009), 2.95},
                                  {Date(13, August, 2010), 2.965}, {Date(15, August, 2011), 2.98},
                                  {Date(13, August, 2012), 3.0},   {Date(13, August, 2014), 3.06},
                                  {Date(13, August, 2017), 3.175}, {Date(13, August, 2019), 3.243},
                                  {Date(15, August, 2022), 3.293}, {Date(14, August, 2027), 3.338},
                                  {Date(13, August, 2032), 3.348}, {Date(15, August, 2037), 3.348},
                                  {Date(13, August, 2047), 3.308}, {Date(13, August, 2057), 3.228}};

    Period observationLag = Period(3, Months);
    Calendar calendar = Sweden();
    BusinessDayConvention bdc = ModifiedFollowing;
    DayCounter dc = Thirty360(Thirty360::BondBasis);


    Date studyDate(13, August, 2007);
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
    std::vector<ext::shared_ptr<ZeroCouponInflationSwapHelper>> helpers =
        makeHelpers(zcData, makeHelper);

    return 0;
}
