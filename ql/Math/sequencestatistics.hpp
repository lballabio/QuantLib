
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file sequencestatistics.hpp
    \brief Statistics tools for sequence (vector, list, array) samples
*/

#ifndef quantlib_sequence_statistics_hpp
#define quantlib_sequence_statistics_hpp

#include <ql/Math/statistics.hpp>

namespace QuantLib {

    //! Statistics analysis of N-dimensional (sequence) data
    /*! It provides 1-dimensional statistics as discrepancy plus
        N-dimensional (sequence) statistics (e.g. mean,
        variance, skewness, kurtosis, etc.) with one component for each
        dimension of the sample space.

        For most of the statistics this class relies on
        the StatisticsType underlying class to provide 1-D methods that
        will be iterated for all the components of the N-D data. These
        lifted methods are the union of all the methods that might be
        requested to the 1-D underlying StatisticsType class, with the
        usual compile-time checks provided by the template approach.
    */
    template <class StatisticsType = Statistics>
    class SequenceStatistics {
      public:
        // typedefs
        typedef StatisticsType statistics_type;
        // constructor
        SequenceStatistics(Size dimension);
        //! \name inspectors
        //@{
        Size size() const { return dimension_; }
        //@}
        //! \name covariance and correlation
        //@{
        //! returns the covariance Matrix
        Disposable<Matrix> covariance() const;
        //! returns the correlation Matrix
        Disposable<Matrix> correlation() const;
        //@}
        //! \name 1-D inspectors lifted from underlying statistics class
        //@{
        Size samples() const;
        double weightSum() const;
        //@}
        //! \name N-D inspectors lifted from underlying statistics class
        //@{
        // void argument list
        std::vector<double> mean() const;
        std::vector<double> variance() const;
        std::vector<double> standardDeviation() const;
        std::vector<double> downsideVariance() const;
        std::vector<double> downsideDeviation() const;
        std::vector<double> semiVariance() const;
        std::vector<double> semiDeviation() const;
        std::vector<double> errorEstimate() const;
        std::vector<double> skewness() const;
        std::vector<double> kurtosis() const;
        std::vector<double> min() const;
        std::vector<double> max() const;

        // single double argument list
        std::vector<double> gaussianPercentile(double y) const;
        std::vector<double> percentile(double y) const;

        std::vector<double> gaussianPotentialUpside(double percentile) const;
        std::vector<double> potentialUpside(double percentile) const;

        std::vector<double> gaussianValueAtRisk(double percentile) const;
        std::vector<double> valueAtRisk(double percentile) const;

        std::vector<double> gaussianExpectedShortfall(double percentile) const;
        std::vector<double> expectedShortfall(double percentile) const;

        std::vector<double> regret(double target) const;

        std::vector<double> gaussianShortfall(double target) const;
        std::vector<double> shortfall(double target) const;

        std::vector<double> gaussianAverageShortfall(double target) const;
        std::vector<double> averageShortfall(double target) const;

        //@}
        //! \name Modifiers
        //@{
        void reset(Size dimension = 0);
        template <class Sequence>
        void add(const Sequence& sample,
                 double weight = 1.0) {
            add(sample.begin(),sample.end(),weight);
        }
        template <class Iterator>
        void add(Iterator begin,
                 Iterator end,
                 double weight = 1.0) {
            QL_REQUIRE(std::distance(begin, end) == int(dimension_),
                       "sample size mismatch");

            quadraticSum_ += weight * outerProduct(begin, end,
                                                   begin, end);

            for (Size i=0; i<dimension_; begin++, i++)
                stats_[i].add(*begin, weight);

        }
        //@}
      protected:
        Size dimension_;
        std::vector<statistics_type> stats_;
        mutable std::vector<double> results_;
        Matrix quadraticSum_;
    };


    // inline definitions

    template <class Stat>
    inline SequenceStatistics<Stat>::SequenceStatistics(Size dimension)
    : dimension_(0) {
        reset(dimension);
    }

    template <class Stat>
    inline Size SequenceStatistics<Stat>::samples() const {
        return stats_[0].samples();
    }

    template <class Stat>
    inline double SequenceStatistics<Stat>::weightSum() const {
        return stats_[0].weightSum();
    }


    // macros for the implementation of the lifted methods

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
    DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(semiVariance)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(semiDeviation)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(errorEstimate)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(skewness)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(kurtosis)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(min)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(max)
    #undef DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID


    // N-D methods' definition with single double argument
    #define DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(METHOD) \
    template <class Stat> \
    std::vector<double> \
    SequenceStatistics<Stat>::METHOD(double x) const { \
        for (Size i=0; i<dimension_; i++) \
            results_[i] = stats_[i].METHOD(x); \
        return results_; \
    }

    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianPercentile)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianPotentialUpside)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianValueAtRisk)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianExpectedShortfall)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianShortfall)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(gaussianAverageShortfall)

    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(percentile)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(potentialUpside)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(valueAtRisk)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(expectedShortfall)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(regret)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(shortfall)
    DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(averageShortfall)
    #undef DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE


    template <class Stat>
    void SequenceStatistics<Stat>::reset(Size dimension) {
        if (dimension == 0)           // if no size given,
            dimension = dimension_;   // keep the current one
        QL_REQUIRE(dimension > 0, "null dimension");
        if (dimension == dimension_) {
            for (Size i=0; i<dimension_; i++)
                stats_[i].reset();
        } else {
            dimension_ = dimension;
            stats_ = std::vector<Stat>(dimension);
            results_ = std::vector<double>(dimension);
        }
        quadraticSum_ = Matrix(dimension_, dimension_, 0.0);
    }



    template <class Stat>
    Disposable<Matrix> SequenceStatistics<Stat>::covariance() const {
        double sampleWeight = weightSum();
        QL_REQUIRE(sampleWeight > 0.0,
                   "sampleWeight=0, unsufficient");

        double sampleNumber = samples();
        QL_REQUIRE(sampleNumber > 1.0,
                   "sample number <=1, unsufficient");

        std::vector<double> m = mean();
        double inv = 1.0/sampleWeight;

        Matrix result = inv*quadraticSum_;
        result -= outerProduct(m.begin(), m.end(),
                               m.begin(), m.end());

        result *= (sampleNumber/(sampleNumber-1.0));
        return result;
    }


    template <class Stat>
    Disposable<Matrix> SequenceStatistics<Stat>::correlation() const {
        Matrix correlation = covariance();
        Array variances = correlation.diagonal();
        for (Size i=0; i<dimension_; i++){
            for (Size j=0; j<dimension_; j++){
                if (i==j) {
                    if (variances[i]==0.0) {
                        correlation[i][j] = 1.0;
                    } else {
                        correlation[i][j] *= 
                            1.0/QL_SQRT(variances[i]*variances[j]);
                    }
                } else {
                    if (variances[i]==0.0 && variances[j]==0) {
                        correlation[i][j] = 1.0;
                    } else if (variances[i]==0.0 || variances[j]==0.0) {
                        correlation[i][j] = 0.0;
                    } else {
                        correlation[i][j] *= 
                            1.0/QL_SQRT(variances[i]*variances[j]);
                    }
                }
            } // j for
        } // i for

        return correlation;
    }

}


#endif
