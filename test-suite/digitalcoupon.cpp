/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "digitalcoupon.hpp"
#include "utilities.hpp"
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/digitalcoupon.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/settings.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/distributions/normaldistribution.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace digital_coupon_test {

    struct CommonVars {
        // global data
        Date today, settlement;
        Real nominal;
        Calendar calendar;
        ext::shared_ptr<IborIndex> index;
        Natural fixingDays;
        RelinkableHandle<YieldTermStructure> termStructure;
        Real optionTolerance;
        Real blackTolerance;

        // cleanup
        SavedSettings backup;

        // setup
        CommonVars() {
            fixingDays = 2;
            nominal = 1000000.0;
            index = ext::shared_ptr<IborIndex>(new Euribor6M(termStructure));
            calendar = index->fixingCalendar();
            today = calendar.adjust(Settings::instance().evaluationDate());
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today,fixingDays,Days);
            termStructure.linkTo(flatRate(settlement,0.05,Actual365Fixed()));
            optionTolerance = 1.e-04;
            blackTolerance = 1e-10;
        }
    };

}


void DigitalCouponTest::testAssetOrNothing() {

    BOOST_TEST_MESSAGE("Testing European asset-or-nothing digital coupon...");

    using namespace digital_coupon_test;

    /*  Call Payoff = (aL+b)Heaviside(aL+b-X) =  a Max[L-X'] + (b+aX')Heaviside(L-X')
        Value Call = aF N(d1') + bN(d2')
        Put Payoff =  (aL+b)Heaviside(X-aL-b) = -a Max[X-L'] + (b+aX')Heaviside(X'-L)
        Value Put = aF N(-d1') + bN(-d2')
        where:
        d1' = ln(F/X')/stdDev + 0.5*stdDev;
    */

    CommonVars vars;

    Volatility vols[] = { 0.05, 0.15, 0.30 };
    Rate strikes[] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07 };
    Real gearings[] = { 1.0, 2.8 };
    Rate spreads[] = { 0.0, 0.005 };

    Real gap = 1e-7; /* low, in order to compare digital option value
                        with black formula result */
    ext::shared_ptr<DigitalReplication>
        replication(new DigitalReplication(Replication::Central, gap));
    for (Real capletVol : vols) {
        RelinkableHandle<OptionletVolatilityStructure> vol;
        vol.linkTo(ext::shared_ptr<OptionletVolatilityStructure>(new ConstantOptionletVolatility(
            vars.today, vars.calendar, Following, capletVol, Actual360())));
        for (Real strike : strikes) {
            for (Size k = 9; k < 10; k++) {
                Date startDate = vars.calendar.advance(vars.settlement,(k+1)*Years);
                Date endDate = vars.calendar.advance(vars.settlement,(k+2)*Years);
                Rate nullstrike = Null<Rate>();
                Date paymentDate = endDate;
                for (Size h=0; h<LENGTH(gearings); h++) {

                    Real gearing = gearings[h];
                    Rate spread = spreads[h];

                    ext::shared_ptr<FloatingRateCoupon> underlying(new
                        IborCoupon(paymentDate, vars.nominal,
                                   startDate, endDate,
                                   vars.fixingDays, vars.index,
                                   gearing, spread));
                    // Floating Rate Coupon - Call Digital option
                    DigitalCoupon digitalCappedCoupon(underlying,
                                        strike, Position::Short, false, nullstrike,
                                        nullstrike, Position::Short, false, nullstrike,
                                        replication);
                    ext::shared_ptr<IborCouponPricer> pricer(new
                                                            BlackIborCouponPricer(vol));
                    digitalCappedCoupon.setPricer(pricer);

                    // Check digital option price vs N(d1) price
                    Time accrualPeriod = underlying->accrualPeriod();
                    Real discount = vars.termStructure->discount(endDate);
                    Date exerciseDate = underlying->fixingDate();
                    Rate forward = underlying->rate();
                    Rate effFwd = (forward-spread)/gearing;
                    Rate effStrike = (strike-spread)/gearing;
                    Real stdDev = std::sqrt(vol->blackVariance(exerciseDate, effStrike));
                    CumulativeNormalDistribution phi;
                    Real d1 = std::log(effFwd/effStrike)/stdDev + 0.5*stdDev;
                    Real d2 = d1 - stdDev;
                    Real N_d1 = phi(d1);
                    Real N_d2 = phi(d2);
                    Real nd1Price = (gearing * effFwd * N_d1 + spread * N_d2)
                                  * vars.nominal * accrualPeriod * discount;
                    Real optionPrice = digitalCappedCoupon.callOptionRate() *
                                       vars.nominal * accrualPeriod * discount;
                    Real error = std::abs(nd1Price - optionPrice);
                    if (error>vars.optionTolerance)
                        BOOST_ERROR("\nDigital Call Option:" <<
                            "\nVolatility = " << io::rate(capletVol) <<
                            "\nStrike = " << io::rate(strike) <<
                            "\nExercise = " << k+1 << " years" <<
                            "\nOption price by replication = "  << optionPrice <<
                            "\nOption price by Cox-Rubinstein formula = " << nd1Price <<
                            "\nError " << error);

                    // Check digital option price vs N(d1) price using Vanilla Option class
                    if (spread==0.0) {
                        ext::shared_ptr<Exercise>
                            exercise(new EuropeanExercise(exerciseDate));
                        Real discountAtFixing = vars.termStructure->discount(exerciseDate);
                        ext::shared_ptr<SimpleQuote>
                            fwd(new SimpleQuote(effFwd*discountAtFixing));
                        ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
                        ext::shared_ptr<YieldTermStructure>
                            qTS = flatRate(vars.today, qRate, Actual360());
                        ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
                        ext::shared_ptr<BlackVolTermStructure>
                            volTS = flatVol(vars.today, capletVol, Actual360());
                        ext::shared_ptr<StrikedTypePayoff>
                            callPayoff(new AssetOrNothingPayoff(Option::Call,effStrike));
                        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
                            BlackScholesMertonProcess(Handle<Quote>(fwd),
                                              Handle<YieldTermStructure>(qTS),
                                              Handle<YieldTermStructure>(vars.termStructure),
                                              Handle<BlackVolTermStructure>(volTS)));
                        ext::shared_ptr<PricingEngine>
                            engine(new AnalyticEuropeanEngine(stochProcess));
                        VanillaOption callOpt(callPayoff, exercise);
                        callOpt.setPricingEngine(engine);
                        Real callVO = vars.nominal * gearing
                                               * accrualPeriod * callOpt.NPV()
                                               * discount / discountAtFixing
                                               * forward / effFwd;
                        error = std::abs(nd1Price - callVO);
                        if (error>vars.blackTolerance)
                            BOOST_ERROR("\nDigital Call Option:" <<
                            "\nVolatility = " << io::rate(capletVol) <<
                            "\nStrike = " << io::rate(strike) <<
                            "\nExercise = " << k+1 << " years" <<
                            "\nOption price by Black asset-ot-nothing payoff = " << callVO <<
                            "\nOption price by Cox-Rubinstein = " << nd1Price <<
                            "\nError " << error );
                    }

                    // Floating Rate Coupon + Put Digital option
                    DigitalCoupon digitalFlooredCoupon(underlying,
                                        nullstrike, Position::Long, false, nullstrike,
                                        strike, Position::Long, false, nullstrike,
                                        replication);
                    digitalFlooredCoupon.setPricer(pricer);

                    // Check digital option price vs N(d1) price
                    N_d1 = phi(-d1);
                    N_d2 = phi(-d2);
                    nd1Price = (gearing * effFwd * N_d1 + spread * N_d2)
                             * vars.nominal * accrualPeriod * discount;
                    optionPrice = digitalFlooredCoupon.putOptionRate() *
                                  vars.nominal * accrualPeriod * discount;
                    error = std::abs(nd1Price - optionPrice);
                    if (error>vars.optionTolerance)
                        BOOST_ERROR("\nDigital Put Option:" <<
                                    "\nVolatility = " << io::rate(capletVol) <<
                                    "\nStrike = " << io::rate(strike) <<
                                    "\nExercise = " << k+1 << " years" <<
                                    "\nOption price by replication = "  << optionPrice <<
                                    "\nOption price by Cox-Rubinstein = " << nd1Price <<
                                    "\nError " << error );

                    // Check digital option price vs N(d1) price using Vanilla Option class
                    if (spread==0.0) {
                        ext::shared_ptr<Exercise>
                            exercise(new EuropeanExercise(exerciseDate));
                        Real discountAtFixing = vars.termStructure->discount(exerciseDate);
                        ext::shared_ptr<SimpleQuote>
                            fwd(new SimpleQuote(effFwd*discountAtFixing));
                        ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
                        ext::shared_ptr<YieldTermStructure>
                            qTS = flatRate(vars.today, qRate, Actual360());
                        ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
                        ext::shared_ptr<BlackVolTermStructure>
                            volTS = flatVol(vars.today, capletVol, Actual360());
                        ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
                            BlackScholesMertonProcess(Handle<Quote>(fwd),
                                              Handle<YieldTermStructure>(qTS),
                                              Handle<YieldTermStructure>(vars.termStructure),
                                              Handle<BlackVolTermStructure>(volTS)));
                        ext::shared_ptr<StrikedTypePayoff>
                            putPayoff(new AssetOrNothingPayoff(Option::Put, effStrike));
                        ext::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine(stochProcess));
                        VanillaOption putOpt(putPayoff, exercise);
                        putOpt.setPricingEngine(engine);
                        Real putVO  = vars.nominal * gearing
                                               * accrualPeriod * putOpt.NPV()
                                               * discount / discountAtFixing
                                               * forward / effFwd;
                        error = std::abs(nd1Price - putVO);
                        if (error>vars.blackTolerance)
                            BOOST_ERROR("\nDigital Put Option:" <<
                            "\nVolatility = " << io::rate(capletVol) <<
                            "\nStrike = " << io::rate(strike) <<
                            "\nExercise = " << k+1 << " years" <<
                            "\nOption price by Black asset-ot-nothing payoff = " << putVO <<
                            "\nOption price by Cox-Rubinstein = " << nd1Price <<
                            "\nError " << error );
                    }
                }
            }
        }
    }
}

