/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/* \file Parities.cpp
   \brief Example on pricing european option with different methods.

*/

// $Id$

#include "ql/quantlib.hpp"

// Rate and Time are just double, but having their own types allows for
// a stronger check at compile time
using QuantLib::Rate;
using QuantLib::Time;

// Option is a helper class that holds the enumeration {Call, Put, Straddle}
using QuantLib::Option;

// Handle is the QuantLib way to have reference-counted objects
using QuantLib::Handle;

// class for statistical analysis
using QuantLib::Math::Statistics;

// single Path of a random variable
// It contains the list of variations
using QuantLib::MonteCarlo::Path;

// the pricer computes final portfolio's value for each random variable path
using QuantLib::MonteCarlo::PathPricer;

// the path generators
using QuantLib::MonteCarlo::GaussianPathGenerator;

// the pricing model for option on a single asset
using QuantLib::MonteCarlo::OneFactorMonteCarloOption;

// the more general model for option on a single asset
using QuantLib::MonteCarlo::MonteCarloModel;

using QuantLib::MonteCarlo::EuropeanPathPricer;
using QuantLib::MonteCarlo::UniformRandomGenerator;
using QuantLib::Pricers::EuropeanOption;
using QuantLib::Pricers::FiniteDifferenceEuropean;

// to format the output of doubles
using QuantLib::DoubleFormatter;

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
        double underlying = 100;
        double strike = 100;      // at the money
        Rate dividendYield = 0.0; // no dividends
        Rate riskFreeRate = 0.05; // 5%
        Time maturity = 1.0;      // 1 year
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
        unsigned int grid = 100;
        value = FiniteDifferenceEuropean(Option::Call, underlying, strike,
            dividendYield, riskFreeRate, maturity, volatility, grid).value();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << "N/A\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;




        // Monte Carlo methods
        // for plain vanilla european option the number of steps is not
        // significant. Let's go for the fastest way: just one step
        int nTimeSteps = 1;
        int nSamples = 200000;
        long seed = 100000*UniformRandomGenerator().next();
	    double drift = riskFreeRate - 0.5*volatility*volatility;
        Statistics samples;
        Handle<GaussianPathGenerator> myPathGenerator(
            new GaussianPathGenerator(drift, volatility*volatility,
                maturity, nTimeSteps, seed));


        // fifth method:  MonteCarlo
        method ="Monte Carlo";
        bool antitheticVariance = false;
        // The European path pricer
        Handle<PathPricer> myEuropeanPathPricer =
            Handle<PathPricer>(new EuropeanPathPricer(Option::Call,
            underlying, strike, exp(-riskFreeRate*maturity),
            antitheticVariance));
        // The OneFactorMontecarloModel generates paths using myPathGenerator
        // each path is priced using myPathPricer
        // prices will be accumulated into samples
	    OneFactorMonteCarloOption mc(myPathGenerator, myEuropeanPathPricer,
		    samples);
        // the model simulates nSamples paths
        mc.addSamples(nSamples);
        // the sampleAccumulator method of OneFactorMonteCarloOption
        // gives access to all the methods of statisticAccumulator
        value = mc.sampleAccumulator().mean();
        estimatedError = mc.sampleAccumulator().errorEstimate();
        discrepancy = QL_FABS(value-rightValue);
        relativeDiscrepancy = discrepancy/rightValue;
        std::cout << method << "\t"
             << DoubleFormatter::toString(value, 4) << "\t"
             << DoubleFormatter::toString(estimatedError, 4) << "\t\t"
             << DoubleFormatter::toString(discrepancy, 6) << "\t"
             << DoubleFormatter::toString(relativeDiscrepancy, 6)
             << std::endl;

        // sixth method:  MonteCarlo with antithetic variance reduction
        method ="MC antithetic";
        antitheticVariance = true;
        // The European path pricer, this time with antithetic variance
        // reduction
        myEuropeanPathPricer =
            Handle<PathPricer>(new EuropeanPathPricer(Option::Call,
            underlying, strike, exp(-riskFreeRate*maturity),
            antitheticVariance));
        // reset the statistic accumulator
        samples.reset();
	    mc = OneFactorMonteCarloOption(myPathGenerator, myEuropeanPathPricer,
		    samples);
        mc.addSamples(nSamples);
        value = mc.sampleAccumulator().mean();
        estimatedError = mc.sampleAccumulator().errorEstimate();
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
