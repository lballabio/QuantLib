
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

/*! \file statistics.h
	\brief statistic tools
*/

#ifndef quantlib_statistic_h
#define quantlib_statistic_h

#include "qldefines.h"
#include "null.h"
#include "qlerrors.h"
#include <iostream>

namespace QuantLib {

	namespace Math {

		//! Statistic tool
		/*! It can accumulate a set of data and return statistic quantities
			as mean, variance, std. deviation, skewness, and kurtosis.
		*/
		class Statistics {
		  public:
		    Statistics();
		    //! \name Inspectors
		    //@{
		    //! size of data set
		    double samples() const;
		    //! sum of data weights
		    double weightSum() const;
		    /*! returns the mean, defined as
		    \f[ \langle x \rangle = \frac{\sum w_i x_i}{\sum w_i}. \f]
		    */
		    double mean() const;
		    /*! returns the variance, defined as
		    \f[ \frac{N}{N-1} \left\langle \left( x-\langle x \rangle \right)^2 \right\rangle. \f]
		    */
		    double variance() const;
		    /*! returns the standard deviation \f$ \sigma \f$, defined as the square root of the variance. */
		    double standardDeviation() const;
		    /*! returns the skewness, defined as
		    \f[ \frac{N^2}{(N-1)(N-2)}
		    \frac{\left\langle \left( x-\langle x \rangle \right)^3 \right\rangle}{\sigma^3}. \f]
		    The above evaluates to 0 for a Gaussian distribution.
		    */
		    double skewness() const;
		    /*! returns the excess kurtosis, defined as
		    \f[ \frac{N(N+1)}{(N-1)(N-2)(N-3)}
		    \frac{\left\langle \left( x-\langle x \rangle \right)^4 \right\rangle}{\sigma^4}
		    -\frac{3(N-1)^2}{(N-2)(N-3)}. \f]
		    The above evaluates to 0 for a Gaussian distribution.
		    */
		    double kurtosis() const;
		    /*! returns the minimum sample value */
		    double min() const;
		    /*! returns the maximum sample value */
		    double max() const;
			//@}
			
		    //! \name Modifiers
		    //@{
		    //! adds a datum to the set, possibly with a weight
		    void add(double value, double weight = 1.0);
		    //! adds a sequence of data to the set
		    template <class DataIterator>
		    void addSequence(DataIterator begin, DataIterator end) {
		      for (;begin!=end;begin++)
		        add(*begin);
		    }
		    //! adds a sequence of data to the set, each with its weight
		    template <class DataIterator, class WeightIterator>
		    void addSequence(DataIterator begin, DataIterator end, WeightIterator wbegin) {
		      for (;begin!=end;begin++,wbegin++)
		        add(*begin, *wbegin);
		    }
		    //! resets the data to a null set
		    void reset();
		    //@}
		  private:
		    double theSampleNumber;
		    double theSampleWeight;
		    double theSum, theQuadraticSum, theCubicSum, theFourthPowerSum;
		    double theMin, theMax;
		};
		
		// inline
		inline void Statistics::add(double value, double weight) {
		  Require(weight>=0.0, "Statistics::add : negative weight not allowed");
		  theSampleNumber += 1;
		  theSampleWeight += weight;
		  double token, temp;
		  token= temp = weight*value;
		  theSum += temp;
		  temp *= token;
		  theQuadraticSum += temp;
		  temp *= token;
		  theCubicSum += temp;
		  temp *= token;
		  theFourthPowerSum += temp;
		  theMin=QL_MIN(value, theMin);
		  theMax=QL_MAX(value, theMax);
		}
		
		inline double Statistics::samples() const {
		  return theSampleNumber;
		}
		
		inline double Statistics::weightSum() const {
		  return theSampleWeight;
		}
		
		inline double Statistics::mean() const {
		  Require(theSampleWeight>0.0, "Stat::mean() : theSampleWeight=0, unsufficient");
		
		  return theSum/theSampleWeight;
		}
		
		inline double Statistics::variance() const {
		  Require(theSampleWeight>0.0, "Stat::variance() : theSampleWeight=0, unsufficient");
		  Require(theSampleNumber>1, "Stat::variance() : sample number <=1, unsufficient");
		  double m = mean();
		
		  return (theSampleNumber/(theSampleNumber-1.0))*
		    (theQuadraticSum - theSum*theSum/theSampleWeight)/theSampleWeight;
		}
		
		inline double Statistics::standardDeviation() const {
		  return QL_SQRT(variance());
		}
		
		inline double Statistics::skewness() const {
		  Require(theSampleNumber>2, "Stat::skewness() : sample number <=2, unsufficient");
		  double s = standardDeviation();
		  Require(s>0.0, "Stat::skewness() : standard_dev=0.0, skew undefined");
		  double m = mean();
		
		  return theSampleNumber*theSampleNumber/
		    ((theSampleNumber-1.0)*(theSampleNumber-2.0)*s*s*s)*
		    (theCubicSum-3.0*m*theQuadraticSum+2.0*m*m*theSum)/theSampleWeight;
		}
		
		inline double Statistics::kurtosis() const {
		  Require(theSampleNumber>3, "Stat::kurtosis() : sample number <=3, unsufficient");
		  double m = mean();
		  double v = variance();
		  Require(v>0.0, "Stat::kurtosis() : variance=0.0, kurtosis undefined");
		
		  return theSampleNumber*theSampleNumber*(theSampleNumber+1.0)/
		    ((theSampleNumber-1.0)*(theSampleNumber-2.0)*(theSampleNumber-3.0)*v*v)*
		    (theFourthPowerSum-4.0*m*theCubicSum+6.0*m*m*theQuadraticSum-3.0*m*m*m*theSum)/theSampleWeight-
		    3.0*(theSampleNumber-1.0)*(theSampleNumber-1.0)/
		    ((theSampleNumber-2.0)*(theSampleNumber-3.0));
		}
		
		inline double Statistics::min() const {
		  Require(theSampleNumber>=0, "Stat::min() : empty sample");
		  return theMin;
		}
		
		inline double Statistics::max() const {
		  Require(theSampleNumber>=0, "Stat::max() : empty sample");
		  return theMax;
		}

	}

}


#endif