void DigitalCouponTest::testAssetOrNothingDeepInTheMoney() {

    BOOST_TEST_MESSAGE("Testing European deep in-the-money asset-or-nothing "
                       "digital coupon...");

    using namespace digital_coupon_test;

    CommonVars vars;

    Real gearing = 1.0;
    Real spread = 0.0;

    Volatility capletVolatility = 0.0001;
    RelinkableHandle<OptionletVolatilityStructure> volatility;
    volatility.linkTo(ext::shared_ptr<OptionletVolatilityStructure>(new
        ConstantOptionletVolatility(vars.today, vars.calendar, Following,
                                    capletVolatility, Actual360())));
    Real gap = 1e-4;
    ext::shared_ptr<DigitalReplication> replication(new
        DigitalReplication(Replication::Central, gap));

    for (Size k = 0; k<10; k++) {   // Loop on start and end dates
        Date startDate = vars.calendar.advance(vars.settlement,(k+1)*Years);
        Date endDate = vars.calendar.advance(vars.settlement,(k+2)*Years);
        Rate nullstrike = Null<Rate>();
        Date paymentDate = endDate;

        ext::shared_ptr<FloatingRateCoupon> underlying(new
            IborCoupon(paymentDate, vars.nominal,
                       startDate, endDate,
                       vars.fixingDays, vars.index,
                       gearing, spread));

        // Floating Rate Coupon - Deep-in-the-money Call Digital option
        Rate strike = 0.001;
        DigitalCoupon digitalCappedCoupon(underlying,
                                          strike, Position::Short, false, nullstrike,
                                          nullstrike, Position::Short, false, nullstrike,
                                          replication);
        ext::shared_ptr<IborCouponPricer> pricer(new
            BlackIborCouponPricer(volatility));
        digitalCappedCoupon.setPricer(pricer);

        // Check price vs its target price
        Time accrualPeriod = underlying->accrualPeriod();
        Real discount = vars.termStructure->discount(endDate);

        Real targetOptionPrice = underlying->price(vars.termStructure);
        Real targetPrice = 0.0;
        Real digitalPrice = digitalCappedCoupon.price(vars.termStructure);
        Real error = std::fabs(targetPrice - digitalPrice);
        Real tolerance = 1e-08;
        if (error>tolerance)
            BOOST_ERROR("\nFloating Coupon - Digital Call Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nCoupon Price = "  << digitalPrice <<
                        "\nTarget price = " << targetPrice <<
                        "\nError = " << error );

        // Check digital option price
        Real replicationOptionPrice = digitalCappedCoupon.callOptionRate() *
                                      vars.nominal * accrualPeriod * discount;
        error = std::abs(targetOptionPrice - replicationOptionPrice);
        Real optionTolerance = 1e-08;
        if (error>optionTolerance)
            BOOST_ERROR("\nDigital Call Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nPrice by replication = " << replicationOptionPrice <<
                        "\nTarget price = " << targetOptionPrice <<
                        "\nError = " << error);

        // Floating Rate Coupon + Deep-in-the-money Put Digital option
        strike = 0.99;
        DigitalCoupon digitalFlooredCoupon(underlying,
                                           nullstrike, Position::Long, false, nullstrike,
                                           strike, Position::Long, false, nullstrike,
                                           replication);
        digitalFlooredCoupon.setPricer(pricer);

        // Check price vs its target price
        targetOptionPrice = underlying->price(vars.termStructure);
        targetPrice = underlying->price(vars.termStructure) + targetOptionPrice ;
        digitalPrice = digitalFlooredCoupon.price(vars.termStructure);
        error = std::fabs(targetPrice - digitalPrice);
        tolerance = 2.5e-06;
        if (error>tolerance)
            BOOST_ERROR("\nFloating Coupon + Digital Put Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nDigital coupon price = "  << digitalPrice <<
                        "\nTarget price = " << targetPrice <<
                        "\nError " << error);

        // Check digital option
        replicationOptionPrice = digitalFlooredCoupon.putOptionRate() *
                                 vars.nominal * accrualPeriod * discount;
        error = std::abs(targetOptionPrice - replicationOptionPrice);
        optionTolerance = 2.5e-06;
        if (error>optionTolerance)
            BOOST_ERROR("\nDigital Put Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nPrice by replication = " << replicationOptionPrice <<
                        "\nTarget price = " << targetOptionPrice <<
                        "\nError " << error);
    }
}

