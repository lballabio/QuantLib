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

/*! \file HedgingError.cpp
    \brief Example about using Montecarlo Path Pricer

    \fullpath Examples/HedgingError/HedgingError.cpp
*/

// $Log$
// Revision 1.9  2001/08/07 11:25:52  sigmud
// copyright header maintenance
//
// Revision 1.8  2001/08/06 16:49:17  nando
// 1) BSMFunction now is VolatilityFunction
// 2) Introduced ExercisePayoff (to be reworked later)
//
// Revision 1.7  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.6  2001/07/27 17:28:31  nando
// minor changes
//
// Revision 1.5  2001/07/27 16:47:25  nando
// updated in order to compile with the new GeneralMontecarlo interface.
//
// Added comments, renamed variables, removed tabs (4 spaces),
// style enforced.
//
// still to do:
// 1) add hedgingerror.hpp
// 2) avoid using namespace QuantLib in favour of
//     using QuantLib::Rate

#include "stdlib.h"
#include <iostream>

#include "ql\quantlib.hpp"

using QuantLib::Rate;
using QuantLib::Time;
using QuantLib::Option;
using QuantLib::Handle;
using QuantLib::Pricers::ExercisePayoff;
using QuantLib::Pricers::EuropeanOption;
using QuantLib::Math::Statistics;
using QuantLib::MonteCarlo::Path;
using QuantLib::MonteCarlo::PathPricer;
using QuantLib::MonteCarlo::StandardPathGenerator;
using QuantLib::MonteCarlo::OneFactorMonteCarloOption;


class HedgeErrorPathPricer : public PathPricer
{
  public:
    HedgeErrorPathPricer():PathPricer(){}
    HedgeErrorPathPricer(Option::Type type,
                         double underlying,
                         double strike,
                         Rate r,
                         Time maturity,
                         double sigma);
    double value(const Path &path) const;
  protected:
    Option::Type type_;
    Rate r_;
    double underlying_, strike_, sigma_;
    Time maturity_;
};

HedgeErrorPathPricer::HedgeErrorPathPricer(Option::Type type,
                                           double underlying,
                                           double strike,
                                           double r,
                                           Time maturity,
                                           double sigma)
: type_(type),underlying_(underlying), strike_(strike), r_(r),
  maturity_(maturity), sigma_(sigma)
{
    QL_REQUIRE(strike_ > 0.0,
        "HedgeErrorPathPricer: strike must be positive");
    QL_REQUIRE(underlying_ > 0.0,
        "HedgeErrorPathPricer: underlying must be positive");
    QL_REQUIRE(r_ >= 0.0, "HedgeErrorPathPricer: risk free rate (r) must"
        " be positive or zero");
    QL_REQUIRE(maturity_ > 0.0,
        "HedgeErrorPathPricer: maturity must be positive");
    QL_REQUIRE(sigma_ >= 0.0, "HedgeErrorPathPricer: volatility (sigma)"
        " must be positive or zero");

    isInitialized_ = true;
}

double HedgeErrorPathPricer::value(const Path & path) const
{
    QL_REQUIRE(isInitialized_,
        "HedgeErrorPathPricer: pricer not initialized");

    int n = path.size();
    QL_REQUIRE(n>0,
        "HedgeErrorPathPricer: the path cannot be empty");

    // discrete hedging interval
    double dt = maturity_/n;

    // initialize stock value at the start of the path
    double stockLogGrowth = 0.0;
    double stock = underlying_;


    /************************/
    /*** the initial deal ***/
    /************************/
    // option value at the start of the path
    EuropeanOption option = EuropeanOption(type_, stock, strike_, 0.0, r_,
        maturity_, sigma_);
    // sell the option, cash in its premium
    double money_account = option.value();
    // compute delta
    double delta = option.delta();
    // delta-hedge the option
    double stockAmount = delta;
    money_account -= stockAmount*stock;



    /**********************************/
    /*** hedging during option life ***/
    /**********************************/
    for(int step = 0; step < n-1; step++){
        // accruing on the money account
        money_account *= QL_EXP( r_*dt );

        // stock growth
        stockLogGrowth += path[step];
        stock = underlying_*QL_EXP(stockLogGrowth);

        // recalculate option value
        EuropeanOption option = EuropeanOption(type_, stock, strike_,
            0.0, r_, maturity_ - dt*(step+1), sigma_);

        // recalculate delta
        delta = option.delta();

        // re-hedging
        money_account -= (delta - stockAmount)*stock;
        stockAmount = delta;
    }



    /*************************/
    /*** option expiration ***/
    /*************************/
    // last accrual on my money account
    money_account *= QL_EXP( r_*dt );
    // last stock growth
    stockLogGrowth += path[n-1];
    stock = underlying_*QL_EXP(stockLogGrowth);

    /*** final Profit&Loss valuation ***/
    double optionPayoff = ExercisePayoff(type_, stock, strike_);
    double profitLoss = stockAmount*stock + money_account - optionPayoff;

    return profitLoss;
}

