
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

/*! \file sequencestatistic.hpp
    \brief Statistic tools for sequence (vector, list, array) samples

    \fullpath
    ql/Math/%sequencestatistic.hpp
*/

// $Id$

#ifndef quantlib_sequence_statistic_hpp
#define quantlib_sequence_statistic_hpp

#include <ql/Math/hstatistic.hpp>
#include <ql/Math/statistic.hpp>
#include <algorithm>
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
        template <class SequenceType, class StatisticsType>
        class SequenceStatistic {
          public:
            // typedefs
            typedef SequenceType   sequence_type;
            typedef StatisticsType statistics_type;
            virtual ~SequenceStatistic() {}
            // constructor
            SequenceStatistic(Size dimension) { reset(dimension); }
            //! \name 1-D inspectors "inherited" by underlying statistic class
            //@{
            Size samples() const;
            double weightSum() const;
            //@}
            //! \name N-D inspectors "inherited" by underlying statistic class
            //@{
            // void argument list
            sequence_type mean() const;
            sequence_type variance() const;
            sequence_type standardDeviation() const;
            sequence_type downsideVariance() const;
            sequence_type downsideDeviation() const;
            sequence_type errorEstimate() const;
            sequence_type skewness() const;
            sequence_type kurtosis() const;
            sequence_type min() const;
            sequence_type max() const;

            // single double argument list
            sequence_type gaussianPercentile(double y) const;
            sequence_type gaussianPotentialUpside(double percentile) const;
            sequence_type gaussianValueAtRisk(double percentile) const;
            sequence_type gaussianExpectedShortfall(double percentile) const;
            sequence_type gaussianShortfall(double target) const;
            sequence_type gaussianAverageShortfall(double target) const;
            sequence_type percentile(double y) const;
            sequence_type potentialUpside(double percentile) const;
            sequence_type valueAtRisk(double percentile) const;
            sequence_type expectedShortfall(double percentile) const;
            sequence_type shortfall(double target) const;
            sequence_type averageShortfall(double target) const;
            //@}
            //! \name Modifiers
            //@{

            // to be implemented
            virtual void reset(Size dimension);

            virtual void add(const sequence_type& sample,
                     double weight = 1.0);

            //@}
          protected:
            Size dimension_;
            std::vector<statistics_type> stats_;
          private:
            mutable sequence_type results_;
        };

        // macros for the implementation of the "inherited" methods

        // N-D methods' definition with void argument list
        #define DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(METHOD) \
        template <class Seq, class Stat> \
        SequenceStatistic<Seq, Stat>::sequence_type \
        SequenceStatistic<Seq, Stat>::METHOD() const { \
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
        template <class Seq, class Stat> \
        SequenceStatistic<Seq, Stat>::sequence_type \
        SequenceStatistic<Seq, Stat>::METHOD(double x) const { \
            for (Size i=0; i<dimension_; i++) \
                results_[i] = stats_[i].METHOD(x); \
            return results_; \
        }

        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianPercentile)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianPotentialUpside)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianValueAtRisk)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianExpectedShortfall)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianShortfall)

        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(percentile)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(potentialUpside)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(valueAtRisk)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(expectedShortfall)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(shortfall)
        DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(averageShortfall)
        #undef DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE


        template <class Seq, class Stat>
        void SequenceStatistic<Seq, Stat>::reset(Size dimension) {
            QL_REQUIRE(dimension > 0,
                "SequenceStatistic::SequenceStatistic : "
                "null dimension for sequence statistic");
            dimension_ = dimension;
            stats_ = std::vector<Stat>(dimension, statistics_type());
            for (Size i=0; i<dimension_; i++)
                stats_[i].reset();
            results_ = sequence_type(dimension);
        }

        template <class Seq, class Stat>
        Size SequenceStatistic<Seq, Stat>::samples() const {
            return stats_[0].samples();
        }

        template <class Seq, class Stat>
        double SequenceStatistic<Seq, Stat>::weightSum() const {
            return stats_[0].weightSum();
        }

        template <class Seq, class Stat>
        void SequenceStatistic<Seq, Stat>::add(
          const SequenceStatistic<Seq, Stat>::sequence_type& sample,
          double weight) {
            QL_REQUIRE(sample.size() == dimension_,
                   "SequenceStatistic::add : "
                   "sample size mismatch");
            Size k;
            for (k=0; k<dimension_; k++)
                stats_[k].add(sample[k], weight);

        }


        typedef SequenceStatistic<std::vector<double>, Statistic> VectorStatistic;
        typedef SequenceStatistic<Array, Statistic> ArrayStatistic;
        typedef SequenceStatistic<std::vector<double>, HStatistic> VectorHStatistic;
        typedef SequenceStatistic<Array, HStatistic> ArrayHStatistic;
    }

}


#endif
