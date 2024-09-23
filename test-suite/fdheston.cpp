/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2008, 2009, 2014 Klaus Spanderen
  Copyright (C) 2014 Johannes Göttker-Schnetmann

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/math/functional.hpp>
#include <ql/methods/finitedifferences/meshers/fdmhestonvariancemesher.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdblackscholesbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdhestonbarrierengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/localconstantvol.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <tuple>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FdHestonTests)

struct NewBarrierOptionData {
    Barrier::Type barrierType;
    Real barrier;
    Real rebate;
    Option::Type type;
    Real strike;
    Real s;        // spot
    Rate q;        // dividend
    Rate r;        // risk-free rate
    Time t;        // time to maturity
    Volatility v;  // volatility
};

class ParableLocalVolatility : public LocalVolTermStructure {
  public:
    ParableLocalVolatility(
                           const Date& referenceDate,
                           Real s0,
                           Real alpha,
                           const DayCounter& dayCounter)
    : LocalVolTermStructure(referenceDate, NullCalendar(), Following, dayCounter),
      referenceDate_(referenceDate),
      s0_(s0),
      alpha_(alpha) {}

    Date maxDate() const override { return Date::maxDate(); }
    Real minStrike() const override { return 0.0; }
    Real maxStrike() const override { return std::numeric_limits<Real>::max(); }

  protected:
    Volatility localVolImpl(Time, Real s) const override {
        return alpha_*(squared(s0_ - s) + 25.0);
    }

  private:
    const Date referenceDate_;
    const Real s0_, alpha_;
};

struct HestonTestData {
    Real kappa;
    Real theta;
    Real sigma;
    Real rho;
    Real r;
    Real q;
    Real T;
    Real K;
};


BOOST_AUTO_TEST_CASE(testFdmHestonVarianceMesher) {
    BOOST_TEST_MESSAGE("Testing FDM Heston variance mesher...");

    const Date today = Date(22, February, 2018);
    const DayCounter dc = Actual365Fixed();
    Settings::instance().evaluationDate() = today;

    const ext::shared_ptr<HestonProcess> process(
        ext::make_shared<HestonProcess>(
            Handle<YieldTermStructure>(flatRate(0.02, dc)),
            Handle<YieldTermStructure>(flatRate(0.02, dc)),
            Handle<Quote>(ext::make_shared<SimpleQuote>(100.0)),
            0.09, 1.0, 0.09, 0.2, -0.5));

    const ext::shared_ptr<FdmHestonVarianceMesher> mesher
        = ext::make_shared<FdmHestonVarianceMesher>(5, process, 1.0);

    const std::vector<Real> locations = mesher->locations();

    const Real expected[] = {
        0.0, 6.652314e-02, 9.000000e-02, 1.095781e-01, 2.563610e-01
    };

    const Real tol = 1e-6;
    for (Size i=0; i < locations.size(); ++i) {
        const Real diff = std::fabs(expected[i] - locations[i]);

        if (diff > tol) {
            BOOST_ERROR("Failed to reproduce Heston variance mesh"
                        << "\n    calculated: " << locations[i]
                        << "\n    expected:   " << expected[i]
                        << std::scientific
                        << "\n    difference  " << diff
                        << "\n    tolerance:  " << tol);
        }
    }

    const ext::shared_ptr<LocalVolTermStructure> lVol =
        ext::make_shared<LocalConstantVol>(today, 2.5, dc);

    const ext::shared_ptr<FdmHestonLocalVolatilityVarianceMesher> constSlvMesher
        = ext::make_shared<FdmHestonLocalVolatilityVarianceMesher>
              (5, process, lVol, 1.0);

    const Real expectedVol = 2.5 * mesher->volaEstimate();
    const Real calculatedVol = constSlvMesher->volaEstimate();

    const Real diff = std::fabs(calculatedVol - expectedVol);
    if (diff > tol) {
        BOOST_ERROR("Failed to reproduce Heston local volatility "
                "variance estimate"
                    << "\n    calculated: " << calculatedVol
                    << "\n    expected:   " << expectedVol
                    << std::scientific
                    << "\n    difference  " << diff
                    << "\n    tolerance:  " << tol);
    }

    const Real alpha = 0.01;
    const ext::shared_ptr<LocalVolTermStructure> leverageFct
        = ext::make_shared<ParableLocalVolatility>(today, 100.0, alpha, dc);

    const ext::shared_ptr<FdmHestonLocalVolatilityVarianceMesher> slvMesher
        = ext::make_shared<FdmHestonLocalVolatilityVarianceMesher>(
              5, process, leverageFct, 0.5, 1, 0.01);

    const Real initialVolEstimate =
        ext::make_shared<FdmHestonVarianceMesher>(5, process, 0.5, 1, 0.01)->
            volaEstimate();

    // const Real vEst = leverageFct->localVol(0, 100) * initialVolEstimate;
    // Mathematica solution
    //    N[Integrate[
    //      alpha*((100*Exp[vEst*x*Sqrt[0.5]] - 100)^2 + 25)*
    //       PDF[NormalDistribution[0, 1], x], {x ,
    //       InverseCDF[NormalDistribution[0, 1], 0.01],
    //       InverseCDF[NormalDistribution[0, 1], 0.99]}]]

    const Real leverageAvg = 0.455881 / (1-0.02);

    const Real volaEstExpected =
        0.5*(leverageAvg + leverageFct->localVol(0, 100)) * initialVolEstimate;

    const Real volaEstCalculated = slvMesher->volaEstimate();

    if (std::fabs(volaEstExpected - volaEstCalculated) > 0.001) {
        BOOST_ERROR("Failed to reproduce Heston local volatility "
                "variance estimate"
                    << "\n    calculated: " << calculatedVol
                    << "\n    expected:   " << expectedVol
                    << std::scientific
                    << "\n    difference  " << std::fabs(volaEstExpected - volaEstCalculated)
                    << "\n    tolerance:  " << tol);
    }
}

