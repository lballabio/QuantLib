
/*!
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

//! $Id$

#include <ql/quantlib.hpp>

using namespace QuantLib;

using QuantLib::Pricers::EuropeanOption;
using QuantLib::Pricers::McEuropean;
using QuantLib::Pricers::FdEuropean;
using QuantLib::TermStructures::FlatForward;
using QuantLib::VolTermStructures::BlackConstantVol;

// helper function for option payoff: MAX((stike-underlying),0), etc.
using QuantLib::Pricers::ExercisePayoff;

// This will be included in the library after a bit of redesign
class Payoff : public QL::ObjectiveFunction{
    public:
        Payoff(Time maturity,
               double strike,
               double s0,
               double sigma,
               Rate r,
               Rate q)
        : maturity_(maturity),
        strike_(strike),
        s0_(s0),
        sigma_(sigma),r_(r), q_(q){}

        double operator()(double x) const {
           double nuT = (r_-q_-0.5*sigma_*sigma_)*maturity_;
           return QL_EXP(-r_*maturity_)
               *ExercisePayoff(Option::Call, s0_*QL_EXP(x), strike_)
               *QL_EXP(-(x - nuT)*(x -nuT)/(2*sigma_*sigma_*maturity_))
               /QL_SQRT(2.0*3.141592*sigma_*sigma_*maturity_);
        }
private:
    Time maturity_;
    double strike_;
    double s0_;
    double sigma_;
    Rate r_,q_;
};


int main(int argc, char* argv[])
{
    try {
        QL_IO_INIT

        // our option
        double underlying = 102;
        double strike = 100;      // at the money
        Spread dividendYield = 0.01; // 1%
        Rate riskFreeRate = 0.05; // 5%

        Date todaysDate(15, May, 1999);
        Date settlementDate(17, May, 1999);
        Date exerciseDate(17, August, 1999); // 3 months
        DayCounter rateDayCounter = DayCounters::Actual365();
        Time maturity = rateDayCounter.yearFraction(settlementDate,
            exerciseDate);

        double volatility = 0.20; // 20%
        std::cout << "Time to maturity = "        << maturity
                  << std::endl;
        std::cout << "Underlying price = "        << underlying
                  << std::endl;
        std::cout << "Strike = "                  << strike
                  << std::endl;
        std::cout << "Risk-free interest rate = " << riskFreeRate
                  << std::endl;
        std::cout << "Volatility = "              << volatility
                  << std::endl;
        std::cout << std::endl;

        std::string method;
        double value, discrepancy, rightValue, relativeDiscrepancy;

        
        std::cout << std::endl << std::endl ;
        
        // write column headings
        std::cout << "Method\t\tValue\tEstimatedError\tDiscrepancy"
            "\tRel. Discr." << std::endl;



        // first method: Black Scholes analytic solution
        method ="Black Scholes";
        value = EuropeanOption(Option::Call, underlying, strike,
            dividendYield, riskFreeRate, maturity, volatility).value();
        double estimatedError = 0.0;
        discrepancy = 0.0;
        relativeDiscrepancy = 0.0;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << DoubleFormatter::toString(estimatedError, 4) << "\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;


        // store the Black Scholes value as the correct one
        rightValue = value;





        // second method: Call-Put parity
        method ="Call-Put parity";
        value = EuropeanOption(Option::Put, underlying, strike,
            dividendYield, riskFreeRate, maturity, volatility).value()
            + underlying*QL_EXP(-dividendYield*maturity) - strike*QL_EXP(- riskFreeRate*maturity);
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << discrepancy << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;


        // third method: Integral
        method ="Integral";
        using QuantLib::Math::SegmentIntegral;
        Payoff po(maturity, strike, underlying, volatility, riskFreeRate,
            dividendYield);
        SegmentIntegral integrator(5000);

        double nuT = (riskFreeRate - dividendYield + 0.5*volatility*volatility)*maturity;
        double infinity = 10.0*volatility*QL_SQRT(maturity);

        value = integrator(po, nuT-infinity, nuT+infinity);
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;




        // fourth method: Finite Differences
        method ="Finite Diff.";
        Size grid = 100;
        value = FdEuropean(Option::Call, underlying, strike,
            dividendYield, riskFreeRate, maturity, volatility, grid).value();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;




        // fifth method: Monte Carlo (crude)
        method ="MC (crude)";
        bool antitheticVariance = false;
        McEuropean mcEur(Option::Call, underlying, strike, dividendYield,
            riskFreeRate, maturity, volatility, antitheticVariance);
        // let's require a tolerance of 0.002%
        value = mcEur.value(0.02);
        estimatedError = mcEur.errorEstimate();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << DoubleFormatter::toString(estimatedError, 4) << "\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // sixth method: Monte Carlo with antithetic variance reduction
        method ="MC (antithetic)";
        // let's use the same number of samples as in the crude Monte Carlo
        Size nSamples = mcEur.sampleAccumulator().samples();
        antitheticVariance = true;
        McEuropean mcEur2(Option::Call, underlying, strike, dividendYield,
            riskFreeRate, maturity, volatility, antitheticVariance);
        value = mcEur2.valueWithSamples(nSamples);
        estimatedError = mcEur2.errorEstimate();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << DoubleFormatter::toString(estimatedError, 4) << "\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;



        // New option pricing framework 
        std::cout << "\nNew Pricing engine framework" << std::endl;

        EuropeanExercise exercise(exerciseDate);


        RelinkableHandle<MarketElement> underlyingH(
            Handle<MarketElement>(new SimpleMarketElement(underlying)));

        // bootstrap the yield/dividend/vol curves
        RelinkableHandle<TermStructure> flatTermStructure(
            Handle<TermStructure>(
                new FlatForward(todaysDate, settlementDate,
                                riskFreeRate, rateDayCounter)));
        RelinkableHandle<TermStructure> flatDividendTS(
            Handle<TermStructure>(
                new FlatForward(todaysDate, settlementDate,
                                dividendYield, rateDayCounter)));
        RelinkableHandle<BlackVolTermStructure> flatVolTS(
            Handle<BlackVolTermStructure>(
                new BlackConstantVol(settlementDate, volatility)));


        Instruments::VanillaOption option(
            Option::Call,
            underlyingH,
            strike,
            flatDividendTS,
            flatTermStructure,
            exercise,
            flatVolTS,
            Handle<PricingEngine>(
                new PricingEngines::EuropeanAnalyticalEngine()));
            
        // method: Black Scholes Engine
        method = "Black Scholes";
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Binomial Method (JR)
        method = "Binomial (JR)";
        option.setPricingEngine(Handle<PricingEngine>(
            new PricingEngines::EuropeanBinomialEngine(
                PricingEngines::EuropeanBinomialEngine::JarrowRudd, 800)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // Binomial Method (CRR)
        method = "Binomial (CRR)";
        option.setPricingEngine(Handle<PricingEngine>(
            new PricingEngines::EuropeanBinomialEngine(
                PricingEngines::EuropeanBinomialEngine::CoxRossRubinstein, 800)));
        value = option.NPV();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;
        

        Handle<PricingEngines::EuropeanAnalyticalEngine> baseEngine(new
            PricingEngines::EuropeanAnalyticalEngine);
        Handle<PricingEngines::QuantoVanillaAnalyticEngine> quantoEngine(new
            PricingEngines::QuantoVanillaAnalyticEngine(baseEngine));

        double correlation = 0.0;
        Instruments::QuantoVanillaOption quantoOption(
            Option::Call,
            underlyingH,
            strike,
            flatDividendTS,
            flatTermStructure,
            exercise,
            flatVolTS,
            quantoEngine,
            flatTermStructure,
            flatVolTS,
            RelinkableHandle<MarketElement>(
                Handle<MarketElement>(new SimpleMarketElement(correlation))));
            
        value = quantoOption.NPV();
        double delta = quantoOption.delta();
        double gamma = quantoOption.gamma();
        double theta = quantoOption.theta();
        double vega = quantoOption.vega();
        double rho = quantoOption.rho();
        double divRho = quantoOption.dividendRho();
        double qvega = quantoOption.qvega();
        double qrho = quantoOption.qrho();
        double qlambda = quantoOption.qlambda();
        std::cout << std::endl << std::endl << "quanto: "
             << DoubleFormatter::toString(value, 4)
             << std::endl;
        std::cout << "quanto delta: "
             << DoubleFormatter::toString(delta, 4)
             << std::endl;
        std::cout << "quanto gamma: "
             << DoubleFormatter::toString(gamma, 4)
             << std::endl;
        std::cout << "quanto theta: "
             << DoubleFormatter::toString(theta, 4)
             << std::endl;
        std::cout << "quanto vega: "
             << DoubleFormatter::toString(vega, 4)
             << std::endl;
        std::cout << "quanto rho: "
             << DoubleFormatter::toString(rho, 4)
             << std::endl;
        std::cout << "quanto divRho: "
             << DoubleFormatter::toString(divRho, 4)
             << std::endl;
        std::cout << "quanto qvega: "
             << DoubleFormatter::toString(qvega, 4)
             << std::endl;
        std::cout << "quanto qrho: "
             << DoubleFormatter::toString(qrho, 4)
             << std::endl;
        std::cout << "quanto qlambda: "
             << DoubleFormatter::toString(qlambda, 4)
             << std::endl;


        Handle<PricingEngines::ForwardVanillaAnalyticEngine> forwardEngine(new
            PricingEngines::ForwardVanillaAnalyticEngine(baseEngine));

        Instruments::ForwardVanillaOption forwardOption(
            Option::Call,
            underlyingH,
            flatDividendTS,
            flatTermStructure,
            exercise,
            flatVolTS,
            forwardEngine,
            1.1, // moneyness
            settlementDate.plusMonths(1) // reset Date
            );
            
        value   = forwardOption.NPV();
        delta   = forwardOption.delta();
        gamma   = forwardOption.gamma();
        theta   = forwardOption.theta();
        vega    = forwardOption.vega();
        rho     = forwardOption.rho();
        divRho  = forwardOption.dividendRho();
        std::cout << std::endl << std::endl << "forward: "
             << DoubleFormatter::toString(value, 4)
             << std::endl;
        std::cout << "forward delta: "
             << DoubleFormatter::toString(delta, 4)
             << std::endl;
        std::cout << "forward gamma: "
             << DoubleFormatter::toString(gamma, 4)
             << std::endl;
        std::cout << "forward theta: "
             << DoubleFormatter::toString(theta, 4)
             << std::endl;
        std::cout << "forward vega: "
             << DoubleFormatter::toString(vega, 4)
             << std::endl;
        std::cout << "forward rho: "
             << DoubleFormatter::toString(rho, 4)
             << std::endl;
        std::cout << "forward divRho: "
             << DoubleFormatter::toString(divRho, 4)
             << std::endl;


        Handle<PricingEngines::ForwardPerformanceVanillaAnalyticEngine>
            forwardPerformanceEngine(new
            PricingEngines::ForwardPerformanceVanillaAnalyticEngine(baseEngine));

        forwardOption.setPricingEngine(forwardPerformanceEngine);

        value   = forwardOption.NPV();
        delta   = forwardOption.delta();
        gamma   = forwardOption.gamma();
        theta   = forwardOption.theta();
        vega    = forwardOption.vega();
        rho     = forwardOption.rho();
        divRho  = forwardOption.dividendRho();
        std::cout << std::endl << std::endl << "forward performance: "
             << DoubleFormatter::toString(value, 4)
             << std::endl;
        std::cout << "forward performance delta: "
             << DoubleFormatter::toString(delta, 4)
             << std::endl;
        std::cout << "forward performance gamma: "
             << DoubleFormatter::toString(gamma, 4)
             << std::endl;
        std::cout << "forward performance theta: "
             << DoubleFormatter::toString(theta, 4)
             << std::endl;
        std::cout << "forward performance vega: "
             << DoubleFormatter::toString(vega, 4)
             << std::endl;
        std::cout << "forward performance rho: "
             << DoubleFormatter::toString(rho, 4)
             << std::endl;
        std::cout << "forward performance divRho: "
             << DoubleFormatter::toString(divRho, 4)
             << std::endl;



///////////////////////////////////


        Handle<PricingEngines::QuantoVanillaAnalyticEngine>
            quantoForwardEngine(new
            PricingEngines::QuantoVanillaAnalyticEngine(forwardEngine));
        quantoOption.setPricingEngine(quantoForwardEngine);

        value   = quantoOption.NPV();
        delta   = quantoOption.delta();
        gamma   = quantoOption.gamma();
        theta   = quantoOption.theta();
        vega    = quantoOption.vega();
        rho     = quantoOption.rho();
        divRho  = quantoOption.dividendRho();
        qvega   = quantoOption.qvega();
        qrho    = quantoOption.qrho();
        qlambda = quantoOption.qlambda();
        std::cout << std::endl << std::endl << "quanto forward: "
             << DoubleFormatter::toString(value, 4)
             << std::endl;
        std::cout << "quanto forward delta: "
             << DoubleFormatter::toString(delta, 4)
             << std::endl;
        std::cout << "quanto forward gamma: "
             << DoubleFormatter::toString(gamma, 4)
             << std::endl;
        std::cout << "quanto forward theta: "
             << DoubleFormatter::toString(theta, 4)
             << std::endl;
        std::cout << "quanto forward vega: "
             << DoubleFormatter::toString(vega, 4)
             << std::endl;
        std::cout << "quanto forward rho: "
             << DoubleFormatter::toString(rho, 4)
             << std::endl;
        std::cout << "quanto forward divRho: "
             << DoubleFormatter::toString(divRho, 4)
             << std::endl;
        std::cout << "quanto forward qvega: "
             << DoubleFormatter::toString(qvega, 4)
             << std::endl;
        std::cout << "quanto forward qrho: "
             << DoubleFormatter::toString(qrho, 4)
             << std::endl;
        std::cout << "quanto forward qlambda: "
             << DoubleFormatter::toString(qlambda, 4)
             << std::endl;



        
        
        Handle<PricingEngines::QuantoVanillaAnalyticEngine>
            quantoForwardPerformanceEngine(new
            PricingEngines::QuantoVanillaAnalyticEngine(forwardPerformanceEngine));
        quantoOption.setPricingEngine(quantoForwardPerformanceEngine);

        value   = quantoOption.NPV();
        delta   = quantoOption.delta();
        gamma   = quantoOption.gamma();
        theta   = quantoOption.theta();
        vega    = quantoOption.vega();
        rho     = quantoOption.rho();
        divRho  = quantoOption.dividendRho();
        qvega   = quantoOption.qvega();
        qrho    = quantoOption.qrho();
        qlambda = quantoOption.qlambda();
        std::cout << std::endl << std::endl << "quanto forward performance: "
             << DoubleFormatter::toString(value, 4)
             << std::endl;
        std::cout << "quanto forward performance delta: "
             << DoubleFormatter::toString(delta, 4)
             << std::endl;
        std::cout << "quanto forward performance gamma: "
             << DoubleFormatter::toString(gamma, 4)
             << std::endl;
        std::cout << "quanto forward performance theta: "
             << DoubleFormatter::toString(theta, 4)
             << std::endl;
        std::cout << "quanto forward performance vega: "
             << DoubleFormatter::toString(vega, 4)
             << std::endl;
        std::cout << "quanto forward performance rho: "
             << DoubleFormatter::toString(rho, 4)
             << std::endl;
        std::cout << "quanto forward performance divRho: "
             << DoubleFormatter::toString(divRho, 4)
             << std::endl;
        std::cout << "quanto forward performance qvega: "
             << DoubleFormatter::toString(qvega, 4)
             << std::endl;
        std::cout << "quanto forward performance qrho: "
             << DoubleFormatter::toString(qrho, 4)
             << std::endl;
        std::cout << "quanto forward performance qlambda: "
             << DoubleFormatter::toString(qlambda, 4)
             << std::endl;





            
        return 0;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}
