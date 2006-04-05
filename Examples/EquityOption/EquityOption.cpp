/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2005 StatPro Italia srl

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

        // our options
        Option::Type type(Option::Put);
        Real underlying = 36;
        Real strike = 40;
        Spread dividendYield = 0.00;
        Rate riskFreeRate = 0.06;
        Volatility volatility = 0.20;

        Date todaysDate(15, May, 1998);
        Date settlementDate(17, May, 1998);
        Settings::instance().evaluationDate() = todaysDate;

        Date maturity(17, May, 1999);
        DayCounter dayCounter = Actual365Fixed();

        std::cout << "Option type = "  << type << std::endl;
        std::cout << "Maturity = "        << maturity << std::endl;
        std::cout << "Underlying price = "        << underlying << std::endl;
        std::cout << "Strike = "                  << strike << std::endl;
        std::cout << "Risk-free interest rate = " << io::rate(riskFreeRate)
                  << std::endl;
        std::cout << "Dividend yield = " << io::rate(dividendYield)
                  << std::endl;
        std::cout << "Volatility = " << io::volatility(volatility)
                  << std::endl;
        std::cout << std::endl;

        std::string method;

        std::cout << std::endl ;

        // write column headings
        Size widths[] = { 35, 14, 14, 14 };
        std::cout << std::setw(widths[0]) << std::left << "Method"
                  << std::setw(widths[1]) << std::left << "European"
                  << std::setw(widths[2]) << std::left << "Bermudan"
                  << std::setw(widths[3]) << std::left << "American"
                  << std::endl;

        std::vector<Date> exerciseDates;
        for (Integer i=1; i<=4; i++)
            exerciseDates.push_back(settlementDate + 3*i*Months);

        boost::shared_ptr<Exercise> europeanExercise(
                                         new EuropeanExercise(maturity));

        boost::shared_ptr<Exercise> bermudanExercise(
                                         new BermudanExercise(exerciseDates));

        boost::shared_ptr<Exercise> americanExercise(
                                         new AmericanExercise(settlementDate,
                                                              maturity));

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

        boost::shared_ptr<StrikedTypePayoff> payoff(
                                        new PlainVanillaPayoff(type, strike));

        boost::shared_ptr<StochasticProcess> stochasticProcess(
                 new BlackScholesMertonProcess(underlyingH, flatDividendTS,
                                               flatTermStructure, flatVolTS));

        // options

        VanillaOption europeanOption(stochasticProcess, payoff,
                                     europeanExercise);

        VanillaOption bermudanOption(stochasticProcess, payoff,
                                     bermudanExercise);

        VanillaOption americanOption(stochasticProcess, payoff,
                                     americanExercise);

        // Analytic formulas:

        // Black-Scholes for European
        method = "Black-Scholes";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                                 new AnalyticEuropeanEngine));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << "N/A"
                  << std::endl;

        // Barone-Adesi and Whaley approximation for American
        method = "Barone-Adesi/Whaley";
        americanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                   new BaroneAdesiWhaleyApproximationEngine));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << "N/A"
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        // Bjerksund and Stensland approximation for American
        method = "Bjerksund/Stensland";
        americanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                  new BjerksundStenslandApproximationEngine));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << "N/A"
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        // Integral

        method = "Integral";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                                                         new IntegralEngine));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << "N/A"
                  << std::endl;

        // Finite differences

        Size timeSteps = 801;

        method = "Finite differences";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                              new FDEuropeanEngine(timeSteps,timeSteps-1)));
        bermudanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                              new FDBermudanEngine(timeSteps,timeSteps-1)));
        americanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
                              new FDAmericanEngine(timeSteps,timeSteps-1)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        // Binomial method

        method = "Binomial Jarrow-Rudd";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<JarrowRudd>(timeSteps)));
        bermudanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<JarrowRudd>(timeSteps)));
        americanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<JarrowRudd>(timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        method = "Binomial Cox-Ross-Rubinstein";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<CoxRossRubinstein>(timeSteps)));
        bermudanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<CoxRossRubinstein>(timeSteps)));
        americanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<CoxRossRubinstein>(timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        method = "Additive equiprobabilities";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<AdditiveEQPBinomialTree>(timeSteps)));
        bermudanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<AdditiveEQPBinomialTree>(timeSteps)));
        americanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<AdditiveEQPBinomialTree>(timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        method = "Binomial Trigeorgis";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Trigeorgis>(timeSteps)));
        bermudanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Trigeorgis>(timeSteps)));
        americanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Trigeorgis>(timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        method = "Binomial Tian";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Tian>(timeSteps)));
        bermudanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Tian>(timeSteps)));
        americanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<Tian>(timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
                  << std::endl;

        method = "Binomial Leisen-Reimer";
        europeanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<LeisenReimer>(timeSteps)));
        bermudanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<LeisenReimer>(timeSteps)));
        americanOption.setPricingEngine(boost::shared_ptr<PricingEngine>(
            new BinomialVanillaEngine<LeisenReimer>(timeSteps)));
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << bermudanOption.NPV()
                  << std::setw(widths[3]) << std::left << americanOption.NPV()
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
        europeanOption.setPricingEngine(mcengine1);
        // Real errorEstimate = europeanOption.errorEstimate();
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << "N/A"
                  << std::endl;

        method = "MC (Sobol)";
        Size nSamples = 32768;  // 2^15

        boost::shared_ptr<PricingEngine> mcengine2;
        mcengine2 =
            MakeMCEuropeanEngine<LowDiscrepancy>().withSteps(timeSteps)
                                                  .withSamples(nSamples);
        europeanOption.setPricingEngine(mcengine2);
        std::cout << std::setw(widths[0]) << std::left << method
                  << std::fixed
                  << std::setw(widths[1]) << std::left << europeanOption.NPV()
                  << std::setw(widths[2]) << std::left << "N/A"
                  << std::setw(widths[3]) << std::left << "N/A"
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
