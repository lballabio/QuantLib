/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers
 Copyright (C) 2026 Kyrylo Protsenko

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/processes/gsrprocess.hpp>
#include <ql/methods/montecarlo/pathgenerator.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/models/shortrate/onefactormodels/gsr.hpp>
#include <ql/instruments/nonstandardswap.hpp>
#include <ql/instruments/nonstandardswaption.hpp>
#include <ql/pricingengines/swaption/gaussian1dswaptionengine.hpp>
#include <ql/pricingengines/swaption/gaussian1djamshidianswaptionengine.hpp>
#include <ql/pricingengines/swaption/gaussian1dnonstandardswaptionengine.hpp>
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/models/shortrate/calibrationhelpers/swaptionhelper.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/pricingengines/swaption/jamshidianswaptionengine.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/instruments/makevanillaswap.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>

using namespace QuantLib;
using boost::unit_test_framework::test_suite;

using std::fabs;

namespace {
    std::pair<ext::shared_ptr<Swaption>,
              ext::shared_ptr<OvernightIndexedSwap> >
    makeObservationConventionSwaption(
        const Schedule& schedule,
        const DayCounter& fixedDayCounter,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        const Calendar& paymentCalendar,
        const ext::shared_ptr<Exercise>& exercise,
        const ext::shared_ptr<PricingEngine>& engine,
        Natural lookbackDays,
        Natural lockoutDays,
        bool applyObservationShift,
        RateAveraging::Type averaging = RateAveraging::Compound) {

        auto swap = ext::make_shared<OvernightIndexedSwap>(
            Swap::Payer, 1.0, schedule, 0.03, fixedDayCounter, overnightIndex,
            0.0, 0, Following, paymentCalendar, false, averaging,
            lookbackDays, lockoutDays, applyObservationShift);
        auto swaption = ext::make_shared<Swaption>(swap, exercise);
        swaption->setPricingEngine(engine);
        return std::make_pair(swaption, swap);
    }

    ext::shared_ptr<OvernightIndexedSwap> makeObservationShiftedSwap(
        const Date& start,
        const Date& end,
        Swap::Type type,
        const Calendar& calendar,
        const DayCounter& fixedDayCounter,
        const ext::shared_ptr<OvernightIndex>& overnightIndex,
        Rate strike,
        Natural lookbackDays) {

        Schedule schedule(start, end, 3 * Months, calendar, ModifiedFollowing,
                          ModifiedFollowing, DateGeneration::Forward, false);
        return ext::make_shared<OvernightIndexedSwap>(
            type, 1.0, schedule, strike, fixedDayCounter, overnightIndex, 0.0,
            0, Following, calendar, false, RateAveraging::Compound,
            lookbackDays, 0, true);
    }

    /* Independent constant-parameter Hull-White reference. The Monte Carlo
       side does not use Gsr bonds, numeraires or its state process. Here x is
       the zero-mean Ornstein-Uhlenbeck state under the risk-neutral measure. */
    DiscountFactor hullWhiteDiscountBond(
        const Handle<YieldTermStructure>& curve,
        Time t,
        Time maturity,
        Real x,
        Real reversion,
        Real volatility) {

        const Real b = (1.0 - std::exp(-reversion * (maturity - t))) /
                       reversion;
        const Real shift =
            volatility * volatility / (2.0 * reversion * reversion) *
            std::pow(1.0 - std::exp(-reversion * t), 2);
        const Real variance =
            volatility * volatility / (4.0 * reversion) *
            (1.0 - std::exp(-2.0 * reversion * t));
        return curve->discount(maturity) / curve->discount(t) *
               std::exp(-b * x - b * shift - b * b * variance);
    }

    DiscountFactor hullWhiteDiscountBond(
        const Handle<YieldTermStructure>& curve,
        Time t,
        const Date& maturity,
        Real x,
        Real reversion,
        Real volatility) {
        return hullWhiteDiscountBond(
            curve, t, curve->timeFromReference(maturity), x, reversion,
            volatility);
    }

    Rate conditionalFloatingRate(
        const Handle<YieldTermStructure>& curve,
        const ext::shared_ptr<FloatingRateCoupon>& coupon,
        Time expiry,
        Real x,
        Real reversion,
        Real volatility) {

        if (const auto overnightCoupon =
                ext::dynamic_pointer_cast<OvernightIndexedCoupon>(coupon)) {
            const auto& valueDates = overnightCoupon->valueDates();
            const auto& interestDates = overnightCoupon->interestDates();
            const DayCounter& dayCounter = overnightCoupon->index()->dayCounter();
            const Time rateAccrualTime =
                overnightCoupon->applyObservationShift() &&
                        overnightCoupon->fixingDays() > 0
                    ? dayCounter.yearFraction(valueDates.front(),
                                              valueDates.back())
                    : dayCounter.yearFraction(interestDates.front(),
                                              interestDates.back());
            return (hullWhiteDiscountBond(
                        curve, expiry, valueDates.front(), x,
                        reversion, volatility) /
                        hullWhiteDiscountBond(
                            curve, expiry, valueDates.back(), x,
                            reversion, volatility) -
                    1.0) /
                   rateAccrualTime;
        }

        const auto iborCoupon = ext::dynamic_pointer_cast<IborCoupon>(coupon);
        QL_REQUIRE(iborCoupon != nullptr, "unsupported floating coupon");
        return (hullWhiteDiscountBond(
                    curve, expiry, iborCoupon->fixingValueDate(), x,
                    reversion, volatility) /
                    hullWhiteDiscountBond(
                        curve, expiry, iborCoupon->fixingMaturityDate(), x,
                        reversion, volatility) -
                1.0) /
               iborCoupon->spanningTimeIndexMaturity();
    }

