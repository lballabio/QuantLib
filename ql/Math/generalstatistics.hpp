
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file generalstatistics.hpp
    \brief statistics tool with empirical-distribution risk measures
*/

// $Id$

#ifndef quantlib_general_statistics_h
#define quantlib_general_statistics_h

#include <ql/null.hpp>
#include <ql/dataformatters.hpp>
#include <vector>

namespace QuantLib {

    namespace Math {
        //! Statistics tool with empirical-distribution risk measures
        /*! It can accumulate a set of data and return statistics (e.g: mean,
            variance, skewness, kurtosis, error estimation, percentile, etc.)
            plus risk measures (e.g.: value-at-risk, expected shortfall, etc.)
            based on the empirical distribution (no gaussian assumption)

            It extends the class IncrementalStatistics with
            empirical-distribution risk measures, and it doesn't suffer the
            numerical instability problem of IncrementalStatistics. The
            penalty is that it stores all samples.
        */
        class GeneralStatistics {
          public:
            GeneralStatistics() { reset(); }
            //! \name Inspectors
            //@{
            //! number of samples collected
            Size samples() const { return samples_.size(); }

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
            double standardDeviation() const {
                return QL_SQRT(variance());
            }

            /*! returns the variance of observations below mean 
                \f[ \frac{\sum w_i (min(0, x_i-\langle x \rangle))^2 }{\sum w_i}. \f]

                Markowitz (1959)
            */
            double semiVariance() const {
                return regret(mean());
            }

            /*! returns the semi deviation, defined as the
                square root of the semi variance.
            */
            double semiDeviation() const {
                return QL_SQRT(semiVariance());
            }

            /*! returns the downside variance, defined as
                \f[ \frac{N}{N-1} \times \frac{ \sum_{i=1}^{N}
                \theta \times x_i^{2}}{ \sum_{i=1}^{N} w_i} \f],
                where \f$ \theta \f$ = 0 if x > 0 and
                \f$ \theta \f$ =1 if x <0
            */
            double downsideVariance() const {
                return regret(0.0);
            }

            /*! returns the downside deviation, defined as the
                square root of the downside variance.
            */
            double downsideDeviation() const {
                return QL_SQRT(downsideVariance());
            }

            /*! returns the variance of observations below target 
                \f[ \frac{\sum w_i (min(0, x_i-target))^2 }{\sum w_i}. \f]

                See Dembo, Freeman "The Rules Of Risk", Wiley (2001)
            */
            double regret(double target) const;


            /*! returns the error estimate \f$ \epsilon \f$, defined as the
                square root of the ratio of the variance to the number of
                samples.
            */
            double errorEstimate() const {
                return QL_SQRT(variance()/samples());
            }

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
            double min() const {
                return std::min_element(samples_.begin(), samples_.end())->first;
            }

            /*! returns the maximum sample value */
            double max() const {
                return std::max_element(samples_.begin(), samples_.end())->first;
            }

            /*! y-th percentile, defined as the value x
                such that \f[ y = \frac{1}{\sqrt{2 \pi}}
                                      \int_{-\infty}^{x} \exp (-u^2/2) du \f]
            */
            double percentile(double y) const;

            //! potential upside (the reciprocal of VAR) at a given percentile
            double potentialUpside(double percentile) const;

            //! Value-At-Risk at a given percentile
            double valueAtRisk(double percentile) const;

            //! Expected Shortfall at a given percentile
            /*! returns the expected loss given that the loss has exceeded
                a VaR threshold:

                \f[ y = E(x | x>VaR(percentile) ) \f]
                
                that is the average of observations below the given percentile.
                Also know as conditional Value-at-Risk.

                Artzner, Delbaen, Eber, Heath (1999)
                "Coherent measures of risk", Mathematical Finance 9
            */
            double expectedShortfall(double percentile) const;

            //! Shortfall risk measure (observations below target)
            double shortfall(double target) const;

            //! Average Shortfall (averaged shortfallness)
            double averageShortfall(double target) const;

            //! access to the sample data accumulated so far
            const std::vector<std::pair<double,double> >& sampleData() const {
                return samples_; }
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
          private:
            mutable std::vector<std::pair<double,double> > samples_;
        };

        /*! \pre weights must be positive or null */
        inline void GeneralStatistics::add(double value, double weight) {
          QL_REQUIRE(weight>=0.0,
              "GeneralStatistics::add : negative weight not allowed");
          samples_.push_back(std::make_pair(value,weight));
        }

        inline void GeneralStatistics::reset() {
            samples_ = std::vector<std::pair<double,double> >();
        }

    }

}


#endif
