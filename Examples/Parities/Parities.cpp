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

/*
    $Id$
*/

// $Source$
// $Log$
// Revision 1.9  2001/08/07 17:33:03  nando
// 1) StandardPathGenerator now is GaussianPathGenerator;
// 2) StandardMultiPathGenerator now is GaussianMultiPathGenerator;
// 3) PathMonteCarlo now is MonteCarloModel;
// 4) added ICGaussian, a Gaussian distribution that use
//    QuantLib::Math::InvCumulativeNormalDistribution to convert uniform
//    distribution extractions into gaussian distribution extractions;
// 5) added a few trailing underscore to private members
// 6) style enforced here and there ....
//
// Revision 1.8  2001/08/07 11:25:52  sigmud
// copyright header maintenance
//
// Revision 1.7  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.6  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.5  2001/07/16 22:24:51  dzuki
// Minor style fixes
//
// Revision 1.4  2001/07/16 21:57:11  dzuki
// Added option surplus integral test
//
// Revision 1.3  2001/07/16 16:12:02  nando
// style and typo fixed
//
// Revision 1.2  2001/07/15 08:34:53  nando
// feedback to Maxim's example
//
// Revision 1.1  2001/07/15 00:12:55  dzuki
// Added "Parities" example
//

#include "stdlib.h"
#include <iostream>

#include "ql\quantlib.hpp"

using namespace std;

using namespace QuantLib;
using namespace QuantLib::Pricers;
using namespace QuantLib::MonteCarlo;


class TestMethodsAndParity
{
public:
	TestMethodsAndParity(Time maturity,
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
		standardValue_ = europeanCallFormula();
	}

	void doTest()
	{
		cout<<"Computes European call option value\n\n";
		cout<<"Time to maturity = "<<maturity_<<"\t Underlying price = "<<s0_
			<<"\t Strike = "<< strike_<<"\nRisk-free interest rate = "<< r_
			<<"\t Volatility = "<<sigma_;
		cout<<"\n";

		printResult("Using call-put parity", europeanPutFormula() +
		    s0_ - strike_*QL_EXP(- r_*maturity_));
		printResult("Monte-Carlo method", europeanCallMC());
		printResult("FiniteDifference method", europeanCallFD());

		testOptionSurplusIntegral();
	}

	void printResult(string method,  double v)
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
		//  EuropeanOption(Type type, double underlying, double strike,
        //                Rate dividendYield, Rate riskFreeRate,
        //                Time residualTime, double volatility)

		return EuropeanOption(Option::Call, s0_, strike_, 0.0,
			r_, maturity_, sigma_).value();
	}

	double europeanPutFormula()
	{
		return EuropeanOption(Option::Put, s0_, strike_, 0.0,
			r_, maturity_, sigma_).value();
	}

    // MonteCarlo
	double europeanCallMC(int nTimeSteps = 100, int nSamples = 100000)
	{
		double tau = maturity_ / nTimeSteps;
		double sigma = sigma_* sqrt(tau);
		double mean = r_ * tau - 0.5*sigma*sigma;
		Math::Statistics samples;

		return OneFactorMonteCarloOption(
					Handle<GaussianPathGenerator>(
					    new GaussianPathGenerator(nTimeSteps,
					                              mean,
					                              sigma*sigma)),
					Handle<PathPricer>(new EuropeanPathPricer(
						Option::Type::Call,
						s0_, strike_, exp(-r_*maturity_))),
					samples
					).sampleAccumulator(nSamples).mean();


	}

    double europeanCallFD(int gridPoints = 100)  // Finite differences
	{
		return FiniteDifferenceEuropean(Option::Call, s0_, strike_, 0.0,
			r_, maturity_, sigma_, 100).value();
	}

	//  Option calculus analogy to energy conservation in heat diffusion equation
    //
    //  Function computes   
    //          Integral[ OptionValue[Log[underlyingPrice]] - 
    //                  - Max[underlyingPrice - Exp[- r*T] * strike, 0],
    //                  d(Log[underlyingPrice]), - inf, +inf]
    //
    //  should be equal for Eropean Call to
    //      1/2*sigma^2*T*Exp[-r*T]*strike
    //

    double optionSurplusIntegral(double t)
	{
		const int NY = 500;
		const double y0 = QL_LOG(s0_/2000);
		const double y1 = QL_LOG(s0_*40);
		const double dy = (y1-y0)/(NY-1);

		int i = 0;
		double sum = 0.0;
       
		double discount = QL_EXP(-r_*t);
		for(double y = y0; y <= y1; y += dy, ++i) {
			double s0 = QL_EXP(y);
			double ds = (EuropeanOption(Option::Call, s0, strike_, 0.0,
										r_, t, sigma_).value()
					 - QL_MAX(s0 - discount*strike_,0.0)
				   )*dy;
			sum += ds;
		}
        return sum;
	}
    
    void testOptionSurplusIntegral()
    {
        double theory = 1./2. * sigma_*sigma_ * strike_ * 
                        maturity_ * QL_EXP(- r_*maturity_);

        double integral = optionSurplusIntegral(maturity_);
        cout<<"\nOption surplus integral: \n";
        cout<<"Integral value: "<<integral<<"\t Theoretical value: "<<theory;
        double err = QL_FABS(integral - theory);
        cout<<"\t Error: "<<err;
        cout<<"\t Relative error: ";
        if(QL_FABS(theory)>1e-16) 
            cout<<err/theory;
        else
            cout<<"not computed";
        cout<<"\n";
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