    Real discountedEuropeanSwaptionPayoff(
        const Handle<YieldTermStructure>& discountCurve,
        const ext::shared_ptr<FixedVsFloatingSwap>& swap,
        const Date& expiry,
        Time expiryTime,
        Real x,
        Real pathDiscount,
        Real reversion,
        Real volatility) {

        Real fixedLegNpv = 0.0;
        for (const auto& cashflow : swap->fixedLeg()) {
            const auto coupon = ext::dynamic_pointer_cast<FixedRateCoupon>(cashflow);
            QL_REQUIRE(coupon != nullptr, "unsupported fixed cashflow");
            QL_REQUIRE(coupon->accrualStartDate() >= expiry,
                       "fixed coupon starts before exercise");
            fixedLegNpv += coupon->amount() * hullWhiteDiscountBond(
                discountCurve, expiryTime, coupon->date(), x, reversion,
                volatility);
        }

        const Handle<YieldTermStructure> forwardingCurve =
            swap->iborIndex()->forwardingTermStructure();
        Real floatingLegNpv = 0.0;
        for (const auto& cashflow : swap->floatingLeg()) {
            const auto coupon =
                ext::dynamic_pointer_cast<FloatingRateCoupon>(cashflow);
            QL_REQUIRE(coupon != nullptr, "unsupported floating cashflow");
            QL_REQUIRE(coupon->accrualStartDate() >= expiry,
                       "floating coupon starts before exercise");
            const Rate rate = conditionalFloatingRate(
                forwardingCurve, coupon, expiryTime, x, reversion, volatility);
            // A delayed coupon is projected at exercise and discounted from
            // its actual payment date, consistently with the GSR engine. This
            // deliberately excludes post-exercise payment-delay convexity.
            const Real amount =
                coupon->nominal() * coupon->accrualPeriod() *
                (coupon->gearing() * rate + coupon->spread());
            floatingLegNpv += amount * hullWhiteDiscountBond(
                discountCurve, expiryTime, coupon->date(), x, reversion,
                volatility);
        }

        const Real payerNpv = floatingLegNpv - fixedLegNpv;
        const Real exerciseValue =
            swap->type() == Swap::Payer ? payerNpv : -payerNpv;
        return pathDiscount * std::max(exerciseValue, 0.0);
    }

    Real hullWhiteMonteCarloEuropeanSwaption(
        const Handle<YieldTermStructure>& curve,
        Real reversion,
        Real volatility,
        const ext::shared_ptr<FixedVsFloatingSwap>& swap,
        const Date& expiry,
        Size samplePairs = 65536) {

        const Time expiryTime = curve->timeFromReference(expiry);
        // x_T and the integral of x over [0,T] are sampled jointly from their
        // exact Gaussian distribution, avoiding a time-discretisation error in
        // both the state and the path discount factor.
        const Real varianceX =
            volatility * volatility /
            (2.0 * reversion) *
            (1.0 - std::exp(-2.0 * reversion * expiryTime));
        const Real varianceIntegral =
            volatility * volatility / (reversion * reversion) *
            (expiryTime -
             2.0 * (1.0 - std::exp(-reversion * expiryTime)) / reversion +
             (1.0 - std::exp(-2.0 * reversion * expiryTime)) /
                 (2.0 * reversion));
        const Real covariance =
            volatility * volatility /
            (2.0 * reversion * reversion) *
            std::pow(1.0 - std::exp(-reversion * expiryTime), 2);
        const Real stdDevX = std::sqrt(varianceX);
        const Real integralLoading = covariance / stdDevX;
        const Real residualStdDev = std::sqrt(
            varianceIntegral - integralLoading * integralLoading);

        SobolRsg sobol(2, 42, SobolRsg::JoeKuoD7);
        const InverseCumulativeNormal inverseNormal;
        Real sum = 0.0;
        for (Size i = 0; i < samplePairs; ++i) {
            const auto& sequence = sobol.nextSequence().value;
            const Real z1 = inverseNormal(sequence[0]);
            const Real z2 = inverseNormal(sequence[1]);
            const Real x = stdDevX * z1;
            const Real integral = integralLoading * z1 + residualStdDev * z2;
            const Real pathDiscount =
                curve->discount(expiryTime) *
                std::exp(-integral - 0.5 * varianceIntegral);
            sum += discountedEuropeanSwaptionPayoff(
                curve, swap, expiry, expiryTime, x, pathDiscount, reversion,
                volatility);
            sum += discountedEuropeanSwaptionPayoff(
                curve, swap, expiry, expiryTime, -x,
                curve->discount(expiryTime) *
                    std::exp(integral - 0.5 * varianceIntegral),
                reversion, volatility);
        }
        return sum / (2.0 * samplePairs);
    }

    Handle<YieldTermStructure> gsrMonteCarloCurve(const Date& today) {
        const DayCounter dc = Actual365Fixed();
        std::vector<Date> dates = {today};
        std::vector<Rate> rates = {0.02};
        for (Size i = 1; i <= 15; ++i) {
            dates.push_back(today + i * Years);
            rates.push_back(0.02 + 0.015 * (1.0 - std::exp(-Real(i) / 4.0)));
        }
        return Handle<YieldTermStructure>(
            ext::make_shared<InterpolatedZeroCurve<Linear> >(dates, rates, dc));
    }

    ext::shared_ptr<Gsr> gsrMonteCarloModel(
        const Handle<YieldTermStructure>& curve,
        Real reversion,
        Real volatility) {
        return ext::make_shared<Gsr>(
            curve, std::vector<Date>(), std::vector<Real>{volatility},
            reversion, 12.0);
    }

    void checkGsrIborSwaptionAgainstMonteCarlo() {
        const Date today(30, June, 2025);
        Settings::instance().evaluationDate() = today;
        const Real reversion = 0.03;
        const Real volatility = 0.009;
        const Calendar calendar = TARGET();
        const Handle<YieldTermStructure> curve = gsrMonteCarloCurve(today);
        const auto index = ext::make_shared<Euribor6M>(curve);
        const Date expiry = calendar.adjust(today + 2 * Years);
        const Date start = calendar.advance(expiry, 2 * Days);
        const Date end = calendar.advance(start, 5 * Years);
        const Schedule fixedSchedule(start, end, 1 * Years, calendar,
                                     ModifiedFollowing, ModifiedFollowing,
                                     DateGeneration::Forward, false);
        const Schedule floatingSchedule(start, end, 6 * Months, calendar,
                                        ModifiedFollowing, ModifiedFollowing,
                                        DateGeneration::Forward, false);
        const auto discounting = ext::make_shared<DiscountingSwapEngine>(curve);
        auto probe = ext::make_shared<VanillaSwap>(
            Swap::Payer, 1.0, fixedSchedule, 0.0, Thirty360(Thirty360::BondBasis),
            floatingSchedule, index, 0.0, Actual360());
        probe->setPricingEngine(discounting);
        auto swap = ext::make_shared<VanillaSwap>(
            Swap::Payer, 1.0, fixedSchedule, probe->fairRate(),
            Thirty360(Thirty360::BondBasis), floatingSchedule, index, 0.0,
            Actual360());
        const auto model =
            gsrMonteCarloModel(curve, reversion, volatility);
        Swaption swaption(swap, ext::make_shared<EuropeanExercise>(expiry));
        swaption.setPricingEngine(
            ext::make_shared<Gaussian1dSwaptionEngine>(model, 128, 8.0));

        const Real calculated = swaption.NPV();
        const Real expected = hullWhiteMonteCarloEuropeanSwaption(
            curve, reversion, volatility, swap, expiry);
        BOOST_CHECK_MESSAGE(
            std::fabs(calculated - expected) < 2.0e-6,
            "GSR Ibor swaption price " << calculated
            << " differs from Monte Carlo " << expected);
    }