BOOST_AUTO_TEST_CASE(testFdmHestonBarrierVsBlackScholes, *precondition(if_speed(Fast))) {

    BOOST_TEST_MESSAGE("Testing FDM with barrier option in Heston model...");

    NewBarrierOptionData values[] = {
        /* The data below are from
          "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag. 72
        */
        //     barrierType, barrier, rebate,         type, strike,     s,    q,    r,    t,    v
        { Barrier::DownOut,    95.0,    3.0, Option::Call,     90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,    100, 100.0, 0.00, 0.08, 1.00, 0.30},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,    110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,     90, 100.0, 0.00, 0.08, 0.25, 0.25},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,    100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,    110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,     90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,    100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,    110, 100.0, 0.04, 0.08, 0.50, 0.25},

        { Barrier::DownIn,     95.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,    90, 100.0, 0.00, 0.08, 0.25, 0.25},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   100, 100.0, 0.00, 0.08, 0.40, 0.25},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.15},

        { Barrier::DownOut,    95.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,   100, 100.0, 0.00, 0.08, 0.40, 0.35},
        { Barrier::DownOut,    95.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.15},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0, Option::Call,   110, 100.0, 0.00, 0.00, 1.00, 0.20},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30},

        { Barrier::DownIn,     95.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,     95.0,    3.0, Option::Call,   110, 100.0, 0.00, 0.08, 1.00, 0.30},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0, Option::Call,   110, 100.0, 0.04, 0.08, 0.50, 0.30},

        { Barrier::DownOut,    95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},

        { Barrier::DownIn,     95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.25},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   110, 100.0, 0.00, 0.04, 1.00, 0.15},

        { Barrier::DownOut,    95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,    95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownOut,   100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpOut,     105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30},

        { Barrier::DownIn,     95.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,     95.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::DownIn,    100.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 1.00, 0.15},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,    90, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   100, 100.0, 0.04, 0.08, 0.50, 0.30},
        { Barrier::UpIn,      105.0,    3.0,  Option::Put,   110, 100.0, 0.04, 0.08, 0.50, 0.30}
    };
    
    const DayCounter dc = Actual365Fixed();     
    const Date todaysDate(28, March, 2004);
    const Date exerciseDate(28, March, 2005);
    Settings::instance().evaluationDate() = todaysDate;

    Handle<Quote> spot(
            ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> qTS(flatRate(qRate, dc));
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    Handle<YieldTermStructure> rTS(flatRate(rRate, dc));
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    Handle<BlackVolTermStructure> volTS(flatVol(vol, dc));

    ext::shared_ptr<BlackScholesMertonProcess> bsProcess(
                      new BlackScholesMertonProcess(spot, qTS, rTS, volTS));

    ext::shared_ptr<PricingEngine> analyticEngine(
                                        new AnalyticBarrierEngine(bsProcess));

    for (auto& value : values) {
        Date exDate = todaysDate + timeToDays(value.t, 365);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        ext::dynamic_pointer_cast<SimpleQuote>(spot.currentLink())->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(value.type, value.strike));

        BarrierOption barrierOption(value.barrierType, value.barrier, value.rebate, payoff,
                                    exercise);

        const Real v0 = vol->value()*vol->value();
        ext::shared_ptr<HestonProcess> hestonProcess(
             new HestonProcess(rTS, qTS, spot, v0, 1.0, v0, 0.005, 0.0));

        barrierOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new FdHestonBarrierEngine(ext::make_shared<HestonModel>(
                              hestonProcess), 200, 101, 3)));

        const Real calculatedHE = barrierOption.NPV();
    
        barrierOption.setPricingEngine(analyticEngine);
        const Real expected = barrierOption.NPV();
    
        const Real tol = 0.0025;
        if (std::fabs(calculatedHE - expected)/expected > tol) {
            BOOST_ERROR("Failed to reproduce expected Heston npv"
                        << "\n    calculated: " << calculatedHE
                        << "\n    expected:   " << expected
                        << "\n    tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testFdmHestonBarrier) {

    BOOST_TEST_MESSAGE("Testing FDM with barrier option for Heston model vs "
                       "Black-Scholes model...");

    Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    Handle<YieldTermStructure> rTS(flatRate(0.05, Actual365Fixed()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual365Fixed()));

    ext::shared_ptr<HestonProcess> hestonProcess(
        new HestonProcess(rTS, qTS, s0, 0.04, 2.5, 0.04, 0.66, -0.8));

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(28, March, 2005);

    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    ext::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Call, 100));

    BarrierOption barrierOption(Barrier::UpOut, 135, 0.0, payoff, exercise);

    barrierOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
            new FdHestonBarrierEngine(ext::make_shared<HestonModel>(
                              hestonProcess), 50, 400, 100)));

    const Real tol = 0.01;
    const Real npvExpected   =  9.1530;
    const Real deltaExpected =  0.5218;
    const Real gammaExpected = -0.0354;

    if (std::fabs(barrierOption.NPV() - npvExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected npv"
                    << "\n    calculated: " << barrierOption.NPV()
                    << "\n    expected:   " << npvExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(barrierOption.delta() - deltaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected delta"
                    << "\n    calculated: " << barrierOption.delta()
                    << "\n    expected:   " << deltaExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(barrierOption.gamma() - gammaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected gamma"
                    << "\n    calculated: " << barrierOption.gamma()
                    << "\n    expected:   " << gammaExpected
                    << "\n    tolerance:  " << tol); 
    }
}

BOOST_AUTO_TEST_CASE(testFdmHestonAmerican) {

    BOOST_TEST_MESSAGE("Testing FDM with American option in Heston model...");

    Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    Handle<YieldTermStructure> rTS(flatRate(0.05, Actual365Fixed()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual365Fixed()));

    ext::shared_ptr<HestonProcess> hestonProcess(
        new HestonProcess(rTS, qTS, s0, 0.04, 2.5, 0.04, 0.66, -0.8));

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(28, March, 2005);

    ext::shared_ptr<Exercise> exercise(new AmericanExercise(exerciseDate));

    ext::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Put, 100));

    VanillaOption option(payoff, exercise);
    ext::shared_ptr<PricingEngine> engine(
         new FdHestonVanillaEngine(ext::make_shared<HestonModel>(
                             hestonProcess), 200, 100, 50));
    option.setPricingEngine(engine);
    
    const Real tol = 0.01;
    const Real npvExpected   =  5.66032;
    const Real deltaExpected = -0.30065;
    const Real gammaExpected =  0.02202;
    
    if (std::fabs(option.NPV() - npvExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected npv"
                    << "\n    calculated: " << option.NPV()
                    << "\n    expected:   " << npvExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(option.delta() - deltaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected delta"
                    << "\n    calculated: " << option.delta()
                    << "\n    expected:   " << deltaExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(option.gamma() - gammaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected gamma"
                    << "\n    calculated: " << option.gamma()
                    << "\n    expected:   " << gammaExpected
                    << "\n    tolerance:  " << tol); 
    }
}

BOOST_AUTO_TEST_CASE(testFdmHestonIkonenToivanen) {

    BOOST_TEST_MESSAGE("Testing FDM Heston for Ikonen and Toivanen tests...");

    /* check prices of american puts as given in:
       From Efficient numerical methods for pricing American options under 
       stochastic volatility, Samuli Ikonen, Jari Toivanen, 
       http://users.jyu.fi/~tene/papers/reportB12-05.pdf
    */
    Handle<YieldTermStructure> rTS(flatRate(0.10, Actual360()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual360()));

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(26, June, 2004);

    ext::shared_ptr<Exercise> exercise(new AmericanExercise(exerciseDate));

    ext::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Put, 10));

    VanillaOption option(payoff, exercise);

    Real strikes[]  = { 8, 9, 10, 11, 12 };
    Real expected[] = { 2.00000, 1.10763, 0.520038, 0.213681, 0.082046 };
    const Real tol = 0.001;
    
    for (Size i=0; i < std::size(strikes); ++i) {
        Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(strikes[i])));
        ext::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(rTS, qTS, s0, 0.0625, 5, 0.16, 0.9, 0.1));
    
        ext::shared_ptr<PricingEngine> engine(
             new FdHestonVanillaEngine(ext::make_shared<HestonModel>(
                                 hestonProcess), 100, 400));
        option.setPricingEngine(engine);
        
        Real calculated = option.NPV();
        if (std::fabs(calculated - expected[i]) > tol) {
            BOOST_ERROR("Failed to reproduce expected npv"
                        << "\n    strike:     " << strikes[i]
                        << "\n    calculated: " << calculated
                        << "\n    expected:   " << expected[i]
                        << "\n    tolerance:  " << tol); 
        }
    }
}

