
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

/*! \file statistics.hpp
    \brief statistic tools

    $Id$
*/

// $Source$
// $Log$
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_statistic_h
#define quantlib_statistic_h

#include "ql/null.hpp"
#include "ql/qlerrors.hpp"
#include "ql/dataformatters.hpp"
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
            //! number of samples collected
            double samples() const;
            //! sum of data weights
            double weightSum() const;
            /*! returns the mean, defined as
                \f[ \langle x \rangle = \frac{\sum w_i x_i}{\sum w_i}. \f]
            */
            double mean() const;
            /*! returns the variance, defined as
                \f[ \frac{N}{N-1} \left\langle \left(
                x-\langle x \rangle \right)^2 \right\rangle. \f]
            */
            double variance() const;
            /*! returns the standard deviation \f$ \sigma \f$, defined as the
                square root of the variance.
            */
            double standardDeviation() const;
            /*! returns the square root of the downside variance, defined as
                \f[ \frac{N}{N-1} \times \frac{ \sum_{i=1}^{N}
                \theta \times x_i^{2}}{ \sum_{i=1}^{N} w_i} \f],
                where \f$ \theta \f$ = 0 if x > 0 and \f$ \theta \f$ =1 if x <0
            */
            double downsideDeviation() const;
            /*! returns the error estimate \f$ \epsilon \f$, defined as the
                square root of the ratio of the variance to the number of
                samples.
            */
            double errorEstimate() const;
            /*! returns the skewness, defined as
                \f[ \frac{N^2}{(N-1)(N-2)} \frac{\left\langle \left(
                x-\langle x \rangle \right)^3 \right\rangle}{\sigma^3}. \f]
                The above evaluates to 0 for a Gaussian distribution.
            */
            double skewness() const;
            /*! returns the excess kurtosis, defined as
                \f[ \frac{N(N+1)}{(N-1)(N-2)(N-3)}
                \frac{\left\langle \left( x-\langle x \rangle \right)^4
                \right\rangle}{\sigma^4} - \frac{3(N-1)^2}{(N-2)(N-3)}. \f]
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
              for (;begin!=end;++begin)
                add(*begin);
            }
            //! adds a sequence of data to the set, each with its weight
            template <class DataIterator, class WeightIterator>
            void addSequence(DataIterator begin, DataIterator end,
              WeightIterator wbegin) {
                for (;begin!=end;++begin,++wbegin)
                    add(*begin, *wbegin);
            }
            //! resets the data to a null set
            void reset();
            //@}
          private:
            double sampleNumber_;
            double sampleWeight_;
            double sum_, quadraticSum_, downsideQuadraticSum_,
                   cubicSum_, fourthPowerSum_;
            double min_, max_;
        };

        // inline definitions

        /*! \pre weights must be positive or null */
        inline void Statistics::add(double value, double weight) {
            QL_REQUIRE(weight>=0.0,
                "Statistics::add : negative weight (" +
                DoubleFormatter::toString(weight) + ") not allowed");
          sampleNumber_ += 1;
          sampleWeight_ += weight;
          double temp = weight*value;
          sum_ += temp;
          temp *= value;
          quadraticSum_ += temp;
          downsideQuadraticSum_ += value < 0.0 ? temp : 0.0;
          temp *= value;
          cubicSum_ += temp;
          temp *= value;
          fourthPowerSum_ += temp;
          min_=QL_MIN(value, min_);
          max_=QL_MAX(value, max_);
        }

        inline double Statistics::samples() const {
          return sampleNumber_;
        }

        inline double Statistics::weightSum() const {
          return sampleWeight_;
        }

        inline double Statistics::mean() const {
          QL_REQUIRE(sampleWeight_>0.0,
            "Stat::mean() : sampleWeight_=0, unsufficient");
          return sum_/sampleWeight_;
        }

        inline double Statistics::variance() const {
          QL_REQUIRE(sampleWeight_>0.0,
            "Stat::variance() : sampleWeight_=0, unsufficient");
          QL_REQUIRE(sampleNumber_>1,
            "Stat::variance() : sample number <=1, unsufficient");

          return (sampleNumber_/(sampleNumber_-1.0))*
            (quadraticSum_ - sum_*sum_/sampleWeight_)/sampleWeight_;
        }

        inline double Statistics::standardDeviation() const {
          return QL_SQRT(variance());
        }

        inline double Statistics::downsideDeviation() const {
          QL_REQUIRE(sampleWeight_>0.0,
            "Stat::variance() : sampleWeight_=0, unsufficient");
          QL_REQUIRE(sampleNumber_>1,
            "Stat::variance() : sample number <=1, unsufficient");

          return QL_SQRT(sampleNumber_/(sampleNumber_-1.0)*
                                 downsideQuadraticSum_ /sampleWeight_);
        }

        inline double Statistics::errorEstimate() const {
            double var = variance();
            QL_REQUIRE(samples() != 0.0,
                       "Statistics: samples are not sufficient");
            if(QL_FABS(var) < 1e-12) var =0.0;
            QL_REQUIRE(var >= 0.0,
                       "Statistics: variance, " +
                       DoubleFormatter::toString(var,20)
                       +" is NEGATIVE");
          return QL_SQRT(var/samples());
        }

        inline double Statistics::skewness() const {
          QL_REQUIRE(sampleNumber_>2,
            "Stat::skewness() : sample number <=2, unsufficient");
          double s = standardDeviation();
          QL_ENSURE(s>0.0,
            "Stat::skewness() : standard_dev=0.0, skew undefined");
          double m = mean();

          return sampleNumber_*sampleNumber_/
            ((sampleNumber_-1.0)*(sampleNumber_-2.0)*s*s*s)*
            (cubicSum_-3.0*m*quadraticSum_+2.0*m*m*sum_)/sampleWeight_;
        }

        inline double Statistics::kurtosis() const {
          QL_REQUIRE(sampleNumber_>3,
            "Stat::kurtosis() : sample number <=3, unsufficient");
          double m = mean();
          double v = variance();
          QL_ENSURE(v>0.0,
            "Stat::kurtosis() : variance=0.0, kurtosis undefined");

          return sampleNumber_*sampleNumber_*(sampleNumber_+1.0) /
            ((sampleNumber_-1.0)*(sampleNumber_-2.0) *
            (sampleNumber_-3.0)*v*v) *
            (fourthPowerSum_ - 4.0*m*cubicSum_ + 6.0*m*m*quadraticSum_ -
            3.0*m*m*m*sum_)/sampleWeight_ -
            3.0*(sampleNumber_-1.0)*(sampleNumber_-1.0) /
            ((sampleNumber_-2.0)*(sampleNumber_-3.0));
        }

        inline double Statistics::min() const {
          QL_REQUIRE(sampleNumber_>=0, "Stat::min_() : empty sample");
          return min_;
        }

        inline double Statistics::max() const {
          QL_REQUIRE(sampleNumber_>=0, "Stat::max_() : empty sample");
          return max_;
        }

    }

}


#endif