    void checkGsrSofrSwaptionAgainstMonteCarlo(
        Integer paymentLag,
        Natural lookbackDays,
        bool applyObservationShift) {

        const Date today(30, June, 2025);
        Settings::instance().evaluationDate() = today;
        const Real reversion = 0.03;
        const Real volatility = 0.009;
        const Handle<YieldTermStructure> curve = gsrMonteCarloCurve(today);
        const auto sofr = ext::make_shared<Sofr>(curve);
        const Calendar calendar = sofr->fixingCalendar();
        const Date expiry = calendar.adjust(today + 2 * Years);
        const Date start = calendar.advance(expiry, 10 * Days);
        const Date end = calendar.advance(start, 5 * Years);
        const Schedule schedule(start, end, 1 * Years, calendar,
                                ModifiedFollowing, ModifiedFollowing,
                                DateGeneration::Forward, false);
        const auto discounting = ext::make_shared<DiscountingSwapEngine>(curve);
        auto probe = ext::make_shared<OvernightIndexedSwap>(
            Swap::Payer, 1.0, schedule, 0.0, Actual360(), sofr, 0.0,
            paymentLag, Following, calendar, false, RateAveraging::Compound,
            lookbackDays, 0, applyObservationShift);
        probe->setPricingEngine(discounting);
        auto swap = ext::make_shared<OvernightIndexedSwap>(
            Swap::Payer, 1.0, schedule, probe->fairRate(), Actual360(), sofr,
            0.0, paymentLag, Following, calendar, false,
            RateAveraging::Compound, lookbackDays, 0,
            applyObservationShift);
        const auto firstCoupon =
            ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
                swap->overnightLeg().front());
        QL_REQUIRE(firstCoupon != nullptr, "expected overnight coupon");
        QL_REQUIRE(firstCoupon->valueDates().front() > expiry,
                   "observation period must start after exercise");
        const Date expectedRateStart =
            lookbackDays == Null<Natural>()
                ? firstCoupon->accrualStartDate()
                : calendar.advance(
                      firstCoupon->accrualStartDate(),
                      -static_cast<Integer>(lookbackDays), Days);
        const Date expectedPaymentDate = calendar.advance(
            firstCoupon->accrualEndDate(), paymentLag, Days, Following);
        BOOST_CHECK_EQUAL(firstCoupon->valueDates().front(), expectedRateStart);
        BOOST_CHECK_EQUAL(firstCoupon->date(), expectedPaymentDate);
        const auto model =
            gsrMonteCarloModel(curve, reversion, volatility);
        Swaption swaption(swap, ext::make_shared<EuropeanExercise>(expiry));
        swaption.setPricingEngine(
            ext::make_shared<Gaussian1dSwaptionEngine>(model, 128, 8.0));

        const Real calculated = swaption.NPV();
        const Real expected = hullWhiteMonteCarloEuropeanSwaption(
            curve, reversion, volatility, swap, expiry);
        BOOST_CHECK_MESSAGE(
            std::fabs(calculated - expected) < 2.0e-6,
            "GSR SOFR swaption price " << calculated
            << " differs from Monte Carlo " << expected
            << " (payment lag " << paymentLag << ", lookback "
            << lookbackDays << ", observation shift "
            << applyObservationShift << ")");
    }

}

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(GsrTests)

BOOST_AUTO_TEST_CASE(testGsrProcess) {

    BOOST_TEST_MESSAGE("Testing GSR process...");

    Date refDate = Settings::instance().evaluationDate();

    // constant reversion, constant volatility, test conditional expectation and
    // variance against
    // existing HullWhiteForwardProcess
    // technically we test two representations of the same constant reversion
    // and volatility structure,
    // namely with and without step dates

    Real tol = 1E-8;

    Real reversion = 0.01;
    Real modelvol = 0.01;

    Handle<YieldTermStructure> yts0(ext::shared_ptr<YieldTermStructure>(
        new FlatForward(0, TARGET(), 0.00, Actual365Fixed())));

    std::vector<Date> stepDates0;
    std::vector<Real> vols0(1, modelvol);
    std::vector<Real> reversions0(1, reversion);

    std::vector<Date> stepDates1;
    for (Size i = 1; i < 60; i++)
        stepDates1.push_back(refDate + (i * 6 * Months));
    std::vector<Real> vols1(stepDates1.size() + 1, modelvol);
    std::vector<Real> reversions1(stepDates1.size() + 1, reversion);

    Real T = 10.0;
    do {

        ext::shared_ptr<Gsr> model(
            new Gsr(yts0, stepDates0, vols0, reversions0, T));
        ext::shared_ptr<StochasticProcess1D> gsrProcess =
            model->stateProcess();
        ext::shared_ptr<Gsr> model2(
            new Gsr(yts0, stepDates1, vols1, reversions1, T));
        ext::shared_ptr<StochasticProcess1D> gsrProcess2 =
            model2->stateProcess();

        ext::shared_ptr<HullWhiteForwardProcess> hwProcess(
            new HullWhiteForwardProcess(yts0, reversion, modelvol));
        hwProcess->setForwardMeasureTime(T);

        Real w, t, xw, hwVal, gsrVal, gsr2Val;

        t = 0.5;
        do {
            w = 0.0;
            do {
                xw = -0.1;
                do {
                    hwVal = hwProcess->expectation(w, xw, t - w);
                    gsrVal = gsrProcess->expectation(w, xw, t - w);
                    gsr2Val = gsrProcess2->expectation(w, xw, t - w);
                    if (fabs(hwVal - gsrVal) > tol)
                        BOOST_ERROR(
                            "Expectation E^{T="
                            << T << "}(x(" << t << ") | x(" << w << ") = " << xw
                            << " is different in HullWhiteProcess(" << hwVal
                            << ") and GsrProcess (" << gsrVal << ")");
                    if (fabs(hwVal - gsr2Val) > tol)
                        BOOST_ERROR(
                            "Expectation E^{T="
                            << T << "}(x(" << t << ") | x(" << w << ") = " << xw
                            << " is different in HullWhiteProcess(" << hwVal
                            << ") and GsrProcess2 (" << gsr2Val << ")");

                    hwVal = hwProcess->variance(w, xw, t - w);
                    gsrVal = gsrProcess->variance(w, xw, t - w);
                    gsr2Val = gsrProcess2->variance(w, xw, t - w);
                    if (fabs(hwVal - gsrVal) > tol)
                        BOOST_ERROR("Variance V((x("
                                    << t << ") | x(" << w << ") = " << xw
                                    << " is different in HullWhiteProcess("
                                    << hwVal << ") and GsrProcess (" << gsrVal
                                    << ")");
                    if (fabs(hwVal - gsr2Val) > tol)
                        BOOST_ERROR("Variance V((x("
                                    << t << ") | x(" << w << ") = " << xw
                                    << " is different in HullWhiteProcess("
                                    << hwVal << ") and GsrProcess2 (" << gsr2Val
                                    << ")");
                    xw += 0.01;
                } while (xw <= 0.1);
                w += t / 5.0;
            } while (w <= t - 0.1);
            t += T / 20.0;
        } while (t <= T - 0.1);
        T += 10.0;
    } while (T <= 30.0);

    // time dependent reversion and volatility (test cases to be added)

    Array times(2);
    Array vols(3);
    Array reversions(3);

    times[0] = 1.0;
    times[1] = 2.0;
    vols[0] = 0.2;
    vols[1] = 0.3;
    vols[2] = 0.4;
    reversions[0] = 0.50;
    reversions[1] = 0.80;
    reversions[2] = 1.30;

    GsrProcess p(times, vols, reversions);
    p.setForwardMeasureTime(10.0);

    // add more test cases here ...
}