BOOST_AUTO_TEST_CASE(testFdmHestonBlackScholes) {

    BOOST_TEST_MESSAGE("Testing FDM Heston with Black Scholes model...");

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(26, June, 2004);

    Handle<YieldTermStructure> rTS(flatRate(0.10, Actual360()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual360()));
    Handle<BlackVolTermStructure> volTS(
                    flatVol(rTS->referenceDate(), 0.25, rTS->dayCounter()));
    
    ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exerciseDate));

    ext::shared_ptr<StrikedTypePayoff> payoff(new
                                      PlainVanillaPayoff(Option::Put, 10));

    VanillaOption option(payoff, exercise);

    Real strikes[]  = { 8, 9, 10, 11, 12 };
    const Real tol = 0.0001;

    for (Real& strike : strikes) {
        Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(strike)));

        ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
                       new GeneralizedBlackScholesProcess(s0, qTS, rTS, volTS));

        option.setPricingEngine(ext::shared_ptr<PricingEngine>(
                                        new AnalyticEuropeanEngine(bsProcess)));
        
        const Real expected = option.NPV();
        
        ext::shared_ptr<HestonProcess> hestonProcess(
            new HestonProcess(rTS, qTS, s0, 0.0625, 1, 0.0625, 0.0001, 0.0));

        // Hundsdorfer scheme
        option.setPricingEngine(ext::shared_ptr<PricingEngine>(
             new FdHestonVanillaEngine(ext::make_shared<HestonModel>(
                                           hestonProcess), 
                                       100, 400, 3)));
        
        Real calculated = option.NPV();
        if (std::fabs(calculated - expected) > tol) {
            BOOST_ERROR("Failed to reproduce expected npv"
                        << "\n    strike:     " << strike << "\n    calculated: " << calculated
                        << "\n    expected:   " << expected << "\n    tolerance:  " << tol);
        }
        
        // Explicit scheme
        option.setPricingEngine(ext::shared_ptr<PricingEngine>(
             new FdHestonVanillaEngine(ext::make_shared<HestonModel>(
                                           hestonProcess),
                                       4000, 400, 3, 0,
                                       FdmSchemeDesc::ExplicitEuler())));

        calculated = option.NPV();
        if (std::fabs(calculated - expected) > tol) {
            BOOST_ERROR("Failed to reproduce expected npv"
                        << "\n    strike:     " << strike << "\n    calculated: " << calculated
                        << "\n    expected:   " << expected << "\n    tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testFdmHestonEuropeanWithDividends) {

    BOOST_TEST_MESSAGE("Testing FDM with European option with dividends in Heston model...");

    Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(100.0)));

    Handle<YieldTermStructure> rTS(flatRate(0.05, Actual365Fixed()));
    Handle<YieldTermStructure> qTS(flatRate(0.0 , Actual365Fixed()));

    auto hestonProcess = ext::make_shared<HestonProcess>(rTS, qTS, s0, 0.04, 2.5, 0.04, 0.66, -0.8);

    Settings::instance().evaluationDate() = Date(28, March, 2004);
    Date exerciseDate(28, March, 2005);

    auto exercise = ext::make_shared<AmericanExercise>(exerciseDate);
    auto payoff = ext::make_shared<PlainVanillaPayoff>(Option::Put, 100);

    const std::vector<Real> dividends(1, 5);
    const std::vector<Date> dividendDates(1, Date(28, September, 2004));
    
    const Real tol = 0.01;
    const Real gammaTol = 0.001;
    const Real npvExpected   =  7.38216;
    const Real deltaExpected = -0.397902;
    const Real gammaExpected =  0.027747;

    VanillaOption option(payoff, exercise);
    auto engine = ext::make_shared<FdHestonVanillaEngine>(
        ext::make_shared<HestonModel>(hestonProcess),
        DividendVector(dividendDates, dividends),
        50, 100, 50);
    option.setPricingEngine(engine);
        
    if (std::fabs(option.NPV() - npvExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected npv"
                    << "\n    calculated: " << option.NPV()
                    << "\n    expected:   " << npvExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(option.delta() - deltaExpected) > tol) {
        BOOST_ERROR("Failed to reproduce expected delta"
                    << "\n    calculated: " << option.delta()
                    << "\n    expected:   " << deltaExpected
                    << "\n    tolerance:  " << tol); 
    }
    if (std::fabs(option.gamma() - gammaExpected) > gammaTol) {
        BOOST_ERROR("Failed to reproduce expected gamma"
                    << "\n    calculated: " << option.gamma()
                    << "\n    expected:   " << gammaExpected
                    << "\n    tolerance:  " << tol); 
    }
}

