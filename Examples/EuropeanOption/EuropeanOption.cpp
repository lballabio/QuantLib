
/*!
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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

// helper function for option payoff: MAX((stike-underlying),0), etc.
using QuantLib::Pricers::ExercisePayoff;

// This will be included in the library after a bit of redesign
class Payoff : public QL::ObjectiveFunction{
    public:
        Payoff(Time maturity,
               double strike,
               double s0,
               double sigma,
               Rate r)
        : maturity_(maturity),
        strike_(strike),
        s0_(s0),
        sigma_(sigma),r_(r){}

        double operator()(double x) const {
           double nuT = (r_-0.5*sigma_*sigma_)*maturity_;
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
    Rate r_;
};


int main(int argc, char* argv[])
{
    try {
        // our option
        double underlying = 102;
        double strike = 100;      // at the money
        Spread dividendYield = 0.0; // no dividends
        Rate riskFreeRate = 0.05; // 5%
        Time maturity = 0.25;      // 3 months
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
            + underlying - strike*QL_EXP(- riskFreeRate*maturity);
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
        Payoff po(maturity, strike, underlying, volatility, riskFreeRate);
        SegmentIntegral integrator(5000);

        double nuT = (riskFreeRate - 0.5*volatility*volatility)*maturity;
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

        Date todaysDate(15, February, 1999);
        Date settlementDate(17, February, 1999);
        DayCounter depositDayCounter = DayCounters::Thirty360();

        // bootstrap the curve
        Handle<TermStructure> flatTermStructure(new
            TermStructures::FlatForward(todaysDate, settlementDate,
            riskFreeRate, depositDayCounter));

        Instruments::VanillaOption option(
            Option::Call,
            Handle<MarketElement>(new SimpleMarketElement(underlying)),
            strike,
            Handle<TermStructure>(),
            flatTermStructure,
            settlementDate.plus(3, Months),
            Handle<MarketElement>(new SimpleMarketElement(volatility)),
            Handle<PricingEngine>(new PricingEngines::EuropeanAnalyticalEngine())
            );
            

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
            Handle<MarketElement>(new SimpleMarketElement(underlying)),
            strike,
            Handle<TermStructure>(),
            flatTermStructure,
            settlementDate.plus(3, Months),
            Handle<MarketElement>(new SimpleMarketElement(volatility)),
            quantoEngine,
            flatTermStructure,
            Handle<MarketElement>(new SimpleMarketElement(volatility)),
            Handle<MarketElement>(new SimpleMarketElement(correlation))
            );
            
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
        std::cout << std::endl << "quanto delta: "
             << DoubleFormatter::toString(delta, 4)
             << std::endl;
        std::cout << std::endl << "quanto gamma: "
             << DoubleFormatter::toString(gamma, 4)
             << std::endl;
        std::cout << std::endl << "quanto theta: "
             << DoubleFormatter::toString(theta, 4)
             << std::endl;
        std::cout << std::endl << "quanto vega: "
             << DoubleFormatter::toString(vega, 4)
             << std::endl;
        std::cout << std::endl << "quanto rho: "
             << DoubleFormatter::toString(rho, 4)
             << std::endl;
        std::cout << std::endl << "quanto divRho: "
             << DoubleFormatter::toString(divRho, 4)
             << std::endl;
        std::cout << std::endl << "quanto qvega: "
             << DoubleFormatter::toString(qvega, 4)
             << std::endl;
        std::cout << std::endl << "quanto qrho: "
             << DoubleFormatter::toString(qrho, 4)
             << std::endl;
        std::cout << std::endl << "quanto qlambda: "
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
