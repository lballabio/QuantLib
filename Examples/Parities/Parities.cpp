/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*
    $Id$
*/

// $Source$
// $Log$
// Revision 1.1  2001/07/15 00:12:55  dzuki
// Added "Parities" example
//

#include <iostream>
#include "stdlib.h"
#include "ql\quantlib.hpp"

using namespace QuantLib;
using namespace QuantLib::Pricers;
using namespace QuantLib::MonteCarlo;
using namespace std;


class TestMethodsAndParity
{
public:
	TestMethodsAndParity(Time maturity, double strike, double s0,
			double sigma, Rate r)
	{
		maturity_ = maturity;
		strike_ = strike;
		s0_ = s0;
		sigma_ = sigma;
		r_ = r;
		standardValue_ = europeanCallFormula();
	}

	void doTest()
	{
		cout<<"Computes European call option value\n\n";
		cout<<"Time to maturity = "<<maturity_<<"\t Underlying price = "<<s0_
			<<"\t Strike = "<< strike_<<"\nRisk-free interest rate = "<< r_
			<<"\t Volatility = "<<sigma_;
		cout<<"\n";

		printResult("Using call-put parity", europeanPutFormula() + s0_ - strike_*QL_EXP(- r_*maturity_));
		printResult("Monte-Carlo method", europeanCallMC());
		printResult("FiniteDifference method", eropeanCallFD());
	}

	void printResult(std::string method,  double v)
	{
		double err = QL_FABS(standardValue_ - v);
		cout<<"\n"<< method <<"\n";
		cout<<" Value = "<< v << "\tError = "<< err;
		cout<<" \tRelative error:";
		if( QL_FABS(standardValue_) > 1e-16 )
			cout<<err/standardValue_;
		else
			cout<<" not computed";
		cout<<"\n";
	}

protected:
	double europeanCallFormula()
	{
		//  BSMEuropeanOption(Type type, double underlying, double strike,
        //                Rate dividendYield, Rate riskFreeRate,
        //                Time residualTime, double volatility)
            
		return BSMEuropeanOption(Option::Call, s0_, strike_, 0.0, 
			r_, maturity_, sigma_).value(); 
	}

	double europeanPutFormula()
	{
		return BSMEuropeanOption(Option::Put, s0_, strike_, 0.0, 
			r_, maturity_, sigma_).value(); 
	}

	double europeanCallMC(int nTimeSteps = 100, int nSamples = 100000) // MonteCarlo
	{
			double tau = maturity_ / nTimeSteps;
			double sigma = sigma_* sqrt(tau);
			double mean = r_ * tau - 0.5*sigma*sigma;
			Math::Statistics samples;

		return OneFactorMonteCarloOption(
					Handle<StandardPathGenerator>( new StandardPathGenerator(nTimeSteps, mean, sigma*sigma)),
					Handle<PathPricer>(new EuropeanPathPricer(
						Option::Type::Call,
						s0_, strike_, exp(-r_*maturity_))),
					samples
					).sampleAccumulator(nSamples).mean();


	}
	double eropeanCallFD(int gridPoints = 100)  // Finite differences
	{
		return FiniteDifferenceEuropean(Option::Call, s0_, strike_, 0.0, 
			r_, maturity_, sigma_, 100).value(); 
	}

private:
	double standardValue_;

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
	TestMethodsAndParity test(1., 100., 100., 0.2, 0.05);
	test.doTest();
	return 0;
}