BOOST_AUTO_TEST_CASE(testFdmHestonConvergence, *precondition(if_speed(Fast))) {

    /* convergence tests based on 
       ADI finite difference schemes for option pricing in the
       Heston model with correlation, K.J. in t'Hout and S. Foulon
    */
    
    BOOST_TEST_MESSAGE("Testing FDM Heston convergence...");

    HestonTestData values[] = {
        { 1.5   , 0.04  , 0.3   , -0.9   , 0.025 , 0.0   , 1.0 , 100 },
        { 3.0   , 0.12  , 0.04  , 0.6    , 0.01  , 0.04  , 1.0 , 100 },
        { 0.6067, 0.0707, 0.2928, -0.7571, 0.03  , 0.0   , 3.0 , 100 },
        { 2.5   , 0.06  , 0.5   , -0.1   , 0.0507, 0.0469, 0.25, 100 }
    };

    FdmSchemeDesc schemes[] = {
        FdmSchemeDesc::Hundsdorfer(),
        FdmSchemeDesc::ModifiedCraigSneyd(),
        FdmSchemeDesc::ModifiedHundsdorfer(),
        FdmSchemeDesc::CraigSneyd(),
        FdmSchemeDesc::TrBDF2(),
        FdmSchemeDesc::CrankNicolson(),
    };
    
    Size tn[] = { 60 };
    Real v0[] = { 0.04 };
    
    const Date todaysDate(28, March, 2004); 
    Settings::instance().evaluationDate() = todaysDate;
    
    Handle<Quote> s0(ext::shared_ptr<Quote>(new SimpleQuote(75.0)));

    for (const auto& scheme : schemes) {
        for (auto& value : values) {
            for (unsigned long j : tn) {
                for (Real k : v0) {
                    Handle<YieldTermStructure> rTS(flatRate(value.r, Actual365Fixed()));
                    Handle<YieldTermStructure> qTS(flatRate(value.q, Actual365Fixed()));

                    ext::shared_ptr<HestonProcess> hestonProcess(new HestonProcess(
                        rTS, qTS, s0, k, value.kappa, value.theta, value.sigma, value.rho));

                    Date exerciseDate =
                        todaysDate + Period(static_cast<Integer>(value.T * 365), Days);
                    ext::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));

                    ext::shared_ptr<StrikedTypePayoff> payoff(
                        new PlainVanillaPayoff(Option::Call, value.K));

                    VanillaOption option(payoff, exercise);
                    ext::shared_ptr<PricingEngine> engine(new FdHestonVanillaEngine(
                        ext::make_shared<HestonModel>(hestonProcess), j, 101, 51, 0, scheme));
                    option.setPricingEngine(engine);
                    
                    const Real calculated = option.NPV();
                    
                    ext::shared_ptr<PricingEngine> analyticEngine(
                        new AnalyticHestonEngine(
                            ext::make_shared<HestonModel>(
                                hestonProcess), 144));
                    
                    option.setPricingEngine(analyticEngine);
                    const Real expected = option.NPV();
                    if (   std::fabs(expected - calculated)/expected > 0.02
                        && std::fabs(expected - calculated) > 0.002) {
                        BOOST_ERROR("Failed to reproduce expected npv"
                                    << "\n    calculated: " << calculated
                                    << "\n    expected:   " << expected
                                    << "\n    tolerance:  " << 0.01); 
                    }
                }
            }
        }
    }
}