void DigitalCouponTest::testAssetOrNothingDeepOutTheMoney() {

    BOOST_TEST_MESSAGE("Testing European deep out-the-money asset-or-nothing "
                       "digital coupon...");

    using namespace digital_coupon_test;

    CommonVars vars;

    Real gearing = 1.0;
    Real spread = 0.0;

    Volatility capletVolatility = 0.0001;
    RelinkableHandle<OptionletVolatilityStructure> volatility;
    volatility.linkTo(ext::shared_ptr<OptionletVolatilityStructure>(new
        ConstantOptionletVolatility(vars.today, vars.calendar, Following,
                                    capletVolatility, Actual360())));
    Real gap = 1e-4;
    ext::shared_ptr<DigitalReplication>
        replication(new DigitalReplication(Replication::Central, gap));

    for (Size k = 0; k<10; k++) { // loop on start and end dates
        Date startDate = vars.calendar.advance(vars.settlement,(k+1)*Years);
        Date endDate = vars.calendar.advance(vars.settlement,(k+2)*Years);
        Rate nullstrike = Null<Rate>();
        Date paymentDate = endDate;

        ext::shared_ptr<FloatingRateCoupon> underlying(new
            IborCoupon(paymentDate, vars.nominal,
                       startDate, endDate,
                       vars.fixingDays, vars.index,
                       gearing, spread));

        // Floating Rate Coupon - Deep-out-of-the-money Call Digital option
        Rate strike = 0.99;
        DigitalCoupon digitalCappedCoupon(underlying,
                                          strike, Position::Short, false, nullstrike,
                                          nullstrike, Position::Long, false, nullstrike,
                                          replication/*Replication::Central, gap*/);
        ext::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(volatility));
        digitalCappedCoupon.setPricer(pricer);

        // Check price vs its target
        Time accrualPeriod = underlying->accrualPeriod();
        Real discount = vars.termStructure->discount(endDate);

        Real targetPrice = underlying->price(vars.termStructure);
        Real digitalPrice = digitalCappedCoupon.price(vars.termStructure);
        Real error = std::fabs(targetPrice - digitalPrice);
        Real tolerance = 1e-10;
        if (error>tolerance)
            BOOST_ERROR("\nFloating Coupon - Digital Call Option :" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nCoupon price = "  << digitalPrice <<
                        "\nTarget price = " << targetPrice <<
                        "\nError = " << error );

        // Check digital option price
        Real targetOptionPrice = 0.;
        Real replicationOptionPrice = digitalCappedCoupon.callOptionRate() *
                                      vars.nominal * accrualPeriod * discount;
        error = std::abs(targetOptionPrice - replicationOptionPrice);
        Real optionTolerance = 1e-08;
        if (error>optionTolerance)
            BOOST_ERROR("\nDigital Call Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nPrice by replication = "  << replicationOptionPrice <<
                        "\nTarget price = " << targetOptionPrice <<
                        "\nError = " << error );

        // Floating Rate Coupon - Deep-out-of-the-money Put Digital option
        strike = 0.01;
        DigitalCoupon digitalFlooredCoupon(underlying,
                                           nullstrike, Position::Long, false, nullstrike,
                                           strike, Position::Long, false, nullstrike,
                                           replication);
        digitalFlooredCoupon.setPricer(pricer);

        // Check price vs its target
        targetPrice = underlying->price(vars.termStructure);
        digitalPrice = digitalFlooredCoupon.price(vars.termStructure);
        tolerance = 1e-08;
        error = std::fabs(targetPrice - digitalPrice);
        if (error>tolerance)
            BOOST_ERROR("\nFloating Coupon + Digital Put Coupon:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nCoupon price = "  << digitalPrice <<
                        "\nTarget price = " << targetPrice <<
                        "\nError = " << error );

        // Check digital option
        targetOptionPrice = 0.0;
        replicationOptionPrice = digitalFlooredCoupon.putOptionRate() *
                                 vars.nominal * accrualPeriod * discount;
        error = std::abs(targetOptionPrice - replicationOptionPrice);
        if (error>optionTolerance)
            BOOST_ERROR("\nDigital Put Coupon:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nPrice by replication = " << replicationOptionPrice <<
                        "\nTarget price = " << targetOptionPrice <<
                        "\nError = " << error );
    }
}

