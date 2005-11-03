/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000-2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/quantlib.hpp>
#include <boost/timer.hpp>
#include <iostream>
#include <iomanip>

using namespace QuantLib;

#if defined(QL_ENABLE_SESSIONS)
namespace QuantLib {

    Integer sessionId() { return 0; }

}
#endif


int main(int, char* [])
{
    try {
        QL_IO_INIT

        boost::timer timer;
        std::cout << std::endl;

        // our option
        Option::Type type(Option::Call);
        Real underlying = 7;
        Real strike = 8;
        Spread dividendYield = 0.05;
        Rate riskFreeRate = 0.05;

        Date todaysDate(15, May, 1998);
        Date settlementDate(17, May, 1998);
        Settings::instance().evaluationDate() = todaysDate;

        Date exerciseDate(17, May, 1999);
        DayCounter dayCounter = Actual365Fixed();

        Volatility volatility = 0.10;
        std::cout << "Option type = "  << type << std::endl;
        std::cout << "Exercise date = "        << exerciseDate
                  << std::endl;
        std::cout << "Underlying price = "        << underlying
                  << std::endl;
        std::cout << "Strike = "                  << strike
                  << std::endl;
        std::cout << "Risk-free interest rate = " << io::rate(riskFreeRate)
                  << std::endl;
        std::cout << "Dividend yield = " << io::rate(dividendYield)
                  << std::endl;
        std::cout << "Volatility = " << io::volatility(volatility)
                  << std::endl;
        std::cout << std::endl;

        boost::shared_ptr<Exercise> exercise(
                                          new EuropeanExercise(exerciseDate));

        Handle<Quote> underlyingH(
            boost::shared_ptr<Quote>(new SimpleQuote(underlying)));

        // bootstrap the yield/dividend/vol curves
        Handle<YieldTermStructure> flatTermStructure(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, riskFreeRate, dayCounter)));
        Handle<YieldTermStructure> flatDividendTS(
            boost::shared_ptr<YieldTermStructure>(
                new FlatForward(settlementDate, dividendYield, dayCounter)));
        Handle<BlackVolTermStructure> flatVolTS(
            boost::shared_ptr<BlackVolTermStructure>(
                new BlackConstantVol(settlementDate, volatility, dayCounter)));

        boost::shared_ptr<StrikedTypePayoff> payoff(new
            PlainVanillaPayoff(type, strike));

        boost::shared_ptr<BlackScholesProcess> stochasticProcess(new
            BlackScholesProcess(underlyingH,
                                flatDividendTS,
                                flatTermStructure,
                                flatVolTS));

        EuropeanOption option(stochasticProcess, payoff, exercise);


        std::string method;
        Real value, discrepancy, rightValue, relativeDiscrepancy;

        std::cout << std::endl;

        // write column headings
        Size widths[] = { 19, 13, 16, 14, 14 };
        std::cout << std::setw(widths[0]) << std::left << "Method"
                  << std::setw(widths[1]) << std::left << "Value"
                  << std::setw(widths[2]) << std::left << "Error estimate"
                  << std::setw(widths[3]) << std::left << "Discrepancy"
                  << std::setw(widths[4]) << std::left << "Rel. Discr."
                  << std::endl;

        // method: Black-Scholes Engine
        method = "Black-Scholes";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new AnalyticEuropeanEngine()));
        rightValue = value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << "N/A"
                  << std::scientific
                  << std::setw(widths[4]) << std::left << "N/A"
                  << std::endl;


        // method: Integral
        method = "Integral";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new IntegralEngine()));
        value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;

        Size timeSteps = 801;

        // Binomial Method (JR)
        method = "Binomial (JR)";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<JarrowRudd>(timeSteps)));
        value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;


        // Binomial Method (CRR)
        method = "Binomial (CRR)";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<CoxRossRubinstein>(timeSteps)));
        value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;

        // Equal Probability Additive Binomial Tree (EQP)
        method = "Additive (EQP)";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<AdditiveEQPBinomialTree>(timeSteps)));
        value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;

        // Equal Jumps Additive Binomial Tree (Trigeorgis)
        method = "Bin. Trigeorgis";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Trigeorgis>(timeSteps)));
        value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;

        // Tian Binomial Tree (third moment matching)
        method = "Binomial Tian";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Tian>(timeSteps)));
        value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;

        // Leisen-Reimer Binomial Tree
        method = "Binomial LR";
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<LeisenReimer>(timeSteps)));
        value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;

        // Finite Differences

        method = "Finite Diff.";
        timeSteps = 100;
        Size gridPoints = 100;
        option.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new FDEuropeanEngine(timeSteps, gridPoints)));
        value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;

        // Monte Carlo Method
        timeSteps = 1;

        method = "MC (crude)";
        Size mcSeed = 42;

        boost::shared_ptr<PricingEngine> mcengine1;
        mcengine1 =
            MakeMCEuropeanEngine<PseudoRandom>().withSteps(timeSteps)
                                                .withTolerance(0.02)
                                                .withSeed(mcSeed);
        option.setPricingEngine(mcengine1);

        value = option.NPV();
        Real errorEstimate = option.errorEstimate();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << errorEstimate
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;

        method = "MC (Sobol)";
        timeSteps = 1;
        Size nSamples = 32768;  // 2^15

        boost::shared_ptr<PricingEngine> mcengine2;
        mcengine2 =
            MakeMCEuropeanEngine<LowDiscrepancy>().withSteps(timeSteps)
                                                  .withSamples(nSamples);
        option.setPricingEngine(mcengine2);

        value = option.NPV();
        discrepancy = std::fabs(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << value
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << discrepancy
                  << std::scientific
                  << std::setw(widths[4]) << std::left << relativeDiscrepancy
                  << std::endl;

        Real seconds = timer.elapsed();
        Integer hours = int(seconds/3600);
        seconds -= hours * 3600;
        Integer minutes = int(seconds/60);
        seconds -= minutes * 60;
        std::cout << " \nRun completed in ";
        if (hours > 0)
            std::cout << hours << " h ";
        if (hours > 0 || minutes > 0)
            std::cout << minutes << " m ";
        std::cout << std::fixed << std::setprecision(0)
                  << seconds << " s\n" << std::endl;

        return 0;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}
