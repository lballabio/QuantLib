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
// $Log$
// Revision 1.11  2001/08/08 16:02:33  nando
// refactoring .... not finished yet
//

#include "stdlib.h"
#include <iostream>

#include "ql\quantlib.hpp"

using namespace std;

// Rate and Time are just double, but having their own types allows for
// a stonger check at compile time
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

// the path generator
using QuantLib::MonteCarlo::GaussianPathGenerator;

// the Montecarlo pricing model for option on a single asset
using QuantLib::MonteCarlo::OneFactorMonteCarloOption;

using QuantLib::MonteCarlo::EuropeanPathPricer;
using QuantLib::Pricers::EuropeanOption;
using QuantLib::Pricers::FiniteDifferenceEuropean;

// to format the output of doubles
using QuantLib::DoubleFormatter;


// ????
// I need to understand the following
// ????
double optionSurplusIntegral(Time maturity_,
        	                 double strike_,
	                         double s0_,
	                         double sigma_,
	                         Rate r_)
{
	const int NY = 500;
	const double y0 = QL_LOG(s0_/2000);
	const double y1 = QL_LOG(s0_*40);
	const double dy = (y1-y0)/(NY-1);

	int i = 0;
	double sum = 0.0;

	double discount = QL_EXP(-r_*maturity_);
	for(double y = y0; y <= y1; y += dy, ++i) {
		double s0 = QL_EXP(y);
		double ds = (EuropeanOption(Option::Call, s0, strike_, 0.0,
									r_, maturity_, sigma_).value()
				 - QL_MAX(s0 - discount*strike_,0.0)
			   )*dy;
		sum += ds;
	}
    return sum;
}



int main(int argc, char* argv[])
{

    // our option
    double underlying = 100;
    double strike = 100;      // at the money
    Rate dividendYield = 0.0; // no dividends
    Rate riskFreeRate = 0.05; // 5%
    Time maturity = 1.0;      // 1 year
    double volatility = 0.20; // 20%
    cout << "Time to maturity = "        << maturity     << endl;
    cout << "Underlying price = "        << underlying   << endl;
	cout << "Strike = "                  << strike       << endl;
    cout << "Risk-free interest rate = " << riskFreeRate << endl;
	cout << "Volatility = "              << volatility   << endl;
    cout << endl;

    // write column headings
    cout << "Method\t\tValue\tEstimatedError\tDiscrepancy"
        "\tRel. Discr." << endl;


    
    // first method: Black Scholes analytic solution    
    string method ="Black Scholes";
    double value = EuropeanOption(Option::Call, underlying, strike,
        dividendYield, riskFreeRate, maturity, volatility).value();
    double estimatedError = 0.0;
    double discrepancy = 0.0;
    double relativeDiscrepancy = 0.0;
    cout << method << "\t" 
         << DoubleFormatter::toString(value, 4) << "\t"
         << DoubleFormatter::toString(estimatedError, 4) << "\t\t"
         << DoubleFormatter::toString(discrepancy, 6) << "\t"
         << DoubleFormatter::toString(relativeDiscrepancy, 6) << endl;


    // store the Black Scholes value as the correct one    
    double rightValue = value;





    // second method: Call-Put parity    
    method ="Call-Put parity";
    value = EuropeanOption(Option::Put, underlying, strike,
        dividendYield, riskFreeRate, maturity, volatility).value()
        + underlying - strike*QL_EXP(- riskFreeRate*maturity);
    estimatedError = 0.0;
    discrepancy = value-rightValue;
    relativeDiscrepancy = discrepancy/rightValue;
    cout << method << "\t" 
         << DoubleFormatter::toString(value, 4) << "\t"
         << "N/A\t\t"
         << discrepancy << "\t"
         << DoubleFormatter::toString(relativeDiscrepancy, 6) << endl;





    // third method: Finite Differences    
    method ="Finite Diff.";
    int grid = 100;
    value = FiniteDifferenceEuropean(Option::Call, underlying, strike,
        dividendYield, riskFreeRate, maturity, volatility, grid).value();
    estimatedError = 0.0;
    discrepancy = value-rightValue;
    relativeDiscrepancy = discrepancy/rightValue;
    cout << method << "\t" 
         << DoubleFormatter::toString(value, 4) << "\t"
         << "N/A\t\t"
         << DoubleFormatter::toString(discrepancy, 6) << "\t"
         << DoubleFormatter::toString(relativeDiscrepancy, 6) << endl;





    // third method: MonteCarlo  
    method ="MonteCarlo";
    // for plain vanilla european option the number of steps is not significant
    // let's go for the fastest way: just one step
    int nTimeSteps = 1;
    int nSamples = 100000;
	double tau = maturity/nTimeSteps;
	double sigma = volatility* sqrt(tau);
	double mean = riskFreeRate * tau - 0.5*sigma*sigma;
	Statistics samples;
    Handle<GaussianPathGenerator> myPathGenerator(
        new GaussianPathGenerator(nTimeSteps, mean, sigma*sigma));
    // The European path pricer
    Handle<PathPricer> myEuropeanPathPricer =
        Handle<PathPricer>(new EuropeanPathPricer(Option::Type::Call,
        underlying, strike, exp(-riskFreeRate*maturity)));
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
    discrepancy = value-rightValue;
    relativeDiscrepancy = discrepancy/rightValue;
    cout << method << "\t" 
         << DoubleFormatter::toString(value, 4) << "\t"
         << DoubleFormatter::toString(estimatedError, 4) << "\t\t"
         << DoubleFormatter::toString(discrepancy, 6) << "\t"
         << DoubleFormatter::toString(relativeDiscrepancy, 6) << endl;




    // ????
    // I need to understand the following
    // ????
    //
    //  Option calculus analogy to energy conservation in heat diffusion
	//  equation
    //  Function computes
    //          Integral[ OptionValue[Log[underlyingPrice]] -
    //                  - Max[underlyingPrice - Exp[- r*T] * strike, 0],
    //                  d(Log[underlyingPrice]), - inf, +inf]
    //
    //  should be equal for Eropean Call to
    //      1/2*sigma^2*T*Exp[-r*T]*strike
    //
    double theory = 1./2. * volatility*volatility * strike *
                    maturity * QL_EXP(- riskFreeRate*maturity);

    double integral = optionSurplusIntegral(maturity, strike, underlying,
        volatility, riskFreeRate);
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


	return 0;
}
