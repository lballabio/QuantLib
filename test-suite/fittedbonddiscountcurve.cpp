/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2019 StatPro Italia srl

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/termstructures/yield/fittedbonddiscountcurve.hpp>
#include <ql/termstructures/yield/nonlinearfittingmethods.hpp>
#include <ql/indexes/ibor/cdor.hpp>
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/canada.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FittedBondDiscountCurveTests)

BOOST_AUTO_TEST_CASE(testEvaluation) {

    BOOST_TEST_MESSAGE("Testing that fitted bond curves work as evaluators...");

    Date today = Settings::instance().evaluationDate();
    ext::shared_ptr<Bond> bond = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0,
                                                                  today + Period(10, Years));
    Handle<Quote> q(ext::make_shared<SimpleQuote>(100.0));

    std::vector<ext::shared_ptr<BondHelper> > helpers(1);
    helpers[0] = ext::make_shared<BondHelper>(q, bond);

    ExponentialSplinesFitting fittingMethod;

    Size maxIterations = 0;
    Array guess(9);
    guess[0] = -51293.44;
    guess[1] = -212240.36;
    guess[2] = 168668.51;
    guess[3] = 88792.74;
    guess[4] = 120712.13;
    guess[5] = -34332.83;
    guess[6] = -66479.66;
    guess[7] = 13605.17;
    guess[8] = 0.0;

    FittedBondDiscountCurve curve(0, TARGET(), helpers, Actual365Fixed(),
                                  fittingMethod, 1e-10, maxIterations, guess);

    BOOST_CHECK_NO_THROW(curve.discount(3.0));
}