void DigitalCouponTest::testCashOrNothing() {

    BOOST_TEST_MESSAGE("Testing European cash-or-nothing digital coupon...");

    /*  Call Payoff = R Heaviside(aL+b-X)
        Value Call = R N(d2')
        Put Payoff =  R Heaviside(X-aL-b)
        Value Put = R N(-d2')
        where:
        d2' = ln(F/X')/stdDev - 0.5*stdDev;
    */

    using namespace digital_coupon_test;

    CommonVars vars;

    Volatility vols[] = { 0.05, 0.15, 0.30 };
    Rate strikes[] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07 };

    Real gearing = 3.0;
    Real spread = -0.0002;

    Real gap = 1e-08; /* very low, in order to compare digital option value
                                     with black formula result */
    ext::shared_ptr<DigitalReplication> replication(new
        DigitalReplication(Replication::Central, gap));

    for (Real capletVol : vols) {
        RelinkableHandle<OptionletVolatilityStructure> vol;
        vol.linkTo(ext::shared_ptr<OptionletVolatilityStructure>(new ConstantOptionletVolatility(
            vars.today, vars.calendar, Following, capletVol, Actual360())));
        for (Real strike : strikes) {
            for (Size k = 0; k < 10; k++) {
                Date startDate = vars.calendar.advance(vars.settlement,(k+1)*Years);
                Date endDate = vars.calendar.advance(vars.settlement,(k+2)*Years);
                Rate nullstrike = Null<Rate>();
                Rate cashRate = 0.01;

                Date paymentDate = endDate;
                ext::shared_ptr<FloatingRateCoupon> underlying(new
                    IborCoupon(paymentDate, vars.nominal,
                               startDate, endDate,
                               vars.fixingDays, vars.index,
                               gearing, spread));
                // Floating Rate Coupon - Call Digital option
                DigitalCoupon digitalCappedCoupon(underlying,
                                          strike, Position::Short, false, cashRate,
                                          nullstrike, Position::Short, false, nullstrike,
                                          replication);
                ext::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(vol));
                digitalCappedCoupon.setPricer(pricer);

                // Check digital option price vs N(d2) price
                Date exerciseDate = underlying->fixingDate();
                Rate forward = underlying->rate();
                Rate effFwd = (forward-spread)/gearing;
                Rate effStrike = (strike-spread)/gearing;
                Time accrualPeriod = underlying->accrualPeriod();
                Real discount = vars.termStructure->discount(endDate);
                Real stdDev = std::sqrt(vol->blackVariance(exerciseDate, effStrike));
                Real ITM = blackFormulaCashItmProbability(Option::Call, effStrike,
                                                          effFwd, stdDev);
                Real nd2Price = ITM * vars.nominal * accrualPeriod * discount * cashRate;
                Real optionPrice = digitalCappedCoupon.callOptionRate() *
                                   vars.nominal * accrualPeriod * discount;
                Real error = std::abs(nd2Price - optionPrice);
                if (error>vars.optionTolerance)
                    BOOST_ERROR("\nDigital Call Option:" <<
                                "\nVolatility = " << io::rate(capletVol) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise = " << k+1 << " years" <<
                                "\nPrice by replication = " << optionPrice <<
                                "\nPrice by Reiner-Rubinstein = " << nd2Price <<
                                "\nError = " << error );

                // Check digital option price vs N(d2) price using Vanilla Option class
                ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));
                Real discountAtFixing = vars.termStructure->discount(exerciseDate);
                ext::shared_ptr<SimpleQuote> fwd(new SimpleQuote(effFwd*discountAtFixing));
                ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
                ext::shared_ptr<YieldTermStructure> qTS = flatRate(vars.today, qRate, Actual360());
                ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
                ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(vars.today, capletVol,
                                                                         Actual360());
                ext::shared_ptr<StrikedTypePayoff> callPayoff(new CashOrNothingPayoff(
                                                        Option::Call, effStrike, cashRate));
                ext::shared_ptr<BlackScholesMertonProcess> stochProcess(new
                BlackScholesMertonProcess(Handle<Quote>(fwd),
                                          Handle<YieldTermStructure>(qTS),
                                          Handle<YieldTermStructure>(vars.termStructure),
                                          Handle<BlackVolTermStructure>(volTS)));
                ext::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine(stochProcess));
                VanillaOption callOpt(callPayoff, exercise);
                callOpt.setPricingEngine(engine);
                Real callVO = vars.nominal * accrualPeriod * callOpt.NPV()
                                       * discount / discountAtFixing;
                error = std::abs(nd2Price - callVO);
                if (error>vars.blackTolerance)
                    BOOST_ERROR("\nDigital Call Option:" <<
                        "\nVolatility = " << io::rate(capletVol) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nOption price by Black asset-ot-nothing payoff = " << callVO <<
                        "\nOption price by Reiner-Rubinstein = " << nd2Price <<
                        "\nError " << error );

                // Floating Rate Coupon + Put Digital option
                DigitalCoupon digitalFlooredCoupon(underlying,
                                          nullstrike, Position::Long, false, nullstrike,
                                          strike, Position::Long, false, cashRate,
                                          replication);
                digitalFlooredCoupon.setPricer(pricer);


                // Check digital option price vs N(d2) price
                ITM = blackFormulaCashItmProbability(Option::Put,
                                                     effStrike,
                                                     effFwd,
                                                     stdDev);
                nd2Price = ITM * vars.nominal * accrualPeriod * discount * cashRate;
                optionPrice = digitalFlooredCoupon.putOptionRate() *
                              vars.nominal * accrualPeriod * discount;
                error = std::abs(nd2Price - optionPrice);
                if (error>vars.optionTolerance)
                    BOOST_ERROR("\nPut Digital Option:" <<
                                "\nVolatility = " << io::rate(capletVol) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise = " << k+1 << " years" <<
                                "\nPrice by replication = "  << optionPrice <<
                                "\nPrice by Reiner-Rubinstein = " << nd2Price <<
                                "\nError = " << error );

                // Check digital option price vs N(d2) price using Vanilla Option class
                ext::shared_ptr<StrikedTypePayoff> putPayoff(new
                    CashOrNothingPayoff(Option::Put, effStrike, cashRate));
                VanillaOption putOpt(putPayoff, exercise);
                putOpt.setPricingEngine(engine);
                Real putVO  = vars.nominal * accrualPeriod * putOpt.NPV()
                                       * discount / discountAtFixing;
                error = std::abs(nd2Price - putVO);
                if (error>vars.blackTolerance)
                    BOOST_ERROR("\nDigital Put Option:" <<
                        "\nVolatility = " << io::rate(capletVol) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nOption price by Black asset-ot-nothing payoff = "  << putVO <<
                        "\nOption price by Reiner-Rubinstein = " << nd2Price <<
                        "\nError " << error );
            }
        }
    }
}

