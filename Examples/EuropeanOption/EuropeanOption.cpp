/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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

        // write column headings
        std::cout << "Method\t\tValue\tEstimatedError\tDiscrepancy"
            "\tRel. Discr." << std::endl;



        // first method: Black Scholes analytic solution
        std::string method ="Black Scholes";
        double value = EuropeanOption(Option::Call, underlying, strike,
            dividendYield, riskFreeRate, maturity, volatility).value();
        double estimatedError = 0.0;
        double discrepancy = 0.0;
        double relativeDiscrepancy = 0.0;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << DoubleFormatter::toString(estimatedError, 4) << "\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;


        // store the Black Scholes value as the correct one
        double rightValue = value;





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
        value = mcEur.value(0.002);
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


        return 0;
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "unknown error" << std::endl;
        return 1;
    }
}
