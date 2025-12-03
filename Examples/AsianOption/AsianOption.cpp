/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2025 Kareem Fareed

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

/*  This example demonstrates Asian option pricing using various methods
    available in QuantLib, including:
    - Discrete vs. Continuous averaging
    - Arithmetic vs. Geometric averaging
    - Multiple pricing engines (Analytic, Monte Carlo, PDE, Approximations)
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif
#include <ql/instruments/asianoption.hpp>
#include <ql/pricingengines/asian/analytic_cont_geom_av_price.hpp>
#include <ql/pricingengines/asian/analytic_discr_geom_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_arith_av_price.hpp>
#include <ql/pricingengines/asian/mc_discr_geom_av_price.hpp>
#include <ql/pricingengines/asian/fdblackscholesasianengine.hpp>
#include <ql/pricingengines/asian/turnbullwakemanasianengine.hpp>
#include <ql/pricingengines/asian/continuousarithmeticasianlevyengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <iostream>
#include <iomanip>

using namespace QuantLib;

int main(int, char* []) {

    try {

        std::cout << std::endl;

        // set up dates
        Calendar calendar = TARGET();
        Date todaysDate(15, November, 2025);
        Date settlementDate(17, November, 2025);
        Settings::instance().evaluationDate() = todaysDate;

        // our options
        Option::Type type(Option::Put);
        Real underlying = 100;
        Real strike = 100;
        Spread dividendYield = 0.03;
        Rate riskFreeRate = 0.06;
        Volatility volatility = 0.20;
        Date maturity(17, November, 2026);
        DayCounter dayCounter = Actual365Fixed();

        std::cout << "Asian Option Pricing Example" << std::endl;
        std::cout << "=============================" << std::endl << std::endl;
        std::cout << "Option type = "  << type << std::endl;
        std::cout << "Maturity = "<< maturity << std::endl;
        std::cout << "Underlying price = "  << underlying << std::endl;
        std::cout << "Strike = " << strike << std::endl;
        std::cout << "Risk-free interest rate = " << io::rate(riskFreeRate)
                  << std::endl;
        std::cout << "Dividend yield = " << io::rate(dividendYield)
                  << std::endl;
        std::cout << "Volatility = " << io::volatility(volatility)
                  << std::endl;
        std::cout << std::endl;

        // Set up fixing dates for discrete averaging (monthly fixings)
        std::vector<Date> fixingDates;
        Date d = settlementDate;
        while (d <= maturity) {
            fixingDates.push_back(d);
            d = calendar.advance(d, 1, Months);
        }

        std::cout << "Number of fixings: " << fixingDates.size() << std::endl;
        std::cout << std::endl;

        // Set up flat yield/dividend/vol curves
        auto underlyingH = makeQuoteHandle(underlying);
        Handle<YieldTermStructure> flatTermStructure(
            ext::make_shared<FlatForward>(settlementDate, riskFreeRate, dayCounter));
        Handle<YieldTermStructure> flatDividendTS(
            ext::make_shared<FlatForward>(settlementDate, dividendYield, dayCounter));
        Handle<BlackVolTermStructure> flatVolTS(
            ext::make_shared<BlackConstantVol>(settlementDate, calendar, volatility,
                                     dayCounter));
        auto payoff = ext::make_shared<PlainVanillaPayoff>(type, strike);
        auto bsmProcess = ext::make_shared<BlackScholesMertonProcess>(
                underlyingH, flatDividendTS, flatTermStructure, flatVolTS);

        auto europeanExercise = ext::make_shared<EuropeanExercise>(maturity);

        // Create Asian options
        DiscreteAveragingAsianOption discreteArithmeticOption(
            Average::Arithmetic,
            0.0,  // running sum
            0,    // past fixings
            fixingDates,
            payoff,
            europeanExercise);

        DiscreteAveragingAsianOption discreteGeometricOption(
            Average::Geometric,
            0.0,  // running product (should be 1.0 for geometric, but 0 means no past fixings)
            0,    // past fixings
            fixingDates,
            payoff,
            europeanExercise);

        ContinuousAveragingAsianOption continuousArithmeticOption(
            Average::Arithmetic,
            payoff,
            europeanExercise);

        ContinuousAveragingAsianOption continuousGeometricOption(
            Average::Geometric,
            payoff,
            europeanExercise);

        // **********************************************
        // DISCRETE AVERAGING - GEOMETRIC AVERAGE
        // **********************************************
        std::cout << std::endl;
        std::cout << "DISCRETE AVERAGING - GEOMETRIC AVERAGE" << std::endl;
        std::cout << "======================================" << std::endl;
        Size widths[] = { 40, 16 };

        // Analytic geometric discrete
        std::cout << std::setw(widths[0]) << std::left << "Analytic (Discrete Geometric)"
                  << std::fixed << std::setprecision(6)
                  << std::setw(widths[1]) << std::left;
        discreteGeometricOption.setPricingEngine(
            ext::make_shared<AnalyticDiscreteGeometricAveragePriceAsianEngine>(bsmProcess));
        std::cout << discreteGeometricOption.NPV() << std::endl;

        // Monte Carlo geometric discrete
        Size mcSeed = 42;
        std::cout << std::setw(widths[0]) << std::left << "Monte Carlo (Discrete Geometric)"
                  << std::fixed << std::setprecision(6)
                  << std::setw(widths[1]) << std::left;
        discreteGeometricOption.setPricingEngine(
            MakeMCDiscreteGeometricAPEngine<PseudoRandom>(bsmProcess)
                .withSamples(10000)
                .withSeed(mcSeed));
        std::cout << discreteGeometricOption.NPV() << std::endl;

        // **********************************************
        // DISCRETE AVERAGING - ARITHMETIC AVERAGE
        // **********************************************
        std::cout << std::endl;
        std::cout << "DISCRETE AVERAGING - ARITHMETIC AVERAGE" << std::endl;
        std::cout << "=======================================" << std::endl;

        // Monte Carlo arithmetic discrete
        std::cout << std::setw(widths[0]) << std::left << "Monte Carlo (Discrete Arithmetic)"
                  << std::fixed << std::setprecision(6)
                  << std::setw(widths[1]) << std::left;
        discreteArithmeticOption.setPricingEngine(
            MakeMCDiscreteArithmeticAPEngine<PseudoRandom>(bsmProcess)
                .withSamples(10000)
                .withSeed(mcSeed));
        std::cout << discreteArithmeticOption.NPV() << std::endl;

        // Monte Carlo arithmetic discrete with control variate
        std::cout << std::setw(widths[0]) << std::left << "MC with Control Variate"
                  << std::fixed << std::setprecision(6)
                  << std::setw(widths[1]) << std::left;
        discreteArithmeticOption.setPricingEngine(
            MakeMCDiscreteArithmeticAPEngine<PseudoRandom>(bsmProcess)
                .withSamples(10000)
                .withControlVariate()
                .withSeed(mcSeed));
        std::cout << discreteArithmeticOption.NPV() << std::endl;

        // Turnbull-Wakeman approximation
        std::cout << std::setw(widths[0]) << std::left << "Turnbull-Wakeman Approximation"
                  << std::fixed << std::setprecision(6)
                  << std::setw(widths[1]) << std::left;
        discreteArithmeticOption.setPricingEngine(
            ext::make_shared<TurnbullWakemanAsianEngine>(bsmProcess));
        std::cout << discreteArithmeticOption.NPV() << std::endl;

        // Finite Differences (PDE method)
        std::cout << std::setw(widths[0]) << std::left << "Finite Differences (PDE)"
                  << std::fixed << std::setprecision(6)
                  << std::setw(widths[1]) << std::left;
        discreteArithmeticOption.setPricingEngine(
            ext::make_shared<FdBlackScholesAsianEngine>(bsmProcess, 100, 100, 50));
        std::cout << discreteArithmeticOption.NPV() << std::endl;

        // **********************************************
        // CONTINUOUS AVERAGING - GEOMETRIC AVERAGE
        // **********************************************
        std::cout << std::endl;
        std::cout << "CONTINUOUS AVERAGING - GEOMETRIC AVERAGE" << std::endl;
        std::cout << "========================================" << std::endl;

        // Analytic geometric continuous
        std::cout << std::setw(widths[0]) << std::left << "Analytic (Continuous Geometric)"
                  << std::fixed << std::setprecision(6)
                  << std::setw(widths[1]) << std::left;
        continuousGeometricOption.setPricingEngine(
            ext::make_shared<AnalyticContinuousGeometricAveragePriceAsianEngine>(bsmProcess));
        std::cout << continuousGeometricOption.NPV() << std::endl;

        // **********************************************
        // CONTINUOUS AVERAGING - ARITHMETIC AVERAGE
        // **********************************************
        std::cout << std::endl;
        std::cout << "CONTINUOUS AVERAGING - ARITHMETIC AVERAGE" << std::endl;
        std::cout << "=========================================" << std::endl;

        // Continuous Arithmetic Levy Engine
        std::cout << std::setw(widths[0]) << std::left << "Continuous Arithmetic Levy Engine"
                  << std::fixed << std::setprecision(6)
                  << std::setw(widths[1]) << std::left;
        auto currentAverage = makeQuoteHandle(0.0);  // No averaging yet for fresh option
        continuousArithmeticOption.setPricingEngine(
            ext::make_shared<ContinuousArithmeticAsianLevyEngine>(bsmProcess, currentAverage, settlementDate));
        std::cout << continuousArithmeticOption.NPV() << std::endl;

        // **********************************************
        // COMPARISON SUMMARY
        // **********************************************
        std::cout << std::endl;
        std::cout << "SUMMARY COMPARISON" << std::endl;
        std::cout << "==================" << std::endl;
        std::cout << std::endl;
        std::cout << "Averaging Type          Method                              NPV" << std::endl;
        std::cout << "--------------------------------------------------------------------------------" << std::endl;

        // Re-price all for summary
        discreteGeometricOption.setPricingEngine(
            ext::make_shared<AnalyticDiscreteGeometricAveragePriceAsianEngine>(bsmProcess));
        std::cout << std::setw(24) << std::left << "Discrete Geometric"
                  << std::setw(36) << std::left << "Analytic"
                  << std::fixed << std::setprecision(6)
                  << discreteGeometricOption.NPV() << std::endl;

        discreteArithmeticOption.setPricingEngine(
            MakeMCDiscreteArithmeticAPEngine<PseudoRandom>(bsmProcess)
                .withSamples(10000)
                .withControlVariate()
                .withSeed(mcSeed));
        std::cout << std::setw(24) << std::left << "Discrete Arithmetic"
                  << std::setw(36) << std::left << "Monte Carlo (Control Variate)"
                  << std::fixed << std::setprecision(6)
                  << discreteArithmeticOption.NPV() << std::endl;

        discreteArithmeticOption.setPricingEngine(
            ext::make_shared<TurnbullWakemanAsianEngine>(bsmProcess));
        std::cout << std::setw(24) << std::left << "Discrete Arithmetic"
                  << std::setw(36) << std::left << "Turnbull-Wakeman"
                  << std::fixed << std::setprecision(6)
                  << discreteArithmeticOption.NPV() << std::endl;

        continuousGeometricOption.setPricingEngine(
            ext::make_shared<AnalyticContinuousGeometricAveragePriceAsianEngine>(bsmProcess));
        std::cout << std::setw(24) << std::left << "Continuous Geometric"
                  << std::setw(36) << std::left << "Analytic"
                  << std::fixed << std::setprecision(6)
                  << continuousGeometricOption.NPV() << std::endl;

        continuousArithmeticOption.setPricingEngine(
            ext::make_shared<ContinuousArithmeticAsianLevyEngine>(bsmProcess, currentAverage, settlementDate));
        std::cout << std::setw(24) << std::left << "Continuous Arithmetic"
                  << std::setw(36) << std::left << "Levy Engine"
                  << std::fixed << std::setprecision(6)
                  << continuousArithmeticOption.NPV() << std::endl;

        std::cout << std::endl;

        return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}