void DigitalCouponTest::testCashOrNothingDeepInTheMoney() {

    BOOST_TEST_MESSAGE("Testing European deep in-the-money cash-or-nothing "
                       "digital coupon...");

    using namespace digital_coupon_test;

    CommonVars vars;

    Real gearing = 1.0;
    Real spread = 0.0;

    Volatility capletVolatility = 0.0001;
    RelinkableHandle<OptionletVolatilityStructure> volatility;
    volatility.linkTo(ext::shared_ptr<OptionletVolatilityStructure>(new
        ConstantOptionletVolatility(vars.today, vars.calendar, Following,
                                    capletVolatility, Actual360())));

    for (Size k = 0; k<10; k++) {   // Loop on start and end dates
        Date startDate = vars.calendar.advance(vars.settlement,(k+1)*Years);
        Date endDate = vars.calendar.advance(vars.settlement,(k+2)*Years);
        Rate nullstrike = Null<Rate>();
        Rate cashRate = 0.01;
        Real gap = 1e-4;
        ext::shared_ptr<DigitalReplication> replication(new
            DigitalReplication(Replication::Central, gap));
        Date paymentDate = endDate;

        ext::shared_ptr<FloatingRateCoupon> underlying(new
            IborCoupon(paymentDate, vars.nominal,
                       startDate, endDate,
                       vars.fixingDays, vars.index,
                       gearing, spread));
        // Floating Rate Coupon - Deep-in-the-money Call Digital option
        Rate strike = 0.001;
        DigitalCoupon digitalCappedCoupon(underlying,
                                          strike, Position::Short, false, cashRate,
                                          nullstrike, Position::Short, false, nullstrike,
                                          replication);
        ext::shared_ptr<IborCouponPricer> pricer(new
            BlackIborCouponPricer(volatility));
        digitalCappedCoupon.setPricer(pricer);

        // Check price vs its target
        Time accrualPeriod = underlying->accrualPeriod();
        Real discount = vars.termStructure->discount(endDate);

        Real targetOptionPrice = cashRate * vars.nominal * accrualPeriod * discount;
        Real targetPrice = underlying->price(vars.termStructure) - targetOptionPrice;
        Real digitalPrice = digitalCappedCoupon.price(vars.termStructure);

        Real error = std::fabs(targetPrice - digitalPrice);
        Real tolerance = 1e-07;
        if (error>tolerance)
            BOOST_ERROR("\nFloating Coupon - Digital Call Coupon:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nCoupon price = "  << digitalPrice <<
                        "\nTarget price = " << targetPrice <<
                        "\nError " << error );

        // Check digital option price
        Real replicationOptionPrice = digitalCappedCoupon.callOptionRate() *
                                      vars.nominal * accrualPeriod * discount;
        error = std::abs(targetOptionPrice - replicationOptionPrice);
        Real optionTolerance = 1e-07;
        if (error>optionTolerance)
            BOOST_ERROR("\nDigital Call Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nPrice by replication = " << replicationOptionPrice <<
                        "\nTarget price = " << targetOptionPrice <<
                        "\nError = " << error);

        // Floating Rate Coupon + Deep-in-the-money Put Digital option
        strike = 0.99;
        DigitalCoupon digitalFlooredCoupon(underlying,
                                           nullstrike, Position::Long, false, nullstrike,
                                           strike, Position::Long, false, cashRate,
                                           replication);
        digitalFlooredCoupon.setPricer(pricer);

        // Check price vs its target
        targetPrice = underlying->price(vars.termStructure) + targetOptionPrice;
        digitalPrice = digitalFlooredCoupon.price(vars.termStructure);
        error = std::fabs(targetPrice - digitalPrice);
        if (error>tolerance)
            BOOST_ERROR("\nFloating Coupon + Digital Put Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nCoupon price = "  << digitalPrice <<
                        "\nTarget price  = " << targetPrice <<
                        "\nError = " << error );

        // Check digital option
        replicationOptionPrice = digitalFlooredCoupon.putOptionRate() *
                                 vars.nominal * accrualPeriod * discount;
        error = std::abs(targetOptionPrice - replicationOptionPrice);
        if (error>optionTolerance)
            BOOST_ERROR("\nDigital Put Coupon:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nPrice by replication = " << replicationOptionPrice <<
                        "\nTarget price = " << targetOptionPrice <<
                        "\nError = " << error );
    }
}