#ifdef QL_HIGH_RESOLUTION_DATE
BOOST_AUTO_TEST_CASE(testFdmHestonIntradayPricing) {

    BOOST_TEST_MESSAGE("Testing FDM Heston intraday pricing...");

    const Option::Type type(Option::Put);
    const Real underlying = 36;
    const Real strike = underlying;
    const Spread dividendYield = 0.00;
    const Rate riskFreeRate = 0.06;
    const Real v0    = 0.2;
    const Real kappa = 1.0;
    const Real theta = v0;
    const Real sigma = 0.0065;
    const Real rho   = -0.75;
    const DayCounter dayCounter = Actual365Fixed();

    const Date maturity(17, May, 2014, 17, 30, 0);

    const ext::shared_ptr<Exercise> europeanExercise(
        new EuropeanExercise(maturity));
    const ext::shared_ptr<StrikedTypePayoff> payoff(
        new PlainVanillaPayoff(type, strike));
    VanillaOption option(payoff, europeanExercise);

    const Handle<Quote> s0(
         ext::shared_ptr<Quote>(new SimpleQuote(underlying)));
    RelinkableHandle<BlackVolTermStructure> flatVolTS;
    RelinkableHandle<YieldTermStructure> flatTermStructure, flatDividendTS;
    const ext::shared_ptr<HestonProcess> process(
        new HestonProcess(flatTermStructure, flatDividendTS, s0,
              v0, kappa, theta, sigma, rho));
    const ext::shared_ptr<HestonModel> model(new HestonModel(process));
    const ext::shared_ptr<PricingEngine> fdm(
        new FdHestonVanillaEngine(model, 20, 100, 26, 0));
    option.setPricingEngine(fdm);

    const Real gammaExpected[] = {
        1.46757, 1.54696, 1.6408, 1.75409, 1.89464,
        2.07548, 2.32046, 2.67944, 3.28164, 4.64096  };

    for (Size i = 0; i < 10; ++i) {
        const Date now(17, May, 2014, 15, i*15, 0);
        Settings::instance().evaluationDate() = now;

        flatTermStructure.linkTo(ext::shared_ptr<YieldTermStructure>(
            new FlatForward(now, riskFreeRate, dayCounter)));
        flatDividendTS.linkTo(ext::shared_ptr<YieldTermStructure>(
            new FlatForward(now, dividendYield, dayCounter)));

        const Real gammaCalculated = option.gamma();
        if (std::fabs(gammaCalculated - gammaExpected[i]) > 1e-4) {
            BOOST_ERROR("unable to reproduce intraday gamma values at time "
                        << "\n   timestamp : " << io::iso_datetime(now)
                        << "\n   expiry    : " << io::iso_datetime(maturity)
                        << "\n   expected  : " << gammaExpected[i]
                        << "\n   calculated: "<<  gammaCalculated);
        }
    }
}
#endif

