/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file MontecarloPricers.i
	
	$Source$
	$Name$
	$Log$
	Revision 1.1  2001/01/04 17:31:23  marmar
	Alpha version of the Monte Carlo tools.

*/

#ifndef shaft_Montecarlo_i
#define shaft_Montecarlo_i

%module MontecarloPricers

%{
#include "quantlib.h"
%}

%include Vectors.i

#if defined(SWIGPYTHON)

%{
#include "standardpathgenerator.h"
using QuantLib::MonteCarlo::StandardPathGenerator;
%}

class StandardPathGenerator{
    public:
	StandardPathGenerator(int dimension, long seed=0);
	Array next() const; // Note that currently Path and Array are equivalent
	double weight() const;
};

%{
#include "mceuropeanpricer.h"
using QuantLib::Pricers::McEuropeanPricer;
%}

class McEuropeanPricer{
    public:
	McEuropeanPricer(OptionType type, double underlying, double strike, 
		 Rate underlyingGrowthRate,   Rate riskFreeRate, double residualTime, 
		 double volatility,	int timesteps, int confnumber, long seed);
	double value() const;
	double errorEstimate() const;
};

%{
#include "geometricasianoption.h"
using QuantLib::Pricers::GeometricAsianOption;
%}

class GeometricAsianOption {
  public:
	GeometricAsianOption(OptionType type, double underlying, double strike, 
		Rate underlyingGrowthRate, Rate exerciseRate,
		double residualTime, double volatility);
	double value() const;
};

%{
#include "mcasianpricer.h"
using QuantLib::Pricers::McAsianPricer;
%}

class McAsianPricer{
    public:
	McAsianPricer(OptionType type, double underlying, double strike, 
		 Rate underlyingGrowthRate,   Rate riskFreeRate, double residualTime, 
		 double volatility,	int timesteps, int confnumber, long seed);
	double value() const;
	double errorEstimate() const;
};

#elif defined(SWIGJAVA)
// export relevant functions
#endif

#endif