void DigitalCouponTest::testCashOrNothingDeepOutTheMoney() {

    BOOST_TEST_MESSAGE("Testing European deep out-the-money cash-or-nothing "
                       "digital coupon...");

    using namespace digital_coupon_test;

    CommonVars vars;

    Real gearing = 1.0;
    Real spread = 0.0;

    Volatility capletVolatility = 0.0001;
    RelinkableHandle<OptionletVolatilityStructure> volatility;
    volatility.linkTo(ext::shared_ptr<OptionletVolatilityStructure>(new
        ConstantOptionletVolatility(vars.today, vars.calendar, Following,
                                    capletVolatility, Actual360())));

    for (Size k = 0; k<10; k++) { // loop on start and end dates
        Date startDate = vars.calendar.advance(vars.settlement,(k+1)*Years);
        Date endDate = vars.calendar.advance(vars.settlement,(k+2)*Years);
        Rate nullstrike = Null<Rate>();
        Rate cashRate = 0.01;
        Real gap = 1e-4;
        ext::shared_ptr<DigitalReplication> replication(new
            DigitalReplication(Replication::Central, gap));
        Date paymentDate = endDate;

        ext::shared_ptr<FloatingRateCoupon> underlying(new
            IborCoupon(paymentDate, vars.nominal,
                       startDate, endDate,
                       vars.fixingDays, vars.index,
                       gearing, spread));
        // Deep out-of-the-money Capped Digital Coupon
        Rate strike = 0.99;
        DigitalCoupon digitalCappedCoupon(underlying,
                                          strike, Position::Short, false, cashRate,
                                          nullstrike, Position::Short, false, nullstrike,
                                          replication);

        ext::shared_ptr<IborCouponPricer> pricer(new BlackIborCouponPricer(volatility));
        digitalCappedCoupon.setPricer(pricer);

        // Check price vs its target
        Time accrualPeriod = underlying->accrualPeriod();
        Real discount = vars.termStructure->discount(endDate);

        Real targetPrice = underlying->price(vars.termStructure);
        Real digitalPrice = digitalCappedCoupon.price(vars.termStructure);
        Real error = std::fabs(targetPrice - digitalPrice);
        Real tolerance = 1e-10;
        if (error>tolerance)
            BOOST_ERROR("\nFloating Coupon + Digital Call Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nCoupon price = "  << digitalPrice <<
                        "\nTarget price  = " << targetPrice <<
                        "\nError = " << error );

        // Check digital option price
        Real targetOptionPrice = 0.;
        Real replicationOptionPrice = digitalCappedCoupon.callOptionRate() *
                                      vars.nominal * accrualPeriod * discount;
        error = std::abs(targetOptionPrice - replicationOptionPrice);
        Real optionTolerance = 1e-10;
        if (error>optionTolerance)
            BOOST_ERROR("\nDigital Call Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nPrice by replication = "  << replicationOptionPrice <<
                        "\nTarget price = " << targetOptionPrice <<
                        "\nError = " << error );

        // Deep out-of-the-money Floored Digital Coupon
        strike = 0.01;
        DigitalCoupon digitalFlooredCoupon(underlying,
                                           nullstrike, Position::Long, false, nullstrike,
                                           strike, Position::Long, false, cashRate,
                                           replication);
        digitalFlooredCoupon.setPricer(pricer);

        // Check price vs its target
        targetPrice = underlying->price(vars.termStructure);
        digitalPrice = digitalFlooredCoupon.price(vars.termStructure);
        tolerance = 1e-09;
        error = std::fabs(targetPrice - digitalPrice);
        if (error>tolerance)
            BOOST_ERROR("\nDigital Floored Coupon:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nCoupon price = "  << digitalPrice <<
                        "\nTarget price  = " << targetPrice <<
                        "\nError = " << error );

        // Check digital option
        targetOptionPrice = 0.0;
        replicationOptionPrice = digitalFlooredCoupon.putOptionRate() *
                                 vars.nominal * accrualPeriod * discount;
        error = std::abs(targetOptionPrice - replicationOptionPrice);
        if (error>optionTolerance)
            BOOST_ERROR("\nDigital Put Option:" <<
                        "\nVolatility = " << io::rate(capletVolatility) <<
                        "\nStrike = " << io::rate(strike) <<
                        "\nExercise = " << k+1 << " years" <<
                        "\nPrice by replication " << replicationOptionPrice <<
                        "\nTarget price " << targetOptionPrice <<
                        "\nError " << error );
    }
}


void DigitalCouponTest::testCallPutParity() {

    BOOST_TEST_MESSAGE("Testing call/put parity for European digital coupon...");

    using namespace digital_coupon_test;

    CommonVars vars;

    Volatility vols[] = { 0.05, 0.15, 0.30 };
    Rate strikes[] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07 };

    Real gearing = 1.0;
    Real spread = 0.0;

    Real gap = 1e-04;
    ext::shared_ptr<DigitalReplication> replication(new
        DigitalReplication(Replication::Central, gap));

    for (Real capletVolatility : vols) {
        RelinkableHandle<OptionletVolatilityStructure> volatility;
        volatility.linkTo(
            ext::shared_ptr<OptionletVolatilityStructure>(new ConstantOptionletVolatility(
                vars.today, vars.calendar, Following, capletVolatility, Actual360())));
        for (Real strike : strikes) {
            for (Size k = 0; k < 10; k++) {
                Date startDate = vars.calendar.advance(vars.settlement,(k+1)*Years);
                Date endDate = vars.calendar.advance(vars.settlement,(k+2)*Years);
                Rate nullstrike = Null<Rate>();

                Date paymentDate = endDate;

                ext::shared_ptr<FloatingRateCoupon> underlying(new
                    IborCoupon(paymentDate, vars.nominal,
                               startDate, endDate,
                               vars.fixingDays, vars.index,
                               gearing, spread));
                // Cash-or-Nothing
                Rate cashRate = 0.01;
                // Floating Rate Coupon + Call Digital option
                DigitalCoupon cash_digitalCallCoupon(underlying,
                                          strike, Position::Long, false, cashRate,
                                          nullstrike, Position::Long, false, nullstrike,
                                          replication);
                ext::shared_ptr<IborCouponPricer> pricer(new
                    BlackIborCouponPricer(volatility));
                cash_digitalCallCoupon.setPricer(pricer);
                // Floating Rate Coupon - Put Digital option
                DigitalCoupon cash_digitalPutCoupon(underlying,
                                          nullstrike, Position::Long, false, nullstrike,
                                          strike, Position::Short, false, cashRate,
                                          replication);

                cash_digitalPutCoupon.setPricer(pricer);
                Real digitalPrice = cash_digitalCallCoupon.price(vars.termStructure) -
                                    cash_digitalPutCoupon.price(vars.termStructure);
                // Target price
                Time accrualPeriod = underlying->accrualPeriod();
                Real discount = vars.termStructure->discount(endDate);
                Real targetPrice = vars.nominal * accrualPeriod *  discount * cashRate;

                Real error = std::fabs(targetPrice - digitalPrice);
                Real tolerance = 1.e-08;
                if (error>tolerance)
                    BOOST_ERROR("\nCash-or-nothing:" <<
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise = " << k+1 << " years" <<
                                "\nPrice = "  << digitalPrice <<
                                "\nTarget Price  = " << targetPrice <<
                                "\nError = " << error );

                // Asset-or-Nothing
                // Floating Rate Coupon + Call Digital option
                DigitalCoupon asset_digitalCallCoupon(underlying,
                                          strike, Position::Long, false, nullstrike,
                                          nullstrike, Position::Long, false, nullstrike,
                                          replication);
                asset_digitalCallCoupon.setPricer(pricer);
                // Floating Rate Coupon - Put Digital option
                DigitalCoupon asset_digitalPutCoupon(underlying,
                                          nullstrike, Position::Long, false, nullstrike,
                                          strike, Position::Short, false, nullstrike,
                                          replication);
                asset_digitalPutCoupon.setPricer(pricer);
                digitalPrice = asset_digitalCallCoupon.price(vars.termStructure) -
                               asset_digitalPutCoupon.price(vars.termStructure);
                // Target price
                targetPrice = vars.nominal *  accrualPeriod *  discount * underlying->rate();
                error = std::fabs(targetPrice - digitalPrice);
                tolerance = 1.e-07;
                if (error>tolerance)
                    BOOST_ERROR("\nAsset-or-nothing:" <<
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise = " << k+1 << " years" <<
                                "\nPrice = "  << digitalPrice <<
                                "\nTarget Price  = " << targetPrice <<
                                "\nError = " << error );
            }
        }
    }
}

