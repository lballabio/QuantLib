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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file HedgingError.cpp
    \brief Example about using Montecarlo Path Pricer

    \fullpath Examples/HedgingError/HedgingError.cpp
*/

// $Log$
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
//
// Revision 1.4  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.3  2001/07/25 11:02:39  nando
// 80 columns enforced
//
// Revision 1.2  2001/07/24 16:54:05  dzuki
// Minor fixes.
//
// Revision 1.1  2001/07/24 16:36:58  dzuki
// Discontinuous hedging error calculation example (HedgingError)


#include "stdlib.h"
#include <iostream>

#include "ql\quantlib.hpp"

using namespace std;

using namespace QuantLib;
using namespace QuantLib::Pricers;
using namespace QuantLib::MonteCarlo;



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
    double computePlainVanilla(Option::Type type,
                               double price,
                               double strike,
                               double discount) const;
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
    BSMEuropeanOption option = BSMEuropeanOption(type_, stock, strike_, 0.0, r_,
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
        BSMEuropeanOption option = BSMEuropeanOption(type_, stock, strike_,
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
    double optionPayoff = computePlainVanilla(type_, stock, strike_, 1.0);
    double profitLoss = stockAmount*stock + money_account - optionPayoff;

    return profitLoss;
}

double HedgeErrorPathPricer::computePlainVanilla(Option::Type type,
                                                 double price,
                                                 double strike,
                                                 double discount) const
{
    double optionPrice;

    switch (type) {
      case Option::Call:
            optionPrice = QL_MAX(price-strike,0.0);
        break;
      case Option::Put:
            optionPrice = QL_MAX(strike-price,0.0);
        break;
      case Option::Straddle:
            optionPrice = QL_FABS(strike-price);
    }
    return discount*optionPrice;
}

class ComputeHedgingError
{
public:
    ComputeHedgingError(Time maturity,
                        double strike,
                        double s0,
                        double sigma,
                        Rate r)
    {
        maturity_ = maturity;
        strike_ = strike;
        s0_ = s0;
        sigma_ = sigma;
        r_ = r;
    }

    void doTest(int nTimeSteps = 21, int nSamples = 50000)
    {
        BSMEuropeanOption option = BSMEuropeanOption(Option::Type::Call, s0_,
            strike_, 0.0, r_, maturity_, sigma_);

        cout << "\n" << "Option value:\t " << option.value() << "\n";

        double theoretical_error_sd =
                QL_SQRT(3.1415926535/4/nTimeSteps)*option.vega()*sigma_;

        cout << "Value of the hedging error SD"
                " computed using Derman & Kamal's formula:\t "
                << theoretical_error_sd;

        double tau = maturity_ / nTimeSteps;

        // Black Scholes assumption
        double sigma = sigma_* sqrt(tau);
        double mean = r_ * tau - 0.5*sigma*sigma;

        Math::Statistics statisticAccumulator;

        Handle<StandardPathGenerator> myPathGenerator(
            new StandardPathGenerator(nTimeSteps, mean, sigma*sigma));

        // casting HedgeErrorPathPricer to Handle<PathPricer>
        Handle<PathPricer> myPathPricer =
            Handle<PathPricer>(new HedgeErrorPathPricer(Option::Type::Call,
            s0_, strike_, r_, maturity_, sigma_));

        OneFactorMonteCarloOption myMontecarloSimulation(myPathGenerator,
            myPathPricer, statisticAccumulator);

        // simulating nSamples paths
        myMontecarloSimulation.addSamples(nSamples);

        Math::Statistics ProfitLossDistribution =
            myMontecarloSimulation.sampleAccumulator();

        cout << "\n";
        cout << "Mean hedging error: \t" <<ProfitLossDistribution.mean()<< "\n";
        cout << "Hedging error standard deviation: \t"
            << ProfitLossDistribution.standardDeviation() << "\n";
    }

private:
    double s0_;
    double strike_;
    Time maturity_;
    double sigma_;
    Rate r_;

};

int main(int argc, char* argv[])
{
    // 1-month at the money call, volatility 20%, risk-free rate 5%
    // underlying price 100.0, strike price 100.0
    ComputeHedgingError test(1./12., 100., 100., 0.2, 0.05);
    test.doTest();
    test.doTest(84);
    return 0;
}
