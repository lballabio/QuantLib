
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

/*! \file normaldistribution.h
	\brief normal and cumulative normal distributions
	
	$Source$
	$Name$
	$Log$
	Revision 1.4  2000/12/27 17:18:35  lballabio
	Changes for compiling under Linux and Alpha Linux

	Revision 1.3  2000/12/14 12:32:30  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#ifndef quantlib_normal_distribution_h
#define quantlib_normal_distribution_h

#include "qldefines.h"
#include <functional>

namespace QuantLib {

	namespace Math {

		class NormalDistribution : public std::unary_function<double,double> {
		  public:
			NormalDistribution(double average = 0.0, double sigma = 1.0);
			// function
			double operator()(double x);
		  private:
			static const double pi;
			double average, sigma, normalizationFactor, denominator;
		};
		
		typedef NormalDistribution GaussianDistribution;
		
		class CumulativeNormalDistribution : public std::unary_function<double,double> {
		  public:
			CumulativeNormalDistribution();
			// function
			double operator()(double x);
			double derivative(double x);
		  private:
			static const double a1, a2, a3, a4, a5, gamma, precision;
			NormalDistribution gaussian;
		};
		
		// inline definitions
		
		inline NormalDistribution::NormalDistribution(double average, double sigma)
		: average(average), sigma(sigma) {
			normalizationFactor = 1.0/(sigma*QL_SQRT(2.0*pi));
			denominator = 2.0*sigma*sigma;
		}
		
		inline double NormalDistribution::operator()(double x) {
			double deltax = x-average;
			return normalizationFactor*QL_EXP(-deltax*deltax/denominator);
		}
		
		inline CumulativeNormalDistribution::CumulativeNormalDistribution() {}
		
		inline double CumulativeNormalDistribution::derivative(double x) {
			return gaussian(x);
		}

	}
	
}


#endif