BOOST_AUTO_TEST_CASE(testMethodOfLinesAndCN) {
    BOOST_TEST_MESSAGE("Testing method of lines to solve Heston PDEs...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(21, February, 2018);

    Settings::instance().evaluationDate() = today;

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100.0));
    const Handle<YieldTermStructure> qTS(flatRate(today, 0.0, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, 0.0, dc));

    const Real v0    = 0.09;
    const Real kappa = 1.0;
    const Real theta = v0;
    const Real sigma = 0.4;
    const Real rho   = -0.75;

    const Date maturity = today + Period(3, Months);

    const ext::shared_ptr<HestonModel> model(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, spot, v0, kappa, theta, sigma, rho)));

    const Size xGrid = 21;
    const Size vGrid = 7;

    const ext::shared_ptr<PricingEngine> fdmDefault(
        ext::make_shared<FdHestonVanillaEngine>(model, 10, xGrid, vGrid, 0));

    const ext::shared_ptr<PricingEngine> fdmMol(
        ext::make_shared<FdHestonVanillaEngine>(
            model, 10, xGrid, vGrid, 0, FdmSchemeDesc::MethodOfLines()));

    const ext::shared_ptr<PlainVanillaPayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Put, spot->value());

    VanillaOption option(
        payoff, ext::make_shared<AmericanExercise>(maturity));

    option.setPricingEngine(fdmMol);
    const Real calculatedMoL = option.NPV();

    option.setPricingEngine(fdmDefault);
    const Real expected = option.NPV();

    const Real tol = 0.005;
    const Real diffMoL = std::fabs(expected - calculatedMoL);

    if (diffMoL > tol) {
        BOOST_FAIL("Failed to reproduce european option values with MOL"
                   << "\n    calculated: " << calculatedMoL
                   << "\n    expected:   " << expected
                   << "\n    difference: " << diffMoL
                   << "\n    tolerance:  " << tol);
    }

    const ext::shared_ptr<PricingEngine> fdmCN(
        ext::make_shared<FdHestonVanillaEngine>(
            model, 10, xGrid, vGrid, 0, FdmSchemeDesc::CrankNicolson()));
    option.setPricingEngine(fdmCN);

    const Real calculatedCN = option.NPV();
    const Real diffCN = std::fabs(expected - calculatedCN);

    if (diffCN > tol) {
        BOOST_FAIL("Failed to reproduce european option values with Crank-Nicolson"
                   << "\n    calculated: " << calculatedCN
                   << "\n    expected:   " << expected
                   << "\n    difference: " << diffCN
                   << "\n    tolerance:  " << tol);
    }

    BarrierOption barrierOption(
        Barrier::DownOut, 85.0, 10.0,
        payoff, ext::make_shared<EuropeanExercise>(maturity));

    barrierOption.setPricingEngine(
        ext::make_shared<FdHestonBarrierEngine>(model, 100, 31, 11));

    const Real expectedBarrier = barrierOption.NPV();

    barrierOption.setPricingEngine(
        ext::make_shared<FdHestonBarrierEngine>(model, 100, 31, 11, 0,
            FdmSchemeDesc::MethodOfLines()));

    const Real calculatedBarrierMoL = barrierOption.NPV();

    const Real barrierTol = 0.01;
    const Real barrierDiffMoL = std::fabs(expectedBarrier - calculatedBarrierMoL);

    if (barrierDiffMoL > barrierTol) {
        BOOST_FAIL("Failed to reproduce barrier option values with MOL"
                   << "\n    calculated: " << calculatedBarrierMoL
                   << "\n    expected:   " << expectedBarrier
                   << "\n    difference: " << barrierDiffMoL
                   << "\n    tolerance:  " << barrierTol);
    }

    barrierOption.setPricingEngine(
        ext::make_shared<FdHestonBarrierEngine>(model, 100, 31, 11, 0,
            FdmSchemeDesc::CrankNicolson()));

    const Real calculatedBarrierCN = barrierOption.NPV();
    const Real barrierDiffCN = std::fabs(expectedBarrier - calculatedBarrierCN);

    if (barrierDiffCN > barrierTol) {
        BOOST_FAIL("Failed to reproduce barrier option values with Crank-Nicolson"
                   << "\n    calculated: " << calculatedBarrierCN
                   << "\n    expected:   " << expectedBarrier
                   << "\n    difference: " << barrierDiffCN
                   << "\n    tolerance:  " << barrierTol);
    }
}

