
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

/*! \file sequencestatistics.hpp
    \brief Statistic tools for sequence (vector, list, array) samples

    \fullpath
    ql/Math/%sequencestatistics.hpp
*/

// $Id$

#ifndef quantlib_sequence_statistics_hpp
#define quantlib_sequence_statistics_hpp

#include <ql/qldefines.hpp>
#include <algorithm>
#include <iterator>
#include <vector>

namespace QuantLib {

    namespace Math {

        //! Statistic analysis of N-dimensional (sequence) data
        /*! It provides 1-dimensional statistic quantities as discrepancy plus
            N-dimensional (sequence) statistic quantities (e.g. mean,
            variance, skewness, kurtosis, etc.) with one component for each
            dimension of the sample space.

            For most of the statistic quantities this class relies on
            the StatisticsType underlying class to provide 1-D methods that
            will be iterated for all the components of the N-D data. These
            "inherited" methods are the union of all the methods that might be
            requested to the 1-D underlying StatisticsType class, with the
            usual compile-time checks provided by the template approach.
        */
        template <class StatisticsType>
        class SequenceStatistics {
          public:
            // typedefs
            typedef StatisticsType statistics_type;
            // constructor
            SequenceStatistics(Size dimension);
            //! \name 1-dimensional inspectors
            //@{
            double discrepancy() const;
            //@}
            //! \name 1-D inspectors "inherited" by underlying statistic class
            //@{
            Size samples() const;
            double weightSum() const;
            //@}
            //! \name N-D inspectors "inherited" by underlying statistic class
            //@{
            // void argument list
            std::vector<double> mean() const;
            std::vector<double> variance() const;
            std::vector<double> standardDeviation() const;
            std::vector<double> downsideVariance() const;
            std::vector<double> downsideDeviation() const;
            std::vector<double> errorEstimate() const;
            std::vector<double> skewness() const;
            std::vector<double> kurtosis() const;
            std::vector<double> min() const;
            std::vector<double> max() const;

            // single double argument list
            std::vector<double> percentile(double y) const;
            std::vector<double> potentialUpside(double percentile) const;
            std::vector<double> valueAtRisk(double percentile) const;
            std::vector<double> expectedShortfall(double percentile) const;
            std::vector<double> shortfall(double target) const;
            std::vector<double> averageShortfall(double target) const;
            //@}
            //! \name Modifiers
            //@{
            template <class Sequence>
            void add(const Sequence& sample,
                     double weight = 1.0) {
                QL_REQUIRE(sample.size() == dimension_,
                           "SequenceStatistics::add : "
                           "sample size mismatch");
                typename Sequence::const_iterator it;
                Size i;
                for (it=sample.begin(), i=0; it!=sample.end(); ++it, ++i)
                    stats_[i].add(*it, weight);
            }

            template <class Iterator>
            void add(Iterator begin, Iterator end,
                     double weight = 1.0) {
                QL_REQUIRE(std::distance(begin,end) == long(dimension_),
                           "SequenceStatistics::add : sample size mismatch");
                for (Size i=0; begin!=end; ++begin, ++i)
                    stats_[i].add(*begin, weight);
            }
            void reset();
            //@}
          private:
            Size dimension_;
            std::vector<statistics_type> stats_;
            mutable std::vector<double> results_;
        };

        // macros for the implementation of the "inherited" methods

        // N-D methods' definition with void argument list
        #define DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(METHOD) \
        template <class Stat> \
        std::vector<double> \
        SequenceStatistics<Stat>::METHOD() const { \
            for (Size i=0; i<dimension_; i++) \
                results_[i] = stats_[i].METHOD(); \
            return results_; \
        }
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(mean)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(variance)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(standardDeviation)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(downsideVariance)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(downsideDeviation)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(errorEstimate)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(skewness)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(kurtosis)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(min)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(max)
        #undef DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID


        // N-D methods' definition with single double argument list
        #define DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(METHOD) \
        template <class Stat> \
        std::vector<double> \
        SequenceStatistics<Stat>::METHOD(double x) const { \
            for (Size i=0; i<dimension_; i++) \
                results_[i] = stats_[i].METHOD(x); \
            return results_; \
        }
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(percentile)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(potentialUpside)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(valueAtRisk)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(expectedShortfall)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(shortfall)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(averageShortfall)
        #undef DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE


        // 1-D methods
        template <class Stat>
        SequenceStatistics<Stat>::SequenceStatistics(Size dimension)
        : dimension_(dimension), stats_(dimension, statistics_type()),
          results_(dimension) {
            QL_REQUIRE(dimension != 0,
                       "SequenceStatistics::SequenceStatistics : "
                       "null dimension for sequence statistics");
        }

        template <class Stat>
        Size SequenceStatistics<Stat>::samples() const {
            return stats_[0].samples();
        }

        template <class Stat>
        double SequenceStatistics<Stat>::weightSum() const {
            return stats_[0].weightSum();
        }

        template <class Stat>
        void SequenceStatistics<Stat>::reset() {
            for (Size i=0; i<dimension_; i++)
                stats_[i].reset();
        }

        template <class Stat>
        double SequenceStatistics<Stat>::discrepancy() const {
            // to be implemented
            return 0.0;
        }

    }

}


#endif
