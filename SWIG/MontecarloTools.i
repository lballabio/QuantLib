/*
 * Copyright (C) 2001
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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file MontecarloTools.i
	
	$Source$
	$Name$
	$Log$
	Revision 1.2  2001/02/13 10:08:58  marmar
	Changed interface to StandardMultiPathGenerator

	Revision 1.1  2001/02/02 10:57:34  marmar
	Basic swig interface for Monte Carlo tools
	
*/

#ifndef shaft_Montecarlo_Tools_i
#define shaft_Montecarlo_Tools_i

%module MontecarloTools

%{
#include "quantlib.h"
%}

%include Vectors.i

#if defined(SWIGPYTHON)
%{
#include "gaussianarraygenerator.h"
using QuantLib::MonteCarlo::GaussianArrayGenerator;
%}

class GaussianArrayGenerator{
    public:
	GaussianArrayGenerator(const PyArray &average,
                           const Matrix &covariance, 
                           long seed=0);
	PyArray next() const;
	double weight() const;
};

%{
#include "standardmultipathgenerator.h"
using QuantLib::MonteCarlo::StandardMultiPathGenerator;
%}

class StandardMultiPathGenerator{
    public:
	StandardMultiPathGenerator(const DoubleVector &timeDelays, 
	                           const PyArray &average, 
                               const Matrix &covariance,
                               long seed=0);
	Matrix next() const;
	double weight() const;
};

#elif defined(SWIGJAVA)
// export relevant functions
#endif

#endif
