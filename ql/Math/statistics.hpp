
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file statistics.hpp
    \brief statistics tool with gaussian risk measures

    \fullpath
    ql/Math/%statistics.hpp
*/

// $Id$

#ifndef quantlib_statistics_h
#define quantlib_statistics_h

#include <ql/null.hpp>
#include <ql/dataformatters.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/Math/riskmeasures.hpp>

namespace QuantLib {

    namespace Math {

        //! Statistics tool with gaussian risk measures
        /*! It can accumulate a set of data and return statistics quantities
            (e.g: mean, variance, skewness, kurtosis, error estimation,
            percentile, etc.) plus gaussian assumption risk measures
            (e.g.: value at risk, expected shortfall, etc.)
        */
        class Statistics {
          public:
            Statistics();
            virtual ~Statistics() {};
            //! \name Inspectors
            //@{
            //! number of samples collected
            Size samples() const;

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

            /*! returns the downside variance, defined as
                \f[ \frac{N}{N-1} \times \frac{ \sum_{i=1}^{N}
                \theta \times x_i^{2}}{ \sum_{i=1}^{N} w_i} \f],
                where \f$ \theta \f$ = 0 if x > 0 and
                \f$ \theta \f$ =1 if x <0
            */
            double downsideVariance() const;

            /*! returns the downside deviation, defined as the
                square root of the downside variance.
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
                \f[ \frac{N^2(N+1)}{(N-1)(N-2)(N-3)}
                \frac{\left\langle \left(x-\langle x \rangle \right)^4
                \right\rangle}{\sigma^4} - \frac{3(N-1)^2}{(N-2)(N-3)}. \f]
                The above evaluates to 0 for a Gaussian distribution.
            */
            double kurtosis() const;

            /*! returns the minimum sample value */
            double min() const;

            /*! returns the maximum sample value */
            double max() const;

            /*! gaussian-assumption y percentile, defined as the value x such
                that \f[ y = \frac{1}{\sqrt{2 \pi}}
                                      \int_{-\infty}^{x} \exp (-u^2/2) du \f]
            */
            double gaussianPercentile(double y) const;

            //! gaussian-assumption Potential-Upside at a given percentile
            double gaussianPotentialUpside(double percentile) const {
                return rm_.potentialUpside(percentile,
                    mean(), standardDeviation());
            }

            //! gaussian-assumption Value-At-Risk at a given percentile
            double gaussianValueAtRisk(double percentile) const {
                return rm_.valueAtRisk(percentile,
                    mean(), standardDeviation());
            }

            //! gaussian-assumption Expected Shortfall at a given percentile
            double gaussianExpectedShortfall(double percentile) const {
                return rm_.expectedShortfall(percentile,
                    mean(), standardDeviation());
            }

            //! gaussian-assumption Shortfall (observations below target)
            double gaussianShortfall(double target) const {
                return rm_.shortfall(target,
                    mean(), standardDeviation());
            }

            //! gaussian-assumption Average Shortfall (averaged shortfallness)
            double gaussianAverageShortfall(double target) const  {
                return rm_.averageShortfall(target,
                    mean(), standardDeviation());
            }
            //@}

            //! \name Modifiers
            //@{
            //! adds a datum to the set, possibly with a weight
            void add(double value, double weight = 1.0);
            //! adds a sequence of data to the set, with default weight
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
          protected:
            Size sampleNumber_;
            double sampleWeight_;
            double sum_, quadraticSum_, downsideQuadraticSum_,
                   cubicSum_, fourthPowerSum_;
            double min_, max_;
            RiskMeasures rm_;
        };

        // inline definitions

        inline Size Statistics::samples() const {
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

        inline double Statistics::standardDeviation() const {
            return QL_SQRT(variance());
        }

        inline double Statistics::downsideDeviation() const {
            return QL_SQRT(downsideVariance());
        }

        inline double Statistics::errorEstimate() const {
            double var = variance();
            QL_REQUIRE(samples() > 0,
                       "Statistics: zero samples are not sufficient");
            return QL_SQRT(var/samples());
        }

        inline double Statistics::min() const {
            QL_REQUIRE(sampleNumber_>0, "Stat::min_() : empty sample");
            return min_;
        }

        inline double Statistics::max() const {
            QL_REQUIRE(sampleNumber_>0, "Stat::max_() : empty sample");
            return max_;
        }

        inline double Statistics::gaussianPercentile(double y) const {

            QL_REQUIRE(y<1.0 && y>0.0,
                "Statistics::percentile : percentile (" +
                DoubleFormatter::toString(y) +
                ") must be in (0%, 100%) extremes excluded");

            Math::InverseCumulativeNormal gInverse(mean(),
                standardDeviation());
            return gInverse(y);
        }
    }

}


#endif
