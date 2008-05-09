/* This example sets up a callable fixed rate bond with a Hull White pricing
   engine and compares to Bloomberg's Hull White price/yield calculations.
*/

#ifdef BOOST_MSVC
/* Uncomment the following lines to unmask floating-point
   exceptions. Warning: unpredictable results can arise...

   See http://www.wilmott.com/messageview.cfm?catid=10&threadid=9481
   Is there anyone with a definitive word about this?
*/
// #include <float.h>
// namespace { unsigned int u = _controlfp(_EM_INEXACT, _MCW_EM); }
#endif

#include <ql/quantlib.hpp>
#include <vector>
#include <math.h>
#include <iomanip>
#include <iostream>
#include <boost/timer.hpp>

using namespace std;
using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {
    Integer sessionId() { return 0; }
}
#endif


boost::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             const boost::shared_ptr<Quote>& forward,
             const DayCounter& dc,
             const Compounding& compounding,
             const Frequency& frequency) {
    return boost::shared_ptr<YieldTermStructure>(
                                       new FlatForward(today,
                                                       Handle<Quote>(forward),
                                                       dc,
                                                       compounding,
                                                       frequency));
}


boost::shared_ptr<YieldTermStructure>
    flatRate(const Date& today,
             Rate forward,
             const DayCounter& dc,
             const Compounding &compounding,
             const Frequency &frequency) {
    return flatRate(today,
            boost::shared_ptr<Quote>(new SimpleQuote(forward)),
            dc,
            compounding,
            frequency);
}