BOOST_AUTO_TEST_CASE(testSpuriousOscillations) {
    BOOST_TEST_MESSAGE("Testing for spurious oscillations when "
            "solving the Heston PDEs...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(7, June, 2018);

    Settings::instance().evaluationDate() = today;

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(100.0));
    const Handle<YieldTermStructure> qTS(flatRate(today, 0.1, dc));
    const Handle<YieldTermStructure> rTS(flatRate(today, 0.0, dc));

    const Real v0    = 0.005;
    const Real kappa = 1.0;
    const Real theta = 0.005;
    const Real sigma = 0.4;
    const Real rho   = -0.75;

    const Date maturity = today + Period(1, Years);

    const ext::shared_ptr<HestonProcess> process =
        ext::make_shared<HestonProcess>(
            rTS, qTS, spot, v0, kappa, theta, sigma, rho);

    const ext::shared_ptr<HestonModel> model =
        ext::make_shared<HestonModel>(process);

    const ext::shared_ptr<FdHestonVanillaEngine> hestonEngine(
        ext::make_shared<FdHestonVanillaEngine>(
            model, 6, 200, 13, 0, FdmSchemeDesc::TrBDF2()));

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, spot->value()),
        ext::make_shared<EuropeanExercise>(maturity));

    option.setupArguments(hestonEngine->getArguments());

    const std::tuple<FdmSchemeDesc, std::string, bool> descs[] = {
        {FdmSchemeDesc::CraigSneyd(), "Craig-Sneyd", true},
        {FdmSchemeDesc::Hundsdorfer(), "Hundsdorfer", true},
        {FdmSchemeDesc::ModifiedHundsdorfer(), "Mod. Hundsdorfer", true},
        {FdmSchemeDesc::Douglas(), "Douglas", true},
        {FdmSchemeDesc::CrankNicolson(), "Crank-Nicolson", true},
        {FdmSchemeDesc::ImplicitEuler(), "Implicit", false},
        {FdmSchemeDesc::TrBDF2(), "TR-BDF2", false}
    };

    for (const auto & [desc, name, spurious] : descs) {
        const ext::shared_ptr<FdmHestonSolver> solver = ext::make_shared<FdmHestonSolver>(
            Handle<HestonProcess>(process), hestonEngine->getSolverDesc(1.0), desc);

        std::vector<Real> gammas;
        for (Real x=99; x < 101.001; x+=0.1) {
            gammas.push_back(solver->gammaAt(x, v0));
        }

        Real maximum = QL_MIN_REAL;
        for (Size i=1; i < gammas.size(); ++i) {
            const Real diff = std::fabs(gammas[i] - gammas[i-1]);
            if (diff > maximum)
                maximum = diff;
        }

        const Real tol = 0.01;
        const bool hasSpuriousOscillations = maximum > tol;

        if (hasSpuriousOscillations != spurious) {
            BOOST_ERROR("unable to reproduce spurious oscillation behaviour "
                        << "\n   scheme name          : " << name
                        << "\n   oscillations observed: " << hasSpuriousOscillations
                        << "\n   oscillations expected: " << spurious);
        }
    }
}