BOOST_AUTO_TEST_CASE(testGsrModel) {

    BOOST_TEST_MESSAGE("Testing GSR model...");

    Date refDate = Settings::instance().evaluationDate();

    Real modelvol = 0.01;
    Real reversion = 0.01;

    std::vector<Date> stepDates; // no step dates
    std::vector<Real> vols(1, modelvol);
    std::vector<Real> reversions(1, reversion);

    std::vector<Date> stepDates1; // artificial step dates (should yield the
                                  // same result)
    for (Size i = 1; i < 60; i++)
        stepDates1.push_back(refDate + (i * 6 * Months));
    std::vector<Real> vols1(stepDates1.size() + 1, modelvol);
    std::vector<Real> reversions1(stepDates1.size() + 1, reversion);

    Handle<YieldTermStructure> yts(ext::shared_ptr<YieldTermStructure>(
        new FlatForward(0, TARGET(), 0.03, Actual365Fixed())));
    ext::shared_ptr<Gsr> model(
        new Gsr(yts, stepDates, vols, reversions, 50.0));
    ext::shared_ptr<Gsr> model2(
        new Gsr(yts, stepDates1, vols1, reversions1, 50.0));
    ext::shared_ptr<HullWhite> hw(new HullWhite(yts, reversion, modelvol));

    // test zerobond prices against existing HullWhite model
    // technically we test two representations of the same constant reversion
    // and volatility structure,
    // namely with and without step dates

    Real tol0 = 1E-8;

    Real w, t, xw;

    w = 0.1;
    do {
        t = w + 0.1;
        do {
            xw = -0.10;
            do {
                Real yw =
                    (xw - model->stateProcess()->expectation(0.0, 0.0, w)) /
                    model->stateProcess()->stdDeviation(0.0, 0.0, w);
                Real rw = xw + 0.03; // instantaneous forward is 0.03
                Real gsrVal = model->zerobond(t, w, yw);
                Real gsr2Val = model2->zerobond(t, w, yw);
                Real hwVal = hw->discountBond(w, t, rw);
                if (fabs(gsrVal - hwVal) > tol0)
                    BOOST_ERROR("Zerobond P("
                                << w << "," << t << " | x=" << xw << " / y="
                                << yw << ") is different in HullWhite ("
                                << hwVal << ") and Gsr (" << gsrVal << ")");
                if (fabs(gsr2Val - hwVal) > tol0)
                    BOOST_ERROR("Zerobond P("
                                << w << "," << t << " | x=" << xw << " / y="
                                << yw << ") is different in HullWhite ("
                                << hwVal << ") and Gsr2 (" << gsr2Val << ")");
                xw += 0.01;
            } while (xw <= 0.10);
            t += 2.5;
        } while (t <= 50.0);
        w += 5.0;
    } while (w <= 50.0);

    // test standard, nonstandard and jamshidian engine against existing Hull
    // White Jamshidian engine

    Date expiry = TARGET().advance(refDate, 5 * Years);
    Period tenor = 10 * Years;
    ext::shared_ptr<SwapIndex> swpIdx(new EuriborSwapIsdaFixA(tenor, yts));
    Real forward = swpIdx->fixing(expiry);

    ext::shared_ptr<VanillaSwap> underlying = swpIdx->underlyingSwap(expiry);
    ext::shared_ptr<VanillaSwap> underlyingFixed =
        MakeVanillaSwap(10 * Years, swpIdx->iborIndex(), forward)
            .withEffectiveDate(swpIdx->valueDate(expiry))
            .withFixedLegCalendar(swpIdx->fixingCalendar())
            .withFixedLegDayCount(swpIdx->dayCounter())
            .withFixedLegTenor(swpIdx->fixedLegTenor())
            .withFixedLegConvention(swpIdx->fixedLegConvention())
            .withFixedLegTerminationDateConvention(
                 swpIdx->fixedLegConvention());
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(expiry));
    ext::shared_ptr<Swaption> stdswaption(
        new Swaption(underlyingFixed, exercise));
    ext::shared_ptr<NonstandardSwaption> nonstdswaption(
        new NonstandardSwaption(*stdswaption));

    stdswaption->setPricingEngine(ext::shared_ptr<PricingEngine>(
        new JamshidianSwaptionEngine(hw, yts)));
    Real HwJamNpv = stdswaption->NPV();

    nonstdswaption->setPricingEngine(ext::shared_ptr<PricingEngine>(
        new Gaussian1dNonstandardSwaptionEngine(model, 64, 7.0, true, false)));
    stdswaption->setPricingEngine(ext::shared_ptr<PricingEngine>(
        new Gaussian1dSwaptionEngine(model, 64, 7.0, true, false)));
    Real GsrNonStdNpv = nonstdswaption->NPV();
    Real GsrStdNpv = stdswaption->NPV();
    stdswaption->setPricingEngine(ext::shared_ptr<PricingEngine>(
        new Gaussian1dJamshidianSwaptionEngine(model)));
    Real GsrJamNpv = stdswaption->NPV();

    if (fabs(HwJamNpv - GsrNonStdNpv) > 0.00005)
        BOOST_ERROR(
            "Jamshidian HW NPV ("
            << HwJamNpv
            << ") deviates from Gaussian1dNonstandardSwaptionEngine NPV ("
            << GsrNonStdNpv << ")");
    if (fabs(HwJamNpv - GsrStdNpv) > 0.00005)
        BOOST_ERROR("Jamshidian HW NPV ("
                    << HwJamNpv
                    << ") deviates from Gaussian1dSwaptionEngine NPV ("
                    << GsrStdNpv << ")");
    if (fabs(HwJamNpv - GsrJamNpv) > 0.00005)
        BOOST_ERROR("Jamshidian HW NPV ("
                    << HwJamNpv
                    << ") deviates from Gaussian1dJamshidianEngine NPV ("
                    << GsrJamNpv << ")");
}