int main(int, char* [])
{
    try {

        boost::timer timer;

        Date today = Date(16,October,2007);
        Settings::instance().evaluationDate() = today;

        cout <<  endl;
        cout << "Pricing a callable fixed rate bond using" << endl;
        cout << "Hull White model w/ reversion parameter = 0.03" << endl;
        cout << "BAC4.65 09/15/12  ISIN: US06060WBJ36" << endl;
        cout << "roughly five year tenor, ";
        cout << "quarterly coupon and call dates" << endl;
        cout << "reference date is : " << today << endl << endl;

        /* Bloomberg OAS1: "N" model (Hull White)
           varying volatility parameter

           The curve entered into Bloomberg OAS1 is a flat curve,
           at constant yield = 5.5%, semiannual compounding.
           Assume here OAS1 curve uses an ACT/ACT day counter,
           as documented in PFC1 as a "default" in the latter case.
        */

        // set up a flat curve corresponding to Bloomberg flat curve

        Rate bbCurveRate = 0.055;
        DayCounter bbDayCounter = ActualActual(ActualActual::Bond);
        InterestRate bbIR(bbCurveRate,bbDayCounter,Compounded,Semiannual);

        Handle<YieldTermStructure> termStructure(flatRate(today,
                                                          bbIR.rate(),
                                                          bbIR.dayCounter(),
                                                          bbIR.compounding(),
                                                          bbIR.frequency()));

        // set up the call schedule

        CallabilitySchedule callSchedule;
        Real callPrice = 100.;
        Size numberOfCallDates = 24;
        Date callDate = Date(15,September,2006);

        for (Size i=0; i< numberOfCallDates; i++) {
            Calendar nullCalendar = NullCalendar();

            Callability::Price myPrice(callPrice,
                                       Callability::Price::Clean);
            callSchedule.push_back(
                boost::shared_ptr<Callability>(
                    new Callability(myPrice,
                                    Callability::Call,
                                    callDate )));
            callDate = nullCalendar.advance(callDate, 3, Months);
        }


        // set up the callable bond

        Date dated = Date(16,September,2004);
        Date issue = dated;
        Date maturity = Date(15,September,2012);
        Natural settlementDays = 3;  // Bloomberg OAS1 settle is Oct 19, 2007
        Calendar bondCalendar = UnitedStates(UnitedStates::GovernmentBond);
        Real coupon = .0465;
        Frequency frequency = Quarterly;
        Real redemption = 100.0;
        Real faceAmount = 100.0;

        /* The 30/360 day counter Bloomberg uses for this bond cannot
           reproduce the US Bond/ISMA (constant) cashflows used in PFC1.
           Therefore use ActAct(Bond)
        */
        DayCounter bondDayCounter = ActualActual(ActualActual::Bond);

        // PFC1 shows no indication dates are being adjusted
        // for weekends/holidays for vanilla bonds
        BusinessDayConvention accrualConvention = Unadjusted;
        BusinessDayConvention paymentConvention = Unadjusted;

        Schedule sch(dated, maturity, Period(frequency), bondCalendar,
                     accrualConvention, accrualConvention,
                     DateGeneration::Backward, false);

        Size maxIterations = 1000;
        Real accuracy = 1e-8;
        Integer gridIntervals = 40;
        Real reversionParameter = .03;

        // output price/yield results for varying volatility parameter

        Real sigma = QL_EPSILON; // core dumps if zero on Cygwin

        boost::shared_ptr<ShortRateModel> hw0(
                       new HullWhite(termStructure,reversionParameter,sigma));

        boost::shared_ptr<PricingEngine> engine0(
                      new TreeCallableFixedRateBondEngine(hw0,gridIntervals));

        CallableFixedRateBond callableBond0(settlementDays, faceAmount, sch,
                                            vector<Rate>(1, coupon),
                                            bondDayCounter, paymentConvention,
                                            redemption, issue, callSchedule);
        callableBond0.setPricingEngine(engine0);

        cout << setprecision(2)
             << showpoint
             << fixed
             << "sigma/vol (%) = "
             << 100.*sigma
             << endl;

        cout << "QuantLib  NPV/yld (%) ";
        cout << callableBond0.NPV() << " / "
             << 100. * callableBond0.yield(bondDayCounter,
                                           Compounded,
                                           frequency,
                                           accuracy,
                                           maxIterations)
             << endl;

        cout << "Bloomberg NPV/yld (%) ";
        cout << "96.50 / 5.47"
             << endl
             << endl;

        sigma = .01;

        cout << "sigma/vol (%) = " << 100.*sigma << endl;

        boost::shared_ptr<ShortRateModel> hw1(
                       new HullWhite(termStructure,reversionParameter,sigma));

        boost::shared_ptr<PricingEngine> engine1(
                      new TreeCallableFixedRateBondEngine(hw1,gridIntervals));

        CallableFixedRateBond callableBond1(settlementDays, faceAmount, sch,
                                            vector<Rate>(1, coupon),
                                            bondDayCounter, paymentConvention,
                                            redemption, issue, callSchedule);
        callableBond1.setPricingEngine(engine1);


        cout << "QuantLib  NPV/yld (%) ";
        cout << callableBond1.NPV() << " / "
             << 100.* callableBond1.yield(bondDayCounter,
                                          Compounded,
                                          frequency,
                                          accuracy,
                                          maxIterations)
             << endl;

        cout << "Bloomberg NPV/yld (%) ";
        cout << "95.68 / 5.66"
             << endl
             << endl;

        ////////////////////

        sigma = .03;

        boost::shared_ptr<ShortRateModel> hw2(
                     new HullWhite(termStructure, reversionParameter, sigma));

        boost::shared_ptr<PricingEngine> engine2(
                      new TreeCallableFixedRateBondEngine(hw2,gridIntervals));

        CallableFixedRateBond callableBond2(settlementDays, faceAmount, sch,
                                            vector<Rate>(1, coupon),
                                            bondDayCounter, paymentConvention,
                                            redemption, issue, callSchedule);
        callableBond2.setPricingEngine(engine2);

        cout << "sigma/vol (%) = "
             << 100.*sigma
             << endl;

        cout << "QuantLib  NPV/yld (%) ";
        cout << callableBond2.NPV() << " / "
             << 100. * callableBond2.yield(bondDayCounter,
                                           Compounded,
                                           frequency,
                                           accuracy,
                                           maxIterations)
             << endl;

        cout << "Bloomberg NPV/yld (%) ";
        cout << "92.34 / 6.49"
             << endl
             << endl;

        ////////////////////////////

        sigma = .06;

        boost::shared_ptr<ShortRateModel> hw3(
                     new HullWhite(termStructure, reversionParameter, sigma));

        boost::shared_ptr<PricingEngine> engine3(
                      new TreeCallableFixedRateBondEngine(hw3,gridIntervals));

        CallableFixedRateBond callableBond3(settlementDays, faceAmount, sch,
                                            vector<Rate>(1, coupon),
                                            bondDayCounter, paymentConvention,
                                            redemption, issue, callSchedule);
        callableBond3.setPricingEngine(engine3);

        cout << "sigma/vol (%) = "
             << 100.*sigma
             << endl;

        cout << "QuantLib  NPV/yld (%) ";
        cout << callableBond3.NPV() << " / "
             << 100. * callableBond3.yield(bondDayCounter,
                                           Compounded,
                                           frequency,
                                           accuracy,
                                           maxIterations)
             << endl;

        cout << "Bloomberg NPV/yld (%) ";
        cout << "87.16 / 7.83"
             << endl
             << endl;

        /////////////////////////

        sigma = .12;

        boost::shared_ptr<ShortRateModel> hw4(
                     new HullWhite(termStructure, reversionParameter, sigma));

        boost::shared_ptr<PricingEngine> engine4(
                      new TreeCallableFixedRateBondEngine(hw4,gridIntervals));

        CallableFixedRateBond callableBond4(settlementDays, faceAmount, sch,
                                            vector<Rate>(1, coupon),
                                            bondDayCounter, paymentConvention,
                                            redemption, issue, callSchedule);
        callableBond4.setPricingEngine(engine4);

        cout << "sigma/vol (%) = "
             << 100.*sigma
             << endl;

        cout << "QuantLib  NPV/yld (%) ";
        cout << callableBond4.NPV() << " / "
             << 100.* callableBond4.yield(bondDayCounter,
                                          Compounded,
                                          frequency,
                                          accuracy,
                                          maxIterations)
             << endl;

        cout << "Bloomberg NPV/yld (%) ";
        cout << "77.31 / 10.65"
             << endl
             << endl;

        Real seconds = timer.elapsed();
        Integer hours = int(seconds/3600);
        seconds -= hours * 3600;
        Integer minutes = int(seconds/60);
        seconds -= minutes * 60;
        cout << " \nRun completed in ";
        if (hours > 0)
            cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            cout << minutes << " m ";
        cout << fixed << setprecision(0)
             << seconds << " s\n" << endl;

        return 0;

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}