BOOST_AUTO_TEST_CASE(testAmericanCallPutParity) {
    BOOST_TEST_MESSAGE("Testing call/put parity for American options "
                       "under the Heston model...");

    // A. Battauz, M. De Donno,m A. Sbuelz:
    // The put-call symmetry for American options in
    // the Heston stochastic volatility model

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(15, April, 2022);

    Settings::instance().evaluationDate() = today;

    struct OptionSpec {
        Real spot;
        Real strike;
        Size maturityInDays;
        Real r, q;
        Real v0, kappa, theta, sig, rho;
    };

    auto buildStochProcess = [&dc](const OptionSpec& testCase) {
        return ext::make_shared<HestonProcess>(
            Handle<YieldTermStructure>(flatRate(testCase.r, dc)),
            Handle<YieldTermStructure>(flatRate(testCase.q, dc)),
            Handle<Quote>(ext::make_shared<SimpleQuote>(testCase.spot)),
            testCase.v0, testCase.kappa,
            testCase.theta, testCase.sig, testCase.rho
        );
    };

    const OptionSpec testCaseSpecs[] = {
        {100.0, 90.0, 365, 0.02, 0.15, 0.25, 1.0, 0.09, 0.5, -0.75},
        {100.0, 90.0, 365, 0.05, 0.20, 0.5, 1.0, 0.05, 0.75, -0.9}
    };

    const Size xGrid = 200;
    const Size vGrid = 25;
    const Size timeStepsPerYear = 50;

    for (const auto& testCaseSpec: testCaseSpecs) {
        const auto maturityDate =
            today + Period(testCaseSpec.maturityInDays, Days);
        const Time maturityTime = dc.yearFraction(today,  maturityDate);
        const Size tGrid = Size(maturityTime * timeStepsPerYear);

        const auto exercise =
            ext::make_shared<AmericanExercise>(today, maturityDate);

        VanillaOption callOption(
            ext::make_shared<PlainVanillaPayoff>(
                Option::Call, testCaseSpec.strike),
            exercise
        );

        callOption.setPricingEngine(
            ext::make_shared<FdHestonVanillaEngine>(
                ext::make_shared<HestonModel>(
                    buildStochProcess(testCaseSpec)),
                tGrid, xGrid, vGrid
            )
        );

        const Real callNpv = callOption.NPV();

        OptionSpec putOptionSpec = {
            testCaseSpec.strike,
            testCaseSpec.spot,
            testCaseSpec.maturityInDays,
            testCaseSpec.q,
            testCaseSpec.r,
            testCaseSpec.v0,
            testCaseSpec.kappa - testCaseSpec.sig*testCaseSpec.rho,
            testCaseSpec.kappa*testCaseSpec.theta/
                (testCaseSpec.kappa - testCaseSpec.sig*testCaseSpec.rho),
            testCaseSpec.sig,
            -testCaseSpec.rho
        };

        VanillaOption putOption(
            ext::make_shared<PlainVanillaPayoff>(
                Option::Put, putOptionSpec.strike),
            exercise
        );

        putOption.setPricingEngine(
            ext::make_shared<FdHestonVanillaEngine>(
                ext::make_shared<HestonModel>(
                    buildStochProcess(putOptionSpec)),
                tGrid, xGrid, vGrid
            )
        );

        const Real putNpv = putOption.NPV();

        const Real diff = std::fabs(putNpv -callNpv);
        const Real tol = 0.025;

        if (diff > tol) {
            BOOST_FAIL("failed to reproduce American call/put parity"
                    << "\n    Put NPV   : " << putNpv
                    << "\n    Call NPV  : " << callNpv
                    << "\n    difference: " << diff
                    << "\n    tolerance : " << tol);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