BOOST_AUTO_TEST_CASE(testFlatExtrapolation) {

    BOOST_TEST_MESSAGE("Testing fitted bond curve with flat extrapolation...");

    Date asof(15, Jul, 2019);
    Settings::instance().evaluationDate() = asof;

    // market quotes for bonds below
    Real quotes[] = {101.2100, 100.6270, 99.9210, 101.6700};

    std::vector<ext::shared_ptr<Bond> > bonds;

    // EJ5346956
    bonds.push_back(ext::make_shared<FixedRateBond>(
        2, 100.0,
        Schedule(Date(1, Feb, 2013), Date(3, Feb, 2020), 6 * Months, Canada(), Following, Following,
                 DateGeneration::Forward, false, Date(3, Aug, 2013)),
        std::vector<Real>(1, 0.046), ActualActual(ActualActual::ISDA)));

    // EK9689119
    bonds.push_back(ext::make_shared<FixedRateBond>(
        2, 100.0,
        Schedule(Date(12, Jun, 2015), Date(12, Jun, 2020), 6 * Months, Canada(), Following,
                 Following, DateGeneration::Forward, false, Date(12, Dec, 2015)),
        std::vector<Real>(1, 0.0295), ActualActual(ActualActual::ISDA)));

    // AQ1410069
    bonds.push_back(ext::make_shared<FixedRateBond>(
        2, 100.0,
        Schedule(Date(24, Nov, 2017), Date(24, Nov, 2020), 6 * Months, Canada(), Following,
                 Following, DateGeneration::Forward, false, Date(24, May, 2018)),
        std::vector<Real>(1, 0.02689), ActualActual(ActualActual::ISDA)));

    // AM5387676
    bonds.push_back(ext::make_shared<FixedRateBond>(
        2, 100.0,
        Schedule(Date(21, Feb, 2017), Date(21, Feb, 2022), 6 * Months, Canada(), Following,
                 Following, DateGeneration::Forward, false, Date(21, Aug, 2017)),
        std::vector<Real>(1, 0.0338), ActualActual(ActualActual::ISDA)));

    std::vector<ext::shared_ptr<BondHelper> > helpers;

    for (Size i = 0; i < bonds.size(); ++i) {
        helpers.push_back(ext::make_shared<BondHelper>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(quotes[i])), bonds[i]));
    }

    // method1 with the usual extrapolation
    NelsonSiegelFitting method1;

    // method2 extrapoates flat before the first and after the last bond maturity
    NelsonSiegelFitting method2(
        Array(), ext::shared_ptr<OptimizationMethod>(), Array(),
        Actual365Fixed().yearFraction(asof, helpers.front()->bond()->maturityDate()),
        Actual365Fixed().yearFraction(asof, helpers.back()->bond()->maturityDate()));

    // Set a guess that will provoke a "bad" calibration for method1, actually this result was
    // observed as a real calibration outcome given the default guess. The setup was more
    // elaborate though and we do not aim to replicate that here.

    Array guess = { 0.0317, 5.0, -3.6796, 24.1703 };

    // build the fitted bond curves

    ext::shared_ptr<FittedBondDiscountCurve> curve1 = ext::make_shared<FittedBondDiscountCurve>(
        asof, helpers, Actual365Fixed(), method1, 1E-10, 10000, guess);

    ext::shared_ptr<FittedBondDiscountCurve> curve2 = ext::make_shared<FittedBondDiscountCurve>(
        asof, helpers, Actual365Fixed(), method2, 1E-10, 10000, guess);

    curve1->enableExtrapolation();
    curve2->enableExtrapolation();

    // extract the model prices using the two curves

    std::vector<Bond::Price> modelPrices1, modelPrices2;

    ext::shared_ptr<PricingEngine> engine1 =
        ext::make_shared<DiscountingBondEngine>(Handle<YieldTermStructure>(curve1));
    ext::shared_ptr<PricingEngine> engine2 =
        ext::make_shared<DiscountingBondEngine>(Handle<YieldTermStructure>(curve2));

    for (auto& bond : bonds) {
        bond->setPricingEngine(engine1);
        modelPrices1.emplace_back(bond->cleanPrice(), Bond::Price::Clean);
        bond->setPricingEngine(engine2);
        modelPrices2.emplace_back(bond->cleanPrice(), Bond::Price::Clean);
    }
    BOOST_CHECK_EQUAL(curve1->fitResults().errorCode(), EndCriteria::MaxIterations);
    BOOST_CHECK_EQUAL(curve2->fitResults().errorCode(), EndCriteria::MaxIterations);

    // the resulting cost values are similar for both approaches
    // i.e. the fit has a similar quality, I get for example:
    // fitted curve cost1 = 0.0921232
    // fitted curve cost2 = 0.0919438

    // Real cost1 = std::sqrt(curve1->fitResults().minimumCostValue());
    // Real cost2 = std::sqrt(curve2->fitResults().minimumCostValue());

    // It turns out that the model yields are quite close for model1 and model2 while the curve
    // yields are hugely different: for model1 the yields are completely off (>> 100%) while for
    // model2 they are close to the bond model yields, as it should be.
    //
    // The reason why model1 produces reasonable bond yields is that the compounding from the
    // evaluation date to the settlement date of the bonds compensates for the discounting of
    // the bond flows in the "right way", although the level of the curve yields is completely
    // off. I get these results:
    //
    // helper  maturity  market yield model yield 1 model yield 2 curve yield 1 curve yield 2
    //  0      0.556164     0.0235711     0.0235647     0.0235709       8.69643     0.0235709
    //  1      0.912329     0.0222977     0.0231515     0.0231468       5.31326     0.0231466
    //  2       1.36438     0.0272363     0.0254977     0.0255014       3.56288      0.025524
    //  3       2.61096     0.0268932     0.0277398     0.0277418       1.87629     0.0278147

    for (Size i = 0; i < helpers.size(); ++i) {
        Real t = curve1->timeFromReference(helpers[i]->bond()->maturityDate());
        // Real marketYield = bonds[i]->yield(quotes[i], Actual365Fixed(), Continuous, NoFrequency);
        // Real modelYield1 = bonds[i]->yield(modelPrices1[i], Actual365Fixed(), Continuous, NoFrequency);
        Real modelYield2 =
            bonds[i]->yield(modelPrices2[i], Actual365Fixed(), Continuous, NoFrequency);
        Real curveYield1 = curve1->zeroRate(t, Continuous).rate();
        Real curveYield2 = curve2->zeroRate(t, Continuous).rate();

        if (curveYield1 < 1.0) {
            BOOST_ERROR("Expecting huge yield; the test premise might be outdated");
        }
        QL_CHECK_CLOSE(modelYield2, curveYield2, 1.0); // 1.0 percent relative tolerance
    }

    // resetting the guess changes the calibration

    curve1->resetGuess({ 0.02, 0.0, 0.0, 0.0 });

    BOOST_CHECK_EQUAL(curve1->fitResults().errorCode(), EndCriteria::StationaryPoint);

    for (Size i = 0; i < helpers.size(); ++i) {
        Real t = curve1->timeFromReference(helpers[i]->bond()->maturityDate());
        Real modelYield1 = bonds[i]->yield(modelPrices1[i], Actual365Fixed(), Continuous, NoFrequency);
        Real curveYield1 = curve1->zeroRate(t, Continuous).rate();

        QL_CHECK_CLOSE(modelYield1, curveYield1, 6); // somewhat better, within a dozen bps
    }

}