void DigitalCouponTest::testReplicationType() {

    BOOST_TEST_MESSAGE("Testing replication type for European digital coupon...");

    using namespace digital_coupon_test;

    CommonVars vars;

    Volatility vols[] = { 0.05, 0.15, 0.30 };
    Rate strikes[] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07 };

    Real gearing = 1.0;
    Real spread = 0.0;

    Real gap = 1e-04;
    ext::shared_ptr<DigitalReplication> subReplication(new
        DigitalReplication(Replication::Sub, gap));
    ext::shared_ptr<DigitalReplication> centralReplication(new
        DigitalReplication(Replication::Central, gap));
    ext::shared_ptr<DigitalReplication> superReplication(new
        DigitalReplication(Replication::Super, gap));

    for (Real capletVolatility : vols) {
        RelinkableHandle<OptionletVolatilityStructure> volatility;
        volatility.linkTo(ext::shared_ptr<OptionletVolatilityStructure>(new
        ConstantOptionletVolatility(vars.today, vars.calendar, Following,
                                    capletVolatility, Actual360())));
        for (Real strike : strikes) {
            for (Size k = 0; k < 10; k++) {
                Date startDate = vars.calendar.advance(vars.settlement,(k+1)*Years);
                Date endDate = vars.calendar.advance(vars.settlement,(k+2)*Years);
                Rate nullstrike = Null<Rate>();

                Date paymentDate = endDate;

                ext::shared_ptr<FloatingRateCoupon> underlying(new
                    IborCoupon(paymentDate, vars.nominal,
                               startDate, endDate,
                               vars.fixingDays, vars.index,
                               gearing, spread));
                // Cash-or-Nothing
                Rate cashRate = 0.005;
                // Floating Rate Coupon + Call Digital option
                DigitalCoupon sub_cash_longDigitalCallCoupon(underlying,
                                          strike, Position::Long, false, cashRate,
                                          nullstrike, Position::Long, false, nullstrike,
                                          subReplication);
                DigitalCoupon central_cash_longDigitalCallCoupon(underlying,
                                          strike, Position::Long, false, cashRate,
                                          nullstrike, Position::Long, false, nullstrike,
                                          centralReplication);
                DigitalCoupon over_cash_longDigitalCallCoupon(underlying,
                                          strike, Position::Long, false, cashRate,
                                          nullstrike, Position::Long, false, nullstrike,
                                          superReplication);
                ext::shared_ptr<IborCouponPricer> pricer(new
                    BlackIborCouponPricer(volatility));
                sub_cash_longDigitalCallCoupon.setPricer(pricer);
                central_cash_longDigitalCallCoupon.setPricer(pricer);
                over_cash_longDigitalCallCoupon.setPricer(pricer);
                Real sub_digitalPrice = sub_cash_longDigitalCallCoupon.price(vars.termStructure);
                Real central_digitalPrice = central_cash_longDigitalCallCoupon.price(vars.termStructure);
                Real over_digitalPrice = over_cash_longDigitalCallCoupon.price(vars.termStructure);
                Real tolerance = 1.e-09;
                if ( ( (sub_digitalPrice > central_digitalPrice) &&
                        std::abs(central_digitalPrice - sub_digitalPrice)>tolerance ) ||
                     ( (central_digitalPrice>over_digitalPrice)  &&
                        std::abs(central_digitalPrice - over_digitalPrice)>tolerance ) )  {
                    BOOST_ERROR("\nCash-or-nothing: Floating Rate Coupon + Call Digital option" <<
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise = " << k+1 << " years" <<
                                std::setprecision(20) <<
                                "\nSub-Replication Price = "  << sub_digitalPrice <<
                                "\nCentral-Replication Price = "  << central_digitalPrice <<
                                "\nOver-Replication Price = "  << over_digitalPrice);
                }

                // Floating Rate Coupon - Call Digital option
                DigitalCoupon sub_cash_shortDigitalCallCoupon(underlying,
                                          strike, Position::Short, false, cashRate,
                                          nullstrike, Position::Long, false, nullstrike,
                                          subReplication);
                DigitalCoupon central_cash_shortDigitalCallCoupon(underlying,
                                          strike, Position::Short, false, cashRate,
                                          nullstrike, Position::Long, false, nullstrike,
                                          centralReplication);
                DigitalCoupon over_cash_shortDigitalCallCoupon(underlying,
                                          strike, Position::Short, false, cashRate,
                                          nullstrike, Position::Long, false, nullstrike,
                                          superReplication);
                sub_cash_shortDigitalCallCoupon.setPricer(pricer);
                central_cash_shortDigitalCallCoupon.setPricer(pricer);
                over_cash_shortDigitalCallCoupon.setPricer(pricer);
                sub_digitalPrice = sub_cash_shortDigitalCallCoupon.price(vars.termStructure);
                central_digitalPrice = central_cash_shortDigitalCallCoupon.price(vars.termStructure);
                over_digitalPrice = over_cash_shortDigitalCallCoupon.price(vars.termStructure);
                if ( ( (sub_digitalPrice > central_digitalPrice) &&
                        std::abs(central_digitalPrice - sub_digitalPrice)>tolerance ) ||
                     ( (central_digitalPrice>over_digitalPrice)  &&
                        std::abs(central_digitalPrice - over_digitalPrice)>tolerance ) )
                    BOOST_ERROR("\nCash-or-nothing: Floating Rate Coupon - Call Digital option" <<
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise = " << k+1 << " years" <<
                                std::setprecision(20) <<
                                "\nSub-Replication Price = "  << sub_digitalPrice <<
                                "\nCentral-Replication Price = "  << central_digitalPrice <<
                                "\nOver-Replication Price = "  << over_digitalPrice);
                // Floating Rate Coupon + Put Digital option
                DigitalCoupon sub_cash_longDigitalPutCoupon(underlying,
                                          nullstrike, Position::Long, false, nullstrike,
                                          strike, Position::Long, false, cashRate,
                                          subReplication);
                DigitalCoupon central_cash_longDigitalPutCoupon(underlying,
                                          nullstrike, Position::Long, false, nullstrike,
                                          strike, Position::Long, false, cashRate,
                                          centralReplication);
                DigitalCoupon over_cash_longDigitalPutCoupon(underlying,
                                          nullstrike, Position::Long, false, nullstrike,
                                          strike, Position::Long, false, cashRate,
                                          superReplication);
                sub_cash_longDigitalPutCoupon.setPricer(pricer);
                central_cash_longDigitalPutCoupon.setPricer(pricer);
                over_cash_longDigitalPutCoupon.setPricer(pricer);
                sub_digitalPrice = sub_cash_longDigitalPutCoupon.price(vars.termStructure);
                central_digitalPrice = central_cash_longDigitalPutCoupon.price(vars.termStructure);
                over_digitalPrice = over_cash_longDigitalPutCoupon.price(vars.termStructure);
                if ( ( (sub_digitalPrice > central_digitalPrice) &&
                        std::abs(central_digitalPrice - sub_digitalPrice)>tolerance ) ||
                     ( (central_digitalPrice>over_digitalPrice)  &&
                        std::abs(central_digitalPrice - over_digitalPrice)>tolerance ) )
                    BOOST_ERROR("\nCash-or-nothing: Floating Rate Coupon + Put Digital option" <<
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise = " << k+1 << " years" <<
                                std::setprecision(20) <<
                                "\nSub-Replication Price = "  << sub_digitalPrice <<
                                "\nCentral-Replication Price = "  << central_digitalPrice <<
                                "\nOver-Replication Price = "  << over_digitalPrice);

                // Floating Rate Coupon - Put Digital option
                DigitalCoupon sub_cash_shortDigitalPutCoupon(underlying,
                                          nullstrike, Position::Long, false, nullstrike,
                                          strike, Position::Short, false, cashRate,
                                          subReplication);
                DigitalCoupon central_cash_shortDigitalPutCoupon(underlying,
                                          nullstrike, Position::Long, false, nullstrike,
                                          strike, Position::Short, false, cashRate,
                                          centralReplication);
                DigitalCoupon over_cash_shortDigitalPutCoupon(underlying,
                                          nullstrike, Position::Long, false, nullstrike,
                                          strike, Position::Short, false, cashRate,
                                          superReplication);
                sub_cash_shortDigitalPutCoupon.setPricer(pricer);
                central_cash_shortDigitalPutCoupon.setPricer(pricer);
                over_cash_shortDigitalPutCoupon.setPricer(pricer);
                sub_digitalPrice = sub_cash_shortDigitalPutCoupon.price(vars.termStructure);
                central_digitalPrice = central_cash_shortDigitalPutCoupon.price(vars.termStructure);
                over_digitalPrice = over_cash_shortDigitalPutCoupon.price(vars.termStructure);
                if ( ( (sub_digitalPrice > central_digitalPrice) &&
                        std::abs(central_digitalPrice - sub_digitalPrice)>tolerance ) ||
                     ( (central_digitalPrice>over_digitalPrice)  &&
                        std::abs(central_digitalPrice - over_digitalPrice)>tolerance ) )
                    BOOST_ERROR("\nCash-or-nothing: Floating Rate Coupon + Call Digital option" <<
                                "\nVolatility = " << io::rate(capletVolatility) <<
                                "\nStrike = " << io::rate(strike) <<
                                "\nExercise = " << k+1 << " years" <<
                                std::setprecision(20) <<
                                "\nSub-Replication Price = "  << sub_digitalPrice <<
                                "\nCentral-Replication Price = "  << central_digitalPrice <<
                                "\nOver-Replication Price = "  << over_digitalPrice);
            }
        }
    }
}


test_suite* DigitalCouponTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Digital coupon tests");
    suite->add(QUANTLIB_TEST_CASE(&DigitalCouponTest::testAssetOrNothing));
    suite->add(QUANTLIB_TEST_CASE(
                       &DigitalCouponTest::testAssetOrNothingDeepInTheMoney));
    suite->add(QUANTLIB_TEST_CASE(
                      &DigitalCouponTest::testAssetOrNothingDeepOutTheMoney));
    suite->add(QUANTLIB_TEST_CASE(&DigitalCouponTest::testCashOrNothing));
    suite->add(QUANTLIB_TEST_CASE(
                        &DigitalCouponTest::testCashOrNothingDeepInTheMoney));
    suite->add(QUANTLIB_TEST_CASE(
                       &DigitalCouponTest::testCashOrNothingDeepOutTheMoney));
    suite->add(QUANTLIB_TEST_CASE(&DigitalCouponTest::testCallPutParity));
    suite->add(QUANTLIB_TEST_CASE(&DigitalCouponTest::testReplicationType));
    return suite;
}
