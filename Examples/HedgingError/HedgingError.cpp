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

/*
    $Id$
*/

// $Source$
// $Log$
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
//


#include "stdlib.h"
#include <iostream>

#include "ql\quantlib.hpp"

using namespace std;

using namespace QuantLib;
using namespace QuantLib::Pricers;
using namespace QuantLib::MonteCarlo;



class PathHedgeErrorCalculator : public PathPricer
{
  public:
    PathHedgeErrorCalculator():PathPricer(){}
    PathHedgeErrorCalculator(Option::Type type, double underlying,
        double strike, Rate r, double maturity, double sigma);

	double value(const Path &path) const;

	double computePlainVanilla(Option::Type type, double price,
								double strike, double discount) const;

  protected:

	mutable Option::Type type_;
	mutable Rate r_;
    mutable double underlying_, strike_, maturity_, sigma_;

};

PathHedgeErrorCalculator::PathHedgeErrorCalculator(Option::Type type,
                                    double underlying,
                                    double strike,
                                    double r,
                                    double maturity,
                                    double sigma)
            :   type_(type),underlying_(underlying),
                strike_(strike), r_(r),
                maturity_(maturity), sigma_(sigma)
{
    QL_REQUIRE(strike_ > 0.0,
        "PathHedgeErrorCalculator: strike must be positive");
    QL_REQUIRE(underlying_ > 0.0,
        "PathHedgeErrorCalculator: underlying must be positive");
    QL_REQUIRE(r_ >= 0.0, "PathHedgeErrorCalculator: risk free rate (r) must"
        " be positive or zero");
   QL_REQUIRE(maturity_ > 0.0,
        "PathHedgeErrorCalculator: maturity must be positive");
   QL_REQUIRE(sigma_ >= 0.0, "PathHedgeErrorCalculator: volatility (sigma)"
        " must be positive or zero");

   isInitialized_ = true;
}

double PathHedgeErrorCalculator::value(const Path & path) const
{
    int n = path.size();

	QL_REQUIRE(isInitialized_,
        "PathHedgeErrorCalculator: pricer not initialized");
    QL_REQUIRE(n>0,
        "PathHedgeErrorCalculator: the path cannot be empty");

    double log_price = 0.0;
	double stock = underlying_;
	BSMEuropeanOption option = BSMEuropeanOption(type_,
											stock,
											strike_,
											0.0,
											r_, maturity_, sigma_);

	double delta = option.delta();
	double money_account = option.value() - delta*stock;
        double dt = maturity_/n;

	for(int i = 0; i < n; i++){
        log_price += path[i];
		money_account *= QL_EXP( r_*dt );
		stock = underlying_*QL_EXP(log_price);
		if(i < n-1) {
			BSMEuropeanOption option = BSMEuropeanOption(type_, stock, strike_,
			    0.0, r_, maturity_ - dt*(i+1), sigma_);
			double new_delta = option.delta();
			money_account -= (new_delta - delta)*stock;
			delta = new_delta;
		}
	}

    return delta*stock + money_account - computePlainVanilla(type_, stock,
															 strike_, 1.0);
}

double PathHedgeErrorCalculator::computePlainVanilla(
									Option::Type type,
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
        BSMEuropeanOption option = BSMEuropeanOption(Option::Type::Call,
											s0_,
											strike_,
											0.0,
											r_, maturity_, sigma_);

        cout << "\n" << "Option value:\t " << option.value() << "\n";

        double theoretical_error_sd =
                QL_SQRT(3.1415926535/4/nTimeSteps)*option.vega()*sigma_;

        cout << "Value of the hedging error SD"
                " computed using Derman & Kamal's formula:\t "
                << theoretical_error_sd;

		double tau = maturity_ / nTimeSteps;
		double sigma = sigma_* sqrt(tau);
		double mean = r_ * tau - 0.5*sigma*sigma;
		Math::Statistics samples;

		samples = OneFactorMonteCarloOption(
					Handle<StandardPathGenerator>(
					    new StandardPathGenerator(nTimeSteps,
					                              mean,
					                              sigma*sigma)),
					Handle<PathPricer>(new PathHedgeErrorCalculator(
						Option::Type::Call,
						s0_, strike_, r_, maturity_, sigma_)),
					samples
				).sampleAccumulator(nSamples);

		cout << "\n";
		cout << "Mean hedging error: \t" << samples.mean() << "\n";
		cout << "Hedging error standard deviation: \t"
		    << samples.standardDeviation() << "\n";
	}

private:
	double	s0_;
	double	strike_;
	Time	maturity_;
	double	sigma_;
	Rate	r_;

};

int main(int argc, char* argv[])
{
	// 1-year at the money call, volatility 20%, risk-free rate 5%
	// underlying price 100.0, strike price 100.0
	ComputeHedgingError test(1./12., 100., 100., 0.2, 0.05);
	test.doTest();
	return 0;
}