BOOST_AUTO_TEST_CASE(testGsrProcessWithPathGenerator) {

    BOOST_TEST_MESSAGE("Testing GSR process path generation...");

    // This test verifies that GsrProcess works correctly with PathGenerator.
    // Previously, GsrProcessCore stored references to the input arrays instead
    // of copies, which caused crashes when temporary arrays were passed
    // (common in language bindings like Python/SWIG).

    Size timeSteps = 4;
    Time length = 2.0;

    // Create GsrProcess with temporary Array objects
    // This simulates what happens in SWIG bindings when Python lists are converted
    ext::shared_ptr<GsrProcess> process(
        new GsrProcess(Array(1, 1.0),           // times (temporary)
                       Array(2, 0.005),          // vols (temporary)
                       Array(1, 0.03)));         // reversions (temporary)

    // Create path generator
    typedef PseudoRandom::rsg_type rsg_type;
    typedef PathGenerator<rsg_type>::sample_type sample_type;

    rsg_type rsg = PseudoRandom::make_sequence_generator(timeSteps, 42);
    PathGenerator<rsg_type> generator(process, length, timeSteps, rsg, false);

    // Generate paths - this would crash before the fix due to dangling references
    const sample_type& sample = generator.next();
    const Path& path = sample.value;

    // Verify the path has the expected structure
    BOOST_CHECK_EQUAL(path.length(), timeSteps + 1);
    BOOST_CHECK_EQUAL(path.front(), 0.0);  // x0 is always 0 for GsrProcess

    // Verify path values are finite (not NaN or Inf)
    for (Size i = 0; i < path.length(); ++i) {
        BOOST_CHECK_MESSAGE(std::isfinite(path[i]),
                          "Path value at index " << i << " is not finite: " << path[i]);
    }

    // Generate a few more paths to ensure stability
    for (Size n = 0; n < 10; ++n) {
        const sample_type& s = generator.next();
        for (Size i = 0; i < s.value.length(); ++i) {
            BOOST_CHECK_MESSAGE(std::isfinite(s.value[i]),
                              "Path " << n << " value at index " << i
                              << " is not finite: " << s.value[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(testGsrModelQuoteUpdate) {

    BOOST_TEST_MESSAGE("Testing GSR model when updating quotes...");

    Date refDate = Settings::instance().evaluationDate();

    Real modelvol = 0.01;
    Real reversion = 0.01;

    std::vector<Date> stepDates;
    std::vector<Real> vols = {modelvol};
    std::vector<Real> reversions = {reversion};

    auto rate = ext::make_shared<SimpleQuote>(0.03);

    Handle<YieldTermStructure> yts(ext::make_shared<FlatForward>(0, TARGET(), Handle<Quote>(rate), Actual365Fixed()));
    auto model = ext::make_shared<Gsr>(yts, stepDates, vols, reversions, 50.0);
    auto hw = ext::make_shared<HullWhite>(yts, reversion, modelvol);

    Date expiry = TARGET().advance(refDate, 5 * Years);
    Period tenor = 10 * Years;
    auto swpIdx = ext::make_shared<EuriborSwapIsdaFixA>(tenor, yts);
    Real forward = swpIdx->fixing(expiry);

    ext::shared_ptr<VanillaSwap> underlyingFixed =
        MakeVanillaSwap(10 * Years, swpIdx->iborIndex(), forward)
            .withEffectiveDate(swpIdx->valueDate(expiry))
            .withFixedLegCalendar(swpIdx->fixingCalendar())
            .withFixedLegDayCount(swpIdx->dayCounter())
            .withFixedLegTenor(swpIdx->fixedLegTenor())
            .withFixedLegConvention(swpIdx->fixedLegConvention())
            .withFixedLegTerminationDateConvention(
                 swpIdx->fixedLegConvention());
    auto exercise = ext::make_shared<EuropeanExercise>(expiry);
    auto stdswaption = ext::make_shared<Swaption>(underlyingFixed, exercise);

    stdswaption->setPricingEngine(
        ext::make_shared<Gaussian1dSwaptionEngine>(model, 64, 7.0, true, false));
    BOOST_CHECK_NO_THROW(stdswaption->NPV());
    Real before = stdswaption->NPV();

    BOOST_CHECK_NO_THROW(rate->setValue(0.04));

    Real after = stdswaption->NPV();
    BOOST_CHECK(std::fabs(before - after) > 0.01);
}

BOOST_AUTO_TEST_CASE(testGsrIborSwaptionAgainstMonteCarlo) {
    BOOST_TEST_MESSAGE("Testing GSR Ibor swaption against Monte Carlo...");
    checkGsrIborSwaptionAgainstMonteCarlo();
}

BOOST_AUTO_TEST_CASE(testGsrSofrSwaptionAgainstMonteCarlo) {
    BOOST_TEST_MESSAGE("Testing GSR SOFR swaption against Monte Carlo...");
    checkGsrSofrSwaptionAgainstMonteCarlo(0, Null<Natural>(), false);
}

BOOST_AUTO_TEST_CASE(testGsrSofrPaymentLagSwaptionAgainstMonteCarlo) {
    BOOST_TEST_MESSAGE(
        "Testing GSR payment-lagged SOFR swaption against Monte Carlo...");
    checkGsrSofrSwaptionAgainstMonteCarlo(2, Null<Natural>(), false);
}

BOOST_AUTO_TEST_CASE(testGsrSofrObservationLagSwaptionAgainstMonteCarlo) {
    BOOST_TEST_MESSAGE(
        "Testing GSR observation-lagged SOFR swaption against Monte Carlo...");
    checkGsrSofrSwaptionAgainstMonteCarlo(0, 5, true);
}

BOOST_AUTO_TEST_CASE(testGsrSofrPaymentAndObservationLagSwaptionAgainstMonteCarlo) {
    BOOST_TEST_MESSAGE("Testing GSR payment- and observation-lagged SOFR "
                       "swaption against Monte Carlo...");
    checkGsrSofrSwaptionAgainstMonteCarlo(2, 5, true);
}


BOOST_AUTO_TEST_CASE(testSofrSwaptionObservationConventions) {

    BOOST_TEST_MESSAGE(
        "Testing Gaussian1d telescoping with SOFR observation conventions...");

    Date today(30, June, 2025);
    Settings::instance().evaluationDate() = today;
    Actual365Fixed dc;

    std::vector<Date> curveDates = {today};
    std::vector<Rate> zeroRates = {0.02};
    for (Size i = 1; i <= 12; ++i) {
        curveDates.push_back(today + i * Years);
        zeroRates.push_back(0.02 + 0.015 * (1.0 - std::exp(-Real(i) / 3.0)));
    }
    Handle<YieldTermStructure> curve(
        ext::make_shared<InterpolatedZeroCurve<Linear> >(
            curveDates, zeroRates, dc));

    auto sofr = ext::make_shared<Sofr>(curve);
    Calendar calendar = sofr->fixingCalendar();
    auto gsr = ext::make_shared<Gsr>(
        curve, std::vector<Date>(), std::vector<Real>{0.008}, 0.03, 12.0);
    auto engine = ext::make_shared<Gaussian1dSwaptionEngine>(gsr, 128, 8.0);

    // The exercise date sits two business days before the accrual start, as it
    // does for a real SOFR swaption, so a lookback puts the coupon's first
    // value date on or before the exercise date.
    for (Natural gapDays : {2, 5}) {

        Date expiry = calendar.adjust(today + 2 * Years);
        Date start = calendar.advance(expiry, gapDays * Days);
        Date end = calendar.advance(start, 5 * Years);
        Schedule schedule(start, end, 1 * Years, calendar, ModifiedFollowing,
                          ModifiedFollowing, DateGeneration::Forward, false);
        auto exercise = ext::make_shared<EuropeanExercise>(expiry);

        // Plain compounding and lookback-with-observation-shift both telescope
        // exactly, so both must price.
        const auto [plain, plainSwap] = makeObservationConventionSwaption(
            schedule, dc, sofr, calendar, exercise, engine, Null<Natural>(), 0,
            false);
        const auto [shifted, shiftedSwap] = makeObservationConventionSwaption(
            schedule, dc, sofr, calendar, exercise, engine, 5, 0, true);

        auto shiftedCoupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
            shiftedSwap->overnightLeg().front());
        BOOST_REQUIRE(shiftedCoupon != nullptr);
        BOOST_CHECK(shiftedCoupon->valueDates().front() <
                    shiftedCoupon->accrualStartDate());
        // the lookback is what this test is about: it must reach back past the
        // exercise date, which is the case the projection has to handle
        BOOST_CHECK(shiftedCoupon->valueDates().front() <= expiry);

        Real plainValue = plain->NPV();
        Real shiftedValue = shifted->NPV();
        BOOST_CHECK(plainValue > 0.0);
        BOOST_CHECK(shiftedValue > 0.0);
        // the lookback moves the underlying forward, so the prices must differ
        BOOST_CHECK(std::fabs(shiftedValue - plainValue) > 1.0e-8);

        // Lookback without observation shift and lockout do not telescope
        // exactly; they are approximated by the same endpoint ratio rather than
        // raising, so the engine must still return a finite, positive value.
        for (Real v : {makeObservationConventionSwaption(
                           schedule, dc, sofr, calendar, exercise, engine, 5, 0,
                           false).first->NPV(),
                       makeObservationConventionSwaption(
                           schedule, dc, sofr, calendar, exercise, engine, 5, 2,
                           true).first->NPV()}) {
            BOOST_CHECK(std::isfinite(v));
            BOOST_CHECK(v > 0.0);
        }
    }
}

BOOST_AUTO_TEST_CASE(testSofrSwaptionObservationShiftNormalization) {

    BOOST_TEST_MESSAGE(
        "Testing Gaussian1d observation-shift accrual normalization...");

    // Under an observation shift the coupon divides its compounded factor by the
    // year fraction over the shifted value dates, while its amount multiplies by
    // the contractual accrual period. The engine must keep that accrualPeriod/tau
    // adjustment, otherwise the swaption underlying drifts from the forward swap
    // on periods whose shifted interval has a different calendar length. Put-call
    // parity ties the engine underlying to the DiscountingSwapEngine value.

    Date today(30, June, 2025);
    Settings::instance().evaluationDate() = today;
    Actual360 dc;

    std::vector<Date> curveDates = {today};
    std::vector<Rate> zeroRates = {0.02};
    for (Size i = 1; i <= 12; ++i) {
        curveDates.push_back(today + i * Years);
        zeroRates.push_back(0.02 + 0.015 * (1.0 - std::exp(-Real(i) / 3.0)));
    }
    Handle<YieldTermStructure> curve(
        ext::make_shared<InterpolatedZeroCurve<Linear> >(
            curveDates, zeroRates, dc));

    auto sofr = ext::make_shared<Sofr>(curve);
    Calendar calendar = sofr->fixingCalendar();
    auto gsr = ext::make_shared<Gsr>(
        curve, std::vector<Date>(), std::vector<Real>{0.008}, 0.03, 12.0);
    auto engine = ext::make_shared<Gaussian1dSwaptionEngine>(gsr, 128, 8.0);
    auto discounting = ext::make_shared<DiscountingSwapEngine>(curve);

    const Rate strike = 0.03;
    const Natural lookback = 5;
    // Use a single 3M period whose observation-shift window is holiday-asymmetric
    // (tau != accrualPeriod), so the normalization error does not cancel across
    // coupons. Place the exercise a few weeks before the accrual start so the
    // first value date stays after expiry (no pre-expiry stub in the check).
    Date expiry, start, end;
    ext::shared_ptr<OvernightIndexedCoupon> coupon;
    Real bugMagnitude = 0.0;
    for (Natural wk = 26; wk < 130 && coupon == nullptr; ++wk) {
        Date s = calendar.advance(today, wk, Weeks);
        Date e = calendar.advance(s, 3 * Months);
        Date ex = calendar.advance(s, -3, Weeks);
        auto c = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
            makeObservationShiftedSwap(s, e, Swap::Payer, calendar, dc, sofr,
                                       strike, lookback)->overnightLeg().front());
        Real ap = c->accrualPeriod();
        Real tau = dc.yearFraction(c->valueDates().front(), c->valueDates().back());
        if (std::fabs(ap - tau) > 1.0e-3 && c->valueDates().front() > ex) {
            Real cfm1 = curve->discount(c->valueDates().front()) /
                            curve->discount(c->valueDates().back()) - 1.0;
            bugMagnitude = std::fabs(cfm1 * (1.0 - ap / tau) * curve->discount(c->date()));
            expiry = ex; start = s; end = e; coupon = c;
        }
    }
    BOOST_REQUIRE(coupon != nullptr);
    // the chosen period must actually exercise the bug, else the test is vacuous
    BOOST_REQUIRE(bugMagnitude > 5.0e-5);

    auto exercise = ext::make_shared<EuropeanExercise>(expiry);
    auto forwardSwap = makeObservationShiftedSwap(
        start, end, Swap::Payer, calendar, dc, sofr, strike, lookback);
    forwardSwap->setPricingEngine(discounting);
    const Real reference = forwardSwap->NPV();

    auto payer = ext::make_shared<Swaption>(
        makeObservationShiftedSwap(start, end, Swap::Payer, calendar, dc, sofr,
                                   strike, lookback),
        exercise);
    auto receiver = ext::make_shared<Swaption>(
        makeObservationShiftedSwap(start, end, Swap::Receiver, calendar, dc,
                                   sofr, strike, lookback),
        exercise);
    payer->setPricingEngine(engine);
    receiver->setPricingEngine(engine);

    // put-call parity: payer - receiver = value of the forward payer swap; the
    // old normalization missed this by ~bugMagnitude, so require a tight match
    const Real parity = payer->NPV() - receiver->NPV();
    BOOST_CHECK_MESSAGE(
        std::fabs(parity - reference) < 1.0e-5,
        "Gaussian1d observation-shifted underlying (" << parity
        << ") does not match the DiscountingSwapEngine forward swap ("
        << reference << "), diff " << (parity - reference)
        << " (bug magnitude " << bugMagnitude << ")");
}

BOOST_AUTO_TEST_CASE(testOvernightIndexUnderlyingWithIborCoupons) {

    BOOST_TEST_MESSAGE("Testing Gaussian1dSwaptionEngine with a VanillaSwap "
                       "built on an overnight index...");

    // A VanillaSwap always builds an IborLeg, so an overnight index yields
    // IborCoupons rather than OvernightIndexedCoupons. The engine must project
    // each coupon from its single fixing instead of compounding over the whole
    // schedule period.

    Date today(30, June, 2025);
    Settings::instance().evaluationDate() = today;
    Actual365Fixed dc;
    Handle<YieldTermStructure> curve(
        ext::make_shared<FlatForward>(today, 0.03, dc));

    auto sofr = ext::make_shared<Sofr>(curve);
    Calendar calendar = sofr->fixingCalendar();
    Date expiry = calendar.adjust(today + 2 * Years);
    Date start = calendar.advance(expiry, 2 * Days);
    Date end = calendar.advance(start, 5 * Years);
    Schedule schedule(start, end, 1 * Years, calendar, ModifiedFollowing,
                      ModifiedFollowing, DateGeneration::Forward, false);

    auto gsr = ext::make_shared<Gsr>(
        curve, std::vector<Date>(), std::vector<Real>{0.008}, 0.03, 12.0);
    auto payerSwap = ext::make_shared<VanillaSwap>(
        Swap::Payer, 1.0, schedule, 0.03, dc, schedule, sofr, 0.0, dc);
    BOOST_REQUIRE(ext::dynamic_pointer_cast<OvernightIndexedCoupon>(
                      payerSwap->floatingLeg().front()) == nullptr);

    // Build an otherwise identical one-day Ibor index. Both swaps contain the
    // same IborCoupons and must therefore take the same generic projection path;
    // dispatching on the SOFR index type instead would compound over each annual
    // schedule period and make the prices diverge.
    auto oneDayIndex = ext::make_shared<IborIndex>(
        "SOFR-like", 1 * Days, sofr->fixingDays(), sofr->currency(), calendar,
        Following, false, sofr->dayCounter(), curve);
    auto referenceSwap = ext::make_shared<VanillaSwap>(
        Swap::Payer, 1.0, schedule, 0.03, dc, schedule, oneDayIndex, 0.0, dc);

    auto exercise = ext::make_shared<EuropeanExercise>(expiry);
    auto payer = ext::make_shared<Swaption>(payerSwap, exercise);
    auto referenceSwaption =
        ext::make_shared<Swaption>(referenceSwap, exercise);
    auto engine = ext::make_shared<Gaussian1dSwaptionEngine>(gsr, 128, 8.0);
    payer->setPricingEngine(engine);
    referenceSwaption->setPricingEngine(engine);

    const Real value = payer->NPV();
    const Real reference = referenceSwaption->NPV();
    BOOST_CHECK_MESSAGE(
        std::fabs(value - reference) < 1.0e-10,
        "Gaussian1d price for IborCoupons on SOFR (" << value
        << ") does not match the equivalent one-day Ibor index price ("
        << reference << "), diff " << (value - reference));
}

BOOST_AUTO_TEST_CASE(testSofrSwaptionPaymentLag) {

    BOOST_TEST_MESSAGE("Testing Gaussian1d engines with SOFR payment lag...");

    Date today(30, June, 2025);
    Settings::instance().evaluationDate() = today;
    Actual365Fixed dc;

    std::vector<Date> curveDates = {today};
    std::vector<Rate> zeroRates = {0.025};
    for (Size i = 1; i <= 12; ++i) {
        curveDates.push_back(today + i * Years);
        zeroRates.push_back(0.025 + 0.01 * (1.0 - std::exp(-Real(i) / 3.0)));
    }
    Handle<YieldTermStructure> curve(
        ext::make_shared<InterpolatedZeroCurve<Linear> >(
            curveDates, zeroRates, dc));

    auto sofr = ext::make_shared<Sofr>(curve);
    Calendar calendar = sofr->fixingCalendar();
    Date expiry = calendar.adjust(today + 3 * Years);
    Date start = calendar.advance(expiry, 2 * Days);
    Date end = calendar.advance(start, 5 * Years);
    Schedule schedule(start, end, 1 * Years, calendar, ModifiedFollowing,
                      ModifiedFollowing, DateGeneration::Forward, false);

    auto discountingEngine = ext::make_shared<DiscountingSwapEngine>(curve);
    auto probe = ext::make_shared<OvernightIndexedSwap>(
        Swap::Payer, 1.0, schedule, 0.0, dc, sofr, 0.0, 2, Following,
        calendar);
    probe->setPricingEngine(discountingEngine);
    auto swap = ext::make_shared<OvernightIndexedSwap>(
        Swap::Payer, 1.0, schedule, probe->fairRate(), dc, sofr, 0.0, 2,
        Following, calendar);
    swap->setPricingEngine(discountingEngine);
    BOOST_CHECK_SMALL(swap->NPV(), 1.0e-12);

    for (const auto& cashflow : swap->overnightLeg()) {
        auto coupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(cashflow);
        BOOST_REQUIRE(coupon != nullptr);
        Date expectedPayment =
            calendar.advance(coupon->accrualEndDate(), 2 * Days, Following);
        BOOST_CHECK_EQUAL(coupon->date(), expectedPayment);
        BOOST_CHECK(coupon->date() > coupon->accrualEndDate());
    }

    Real reversion = 0.03, sigma = 0.008;
    auto gsr = ext::make_shared<Gsr>(curve, std::vector<Date>(),
                                     std::vector<Real>{sigma}, reversion, 12.0);
    auto exercise = ext::make_shared<EuropeanExercise>(expiry);
    auto swaption = ext::make_shared<Swaption>(swap, exercise);

    swaption->setPricingEngine(
        ext::make_shared<Gaussian1dSwaptionEngine>(gsr, 128, 8.0));
    Real gaussian = swaption->NPV();
    swaption->setPricingEngine(
        ext::make_shared<Gaussian1dJamshidianSwaptionEngine>(gsr));
    Real gaussianJamshidian = swaption->NPV();

    BOOST_CHECK_CLOSE(gaussianJamshidian, gaussian, 0.2);
    // Conversion to a NonstandardSwap must preserve the payment lag, and the
    // nonstandard Gaussian engine must reproduce the standard engine.
    auto nonstandardSwap = ext::make_shared<NonstandardSwap>(*swap);
    nonstandardSwap->setPricingEngine(discountingEngine);
    BOOST_CHECK_EQUAL(nonstandardSwap->paymentLag(), 2);
    BOOST_CHECK_SMALL(nonstandardSwap->NPV() - swap->NPV(), 1.0e-12);
    auto nonstandardSwaption =
        ext::make_shared<NonstandardSwaption>(nonstandardSwap, exercise);
    nonstandardSwaption->setPricingEngine(
        ext::make_shared<Gaussian1dNonstandardSwaptionEngine>(
            gsr, 128, 8.0, true, false, Handle<Quote>(), curve));
    BOOST_CHECK_CLOSE(nonstandardSwaption->NPV(), gaussian, 0.2);

    // Exercise the same path with variable notionals and margins.
    Size coupons = schedule.size() - 1;
    std::vector<Real> fixedNominals(coupons), floatingNominals(coupons),
        fixedRates(coupons, probe->fairRate()), gearings(coupons, 1.0);
    std::vector<Spread> spreads(coupons);
    for (Size i = 0; i < coupons; ++i) {
        fixedNominals[i] = 1.0 - 0.05 * i;
        floatingNominals[i] = 1.0 - 0.04 * i;
        spreads[i] = 0.0001 * i;
    }
    auto variableSwap = ext::make_shared<NonstandardSwap>(
        Swap::Payer, fixedNominals, floatingNominals, schedule, fixedRates, dc,
        schedule, sofr, gearings, spreads, dc, false, false,
        std::optional<BusinessDayConvention>(Following), 2, calendar);
    variableSwap->setPricingEngine(discountingEngine);
    BOOST_CHECK(std::isfinite(variableSwap->NPV()));
    for (const auto& cashflow : variableSwap->floatingLeg()) {
        auto coupon = ext::dynamic_pointer_cast<OvernightIndexedCoupon>(cashflow);
        BOOST_REQUIRE(coupon != nullptr);
        BOOST_CHECK_EQUAL(
            coupon->date(),
            calendar.advance(coupon->accrualEndDate(), 2 * Days, Following));
    }
    auto variableSwaption =
        ext::make_shared<NonstandardSwaption>(variableSwap, exercise);
    variableSwaption->setPricingEngine(
        ext::make_shared<Gaussian1dNonstandardSwaptionEngine>(
            gsr, 128, 8.0, true, false, Handle<Quote>(), curve));
    Real payerValue = variableSwaption->NPV();
    BOOST_CHECK(std::isfinite(payerValue));
    BOOST_CHECK(payerValue >= 0.0);

    std::vector<Real> scaledFixedNominals = fixedNominals;
    std::vector<Real> scaledFloatingNominals = floatingNominals;
    for (Size i = 0; i < coupons; ++i) {
        scaledFixedNominals[i] *= 2.0;
        scaledFloatingNominals[i] *= 2.0;
    }
    auto scaledSwap = ext::make_shared<NonstandardSwap>(
        Swap::Payer, scaledFixedNominals, scaledFloatingNominals, schedule,
        fixedRates, dc, schedule, sofr, gearings, spreads, dc, false, false,
        std::optional<BusinessDayConvention>(Following), 2, calendar);
    auto scaledSwaption =
        ext::make_shared<NonstandardSwaption>(scaledSwap, exercise);
    scaledSwaption->setPricingEngine(
        ext::make_shared<Gaussian1dNonstandardSwaptionEngine>(
            gsr, 128, 8.0, true, false, Handle<Quote>(), curve));
    // Exact notional homogeneity provides a reference value while retaining
    // the variable-notional, variable-margin and payment-lag path. The two
    // prices come from separate integrations, so the check is relative but not
    // at roundoff level (BOOST_CHECK_CLOSE takes a percentage).
    BOOST_CHECK_CLOSE(scaledSwaption->NPV(), 2.0 * payerValue, 1.0e-6);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
