
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

/*! \file gaussianstatistics.hpp
    \brief statistics tool with gaussian risk measures
*/

// $Id$

#ifndef quantlib_gaussian_statistics_h
#define quantlib_gaussian_statistics_h

#include <ql/null.hpp>
#include <ql/dataformatters.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/Math/riskmeasures.hpp>

namespace QuantLib {

    namespace Math {

        //! GaussianStatistics tool with gaussian risk measures
        /*! It can accumulate a set of data and return gaussianstatistics quantities
            (e.g: mean, variance, skewness, kurtosis, error estimation,
            percentile, etc.) plus gaussian assumption risk measures
            (e.g.: value at risk, expected shortfall, etc.)

            \warning high moments are numerically unstable for high
                     average/standardDeviation ratios
        */
        class GaussianStatistics {
          public:
            GaussianStatistics();
            virtual ~GaussianStatistics() {};
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

            /*! gaussian-assumption y-th percentile, defined as the value x
                such that \f[ y = \frac{1}{\sqrt{2 \pi}}
                                      \int_{-\infty}^{x} \exp (-u^2/2) du \f]
            */
            double gaussianPercentile(double percentile) const;

            //! gaussian-assumption Potential-Upside at a given percentile
            double gaussianPotentialUpside(double percentile) const;

            //! gaussian-assumption Value-At-Risk at a given percentile
            double gaussianValueAtRisk(double percentile) const;

            //! gaussian-assumption Expected Shortfall at a given percentile
            double gaussianExpectedShortfall(double percentile) const;

            //! gaussian-assumption Shortfall (observations below target)
            double gaussianShortfall(double target) const;

            //! gaussian-assumption Average Shortfall (averaged shortfallness)
            double gaussianAverageShortfall(double target) const;
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

        inline Size GaussianStatistics::samples() const {
            return sampleNumber_;
        }

        inline double GaussianStatistics::weightSum() const {
            return sampleWeight_;
        }

        inline double GaussianStatistics::mean() const {
            QL_REQUIRE(sampleWeight_>0.0,
                       "GaussianStatistics::mean() : "
                       "sampleWeight_=0, unsufficient");
            return sum_/sampleWeight_;
        }

        inline double GaussianStatistics::standardDeviation() const {
            return QL_SQRT(variance());
        }

        inline double GaussianStatistics::downsideDeviation() const {
            return QL_SQRT(downsideVariance());
        }

        inline double GaussianStatistics::errorEstimate() const {
            double var = variance();
            QL_REQUIRE(samples() > 0,
                       "GaussianStatistics::errorEstimate : "
                       "zero samples are not sufficient");
            return QL_SQRT(var/samples());
        }

        inline double GaussianStatistics::min() const {
            QL_REQUIRE(sampleNumber_>0,
                       "GaussianStatistics::min_() : "
                       "empty sample");
            return min_;
        }

        inline double GaussianStatistics::max() const {
            QL_REQUIRE(sampleNumber_>0,
                       "GaussianStatistics::max_() : "
                       "empty sample");
            return max_;
        }

        // RiskMeasures proxies
        #define RISKMEASURE_PROXY_DOUBLE_RESULT_DOUBLE_ARG(METHOD) \
        inline double GaussianStatistics::METHOD(double y) const { \
            return rm_.METHOD(y, mean(), standardDeviation()); \
        }
        RISKMEASURE_PROXY_DOUBLE_RESULT_DOUBLE_ARG(gaussianPercentile)
        RISKMEASURE_PROXY_DOUBLE_RESULT_DOUBLE_ARG(gaussianPotentialUpside)
        RISKMEASURE_PROXY_DOUBLE_RESULT_DOUBLE_ARG(gaussianValueAtRisk)
        RISKMEASURE_PROXY_DOUBLE_RESULT_DOUBLE_ARG(gaussianExpectedShortfall)
        RISKMEASURE_PROXY_DOUBLE_RESULT_DOUBLE_ARG(gaussianShortfall)
        RISKMEASURE_PROXY_DOUBLE_RESULT_DOUBLE_ARG(gaussianAverageShortfall)
        #undef RISKMEASURE_PROXY_DOUBLE_RESULT_DOUBLE_ARG


    }

}


#endif