BOOST_AUTO_TEST_CASE(testRequiredGuess) {

    BOOST_TEST_MESSAGE("Testing that fitted bond curves require a guess when given an L2 penalty...");

    Date today = Settings::instance().evaluationDate();
    auto bond1 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(1, Years));
    auto bond2 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(2, Years));
    auto bond3 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(5, Years));
    auto bond4 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(10, Years));

    std::vector<ext::shared_ptr<BondHelper> > helpers(4);
    helpers[0] = ext::make_shared<BondHelper>(makeQuoteHandle(99.0), bond1);
    helpers[1] = ext::make_shared<BondHelper>(makeQuoteHandle(98.0), bond2);
    helpers[2] = ext::make_shared<BondHelper>(makeQuoteHandle(95.0), bond3);
    helpers[3] = ext::make_shared<BondHelper>(makeQuoteHandle(90.0), bond4);

    Array weights = {};
    ext::shared_ptr<OptimizationMethod> optimizer = {};
    Array l2 = { 0.25, 0.25, 0.25, 0.25 };
    NelsonSiegelFitting fittingMethod(weights, optimizer, l2);

    Real accuracy = 1e-10;
    Size maxIterations = 10000;
    FittedBondDiscountCurve curve(0, TARGET(), helpers, Actual365Fixed(),
                                  fittingMethod, accuracy, maxIterations);

    BOOST_CHECK_EXCEPTION(curve.discount(3.0), Error,
                          ExpectedErrorMessage("L2 penalty requires a guess"));
}

BOOST_AUTO_TEST_CASE(testGuessSize) {

    BOOST_TEST_MESSAGE("Testing that fitted bond curves check the guess size when given...");

    Date today = Settings::instance().evaluationDate();
    auto bond1 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(1, Years));
    auto bond2 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(2, Years));
    auto bond3 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(5, Years));
    auto bond4 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(10, Years));

    std::vector<ext::shared_ptr<BondHelper> > helpers(4);
    helpers[0] = ext::make_shared<BondHelper>(makeQuoteHandle(99.0), bond1);
    helpers[1] = ext::make_shared<BondHelper>(makeQuoteHandle(98.0), bond2);
    helpers[2] = ext::make_shared<BondHelper>(makeQuoteHandle(95.0), bond3);
    helpers[3] = ext::make_shared<BondHelper>(makeQuoteHandle(90.0), bond4);

    NelsonSiegelFitting fittingMethod;

    Real accuracy = 1e-10;
    Size maxIterations = 10000;
    Array guess = { 0.01, 0.0, 0.0 };  // too few
    FittedBondDiscountCurve curve(0, TARGET(), helpers, Actual365Fixed(),
                                  fittingMethod, accuracy, maxIterations, guess);

    BOOST_CHECK_EXCEPTION(curve.discount(3.0), Error,
                          ExpectedErrorMessage("wrong size for guess"));
}




BOOST_AUTO_TEST_CASE(testConstraint) {

    BOOST_TEST_MESSAGE("Testing that fitted bond curves respect passed constraint...");

    class FlatZero : public FittedBondDiscountCurve::FittingMethod {
      public:
        FlatZero(Constraint constraint = NoConstraint())
        : FittedBondDiscountCurve::FittingMethod(true,        // constrainAtZero
                                                 Array(),     // weights
                                                 ext::shared_ptr<OptimizationMethod>(),
                                                 Array(),     // l2
                                                 0.0,         // minCutoffTime
                                                 QL_MAX_REAL, //maxCutoffTime
                                                 std::move(constraint)) {}

        std::unique_ptr<FittedBondDiscountCurve::FittingMethod> clone() const override {
            return std::unique_ptr<FittedBondDiscountCurve::FittingMethod>(new FlatZero(*this));
        }

      private:
        Size size() const override { return 1; }

        DiscountFactor discountFunction(const Array& x, Time t) const override {
            Real zeroRate = x[0];
            DiscountFactor d = std::exp(-zeroRate * t);
            return d;
        }
    };

    Date today = Settings::instance().evaluationDate();
    auto bond1 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(1, Years));
    auto bond2 = ext::make_shared<ZeroCouponBond>(3, TARGET(), 100.0, today + Period(2, Years));

    std::vector<ext::shared_ptr<BondHelper>> helpers(2);
    helpers[0] = ext::make_shared<BondHelper>(makeQuoteHandle(101.0), bond1);
    helpers[1] = ext::make_shared<BondHelper>(makeQuoteHandle(102.0), bond2);

    Real accuracy = 1e-10;      // default value
    Size maxIterations = 10000; // default value
    Array guess = {0.01};       // something positive so that initial value is in feasible region
    
    FlatZero unconstrainedMethod;
    FittedBondDiscountCurve unconstrainedCurve(0, TARGET(), helpers, Actual365Fixed(), unconstrainedMethod, 
                                               accuracy, maxIterations, guess);
    BOOST_CHECK_LT(unconstrainedCurve.fitResults().solution()[0], 0.0);

    FlatZero positiveMethod{PositiveConstraint()};
    FittedBondDiscountCurve positiveCurve(0, TARGET(), helpers, Actual365Fixed(), positiveMethod,
                                          accuracy, maxIterations, guess);
    BOOST_CHECK_GT(positiveCurve.fitResults().solution()[0], 0.0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