class ComputeHedgingError
{
public:
    ComputeHedgingError(Option::Type type,
                        Time maturity,
                        double strike,
                        double s0,
                        double sigma,
                        Rate r)
    {
        type_ = type;
        maturity_ = maturity;
        strike_ = strike;
        s0_ = s0;
        sigma_ = sigma;
        r_ = r;

        EuropeanOption option = EuropeanOption(type_, s0_,
            strike_, 0.0, r_, maturity_, sigma_);

        std::cout << "\n" << "Option value:\t " << option.value() << "\n";

        vega_ = option.vega();
    }

    void doTest(int nTimeSteps, int nSamples)
    {
        // hedging interval
        double tau = maturity_ / nTimeSteps;

        // Black Scholes assumption rules the path generator
        double sigma = sigma_* sqrt(tau);
        double mean = r_ * tau - 0.5*sigma*sigma;
        Handle<StandardPathGenerator> myPathGenerator(
            new StandardPathGenerator(nTimeSteps, mean, sigma*sigma));

        // the path pricer knows how to price a path
        Handle<PathPricer> myPathPricer =
            Handle<PathPricer>(new HedgeErrorPathPricer(type_,
            s0_, strike_, r_, maturity_, sigma_));

        // a statistic accumulator to accumulate path values
        Statistics statisticAccumulator;

        // use one factor Montecarlo model
        OneFactorMonteCarloOption myMontecarloSimulation(myPathGenerator,
            myPathPricer, statisticAccumulator);

        // simulate nSamples paths
        myMontecarloSimulation.addSamples(nSamples);

        // get back the value distribution
        Statistics ProfitLossDistribution =
            myMontecarloSimulation.sampleAccumulator();

        std::cout << "\n";
        std::cout << "Montecarlo simulation: " << nSamples
            << " samples" << "\n";
        std::cout << "Hedging " << nTimeSteps << " times" << "\n";
        std::cout << "Mean Profit&Loss: \t" << ProfitLossDistribution.mean()
            << "\n";
        std::cout << "Profit&Loss standard deviation (Montecarlo) :\t"
             << ProfitLossDistribution.standardDeviation() << "\n";
        double theoretical_error_sd =
                QL_SQRT(3.1415926535/4/nTimeSteps)*vega_*sigma_;

        std::cout << "Profit&Loss StDev (Derman & Kamal's formula):\t"
                << theoretical_error_sd << "\n";

        std::cout << "Profit&Loss skewness           (Montecarlo) :\t"
            << ProfitLossDistribution.skewness() << "\n";
        std::cout << "Profit&Loss excess kurtosis    (Montecarlo) :\t"
             << ProfitLossDistribution.kurtosis() << "\n";

    }

private:
    Option::Type type_;
    double s0_;
    double strike_;
    Time maturity_;
    double sigma_;
    Rate r_;
    double vega_;
};

int main(int argc, char* argv[])
{
    // 1-month call, volatility 20%, risk-free rate 5%
    // strike price 100.0, underlying price 100.0
    Time maturity = 1./12.;
    double strike = 100;
    double underlying = 100;
    double volatility = 0.20;
    Rate riskFreeRate = 0.05;
    ComputeHedgingError test(Option::Type::Call, maturity, strike, underlying,
        volatility, riskFreeRate);

    test.doTest( 1, 50000);
    test.doTest( 4, 50000);
    test.doTest(21, 50000);
    test.doTest(84, 50000);

    return 0;
}
