/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2022 Chester Wong
 Copyright (C) 2022 Wojciech Czernous


 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file sequencestatistics.hpp
    \brief Statistics tools for sequence (vector, list, array) samples
*/

#ifndef quantlib_sequence_statistics_hpp
#define quantlib_sequence_statistics_hpp

#include <ql/math/statistics/statistics.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/math/matrix.hpp>

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

        The likelihood-ratio test statistic for covariance matrix 
        under p-variate normal distribution assumption, is based on 
        <i>
        Tiefeng Jiang, Fan Yang "Central limit theorems for classical 
        likelihood ratio tests for high-dimensional normal distributions," 
        The Annals of Statistics, Ann. Statist. 41(4), 2029-2074, (August 2013)
        https://dx.doi.org/10.1214/13-AOS1134
        Section 2.5 is relevant here.
        </i>
        For this statistic, we produce the cdf value of N(0,1) distribution,
        so the result should be distributed uniformly on (0,1).


        \test the correctness of the returned values is tested by
              checking them against numerical calculations; 
              LRT statistic for covariance matrix
              is calculated for a standard normal sequence generator
    */
    template <class StatisticsType>
    class GenericSequenceStatistics {
      public:
        // typedefs
        typedef StatisticsType statistics_type;
        typedef std::vector<typename StatisticsType::value_type> value_type;
        // constructor
        GenericSequenceStatistics(Size dimension = 0);
        //! \name inspectors
        //@{
        Size size() const { return dimension_; }
        //@}
        //! \name covariance and correlation
        //@{
        //! returns the sample covariance Matrix (unbiased)
        Matrix covariance() const;
        //! returns the correlation Matrix
        Matrix correlation() const;
        //! returns the likelihood-ratio test statistic for covariance
        Real likelihoodratiotest(const Matrix& expectedCovariance) const;
        //@}
        //! \name 1-D inspectors lifted from underlying statistics class
        //@{
        Size samples() const;
        Real weightSum() const;
        //@}
        //! \name N-D inspectors lifted from underlying statistics class
        //@{
        // void argument list
        std::vector<Real> mean() const;
        std::vector<Real> variance() const;
        std::vector<Real> standardDeviation() const;
        std::vector<Real> downsideVariance() const;
        std::vector<Real> downsideDeviation() const;
        std::vector<Real> semiVariance() const;
        std::vector<Real> semiDeviation() const;
        std::vector<Real> errorEstimate() const;
        std::vector<Real> skewness() const;
        std::vector<Real> kurtosis() const;
        std::vector<Real> min() const;
        std::vector<Real> max() const;

        // single argument list
        std::vector<Real> gaussianPercentile(Real y) const;
        std::vector<Real> percentile(Real y) const;

        std::vector<Real> gaussianPotentialUpside(Real percentile) const;
        std::vector<Real> potentialUpside(Real percentile) const;

        std::vector<Real> gaussianValueAtRisk(Real percentile) const;
        std::vector<Real> valueAtRisk(Real percentile) const;

        std::vector<Real> gaussianExpectedShortfall(Real percentile) const;
        std::vector<Real> expectedShortfall(Real percentile) const;

        std::vector<Real> regret(Real target) const;

        std::vector<Real> gaussianShortfall(Real target) const;
        std::vector<Real> shortfall(Real target) const;

        std::vector<Real> gaussianAverageShortfall(Real target) const;
        std::vector<Real> averageShortfall(Real target) const;

        //@}
        //! \name Modifiers
        //@{
        void reset(Size dimension = 0);
        template <class Sequence>
        void add(const Sequence& sample,
                 Real weight = 1.0) {
            add(sample.begin(), sample.end(), weight);
        }
        template <class Iterator>
        void add(Iterator begin,
                 Iterator end,
                 Real weight = 1.0) {
            if (dimension_ == 0) {
                // stat wasn't initialized yet
                QL_REQUIRE(end>begin, "sample error: end<=begin");
                Size dimension = std::distance(begin, end);
                reset(dimension);
            }

            QL_REQUIRE(std::distance(begin, end) == Integer(dimension_),
                       "sample size mismatch: " << dimension_ <<
                       " required, " << std::distance(begin, end) <<
                       " provided");

            quadraticSum_ += weight * outerProduct(begin, end,
                                                   begin, end);

            for (Size i=0; i<dimension_; ++begin, ++i)
                stats_[i].add(*begin, weight);

        }
        //@}
      protected:
        Size dimension_ = 0;
        std::vector<statistics_type> stats_;
        mutable std::vector<Real> results_;
        Matrix quadraticSum_;
    };

    //! default multi-dimensional statistics tool
    /*! \test the correctness of the returned values is tested by
              checking them against numerical calculations.
    */
    typedef GenericSequenceStatistics<Statistics> SequenceStatistics;
    typedef GenericSequenceStatistics<IncrementalStatistics> SequenceStatisticsInc;

    // inline definitions

    template <class Stat>
    inline GenericSequenceStatistics<Stat>::GenericSequenceStatistics(Size dimension) {
        reset(dimension);
    }

    template <class Stat>
    inline Size GenericSequenceStatistics<Stat>::samples() const {
        return (stats_.empty()) ? 0 : stats_[0].samples();
    }

    template <class Stat>
    inline Real GenericSequenceStatistics<Stat>::weightSum() const {
        return (stats_.empty()) ? 0.0 : stats_[0].weightSum();
    }


    // macros for the implementation of the lifted methods

    // N-D methods' definition with void argument list
    #define DEFINE_SEQUENCE_STAT_CONST_METHOD_VOID(METHOD) \
    template <class Stat> \
    std::vector<Real> \
    GenericSequenceStatistics<Stat>::METHOD() const { \
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


    // N-D methods' definition with single argument
    #define DEFINE_SEQUENCE_STAT_CONST_METHOD_DOUBLE(METHOD) \
    template <class Stat> \
    std::vector<Real> \
    GenericSequenceStatistics<Stat>::METHOD(Real x) const { \
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
    void GenericSequenceStatistics<Stat>::reset(Size dimension) {
        // (re-)initialize
        if (dimension > 0) {
            if (dimension == dimension_) {
                for (Size i=0; i<dimension_; ++i)
                    stats_[i].reset();
            } else {
                dimension_ = dimension;
                stats_ = std::vector<Stat>(dimension);
                results_ = std::vector<Real>(dimension);
            }
            quadraticSum_ = Matrix(dimension_, dimension_, 0.0);
        } else {
            dimension_ = dimension;
        }
    }

    template <class Stat>
    Matrix GenericSequenceStatistics<Stat>::covariance() const {
        Real sampleWeight = weightSum();
        QL_REQUIRE(sampleWeight > 0.0,
                   "sampleWeight=0, unsufficient");

        Real sampleNumber = static_cast<Real>(samples());
        QL_REQUIRE(sampleNumber > 1.0,
                   "sample number <=1, unsufficient");

        std::vector<Real> m = mean();
        Real inv = 1.0/sampleWeight;

        Matrix result = inv*quadraticSum_;
        result -= outerProduct(m.begin(), m.end(),
                               m.begin(), m.end());

        result *= (sampleNumber/(sampleNumber-1.0));
        return result;
    }


    template <class Stat>
    Matrix GenericSequenceStatistics<Stat>::correlation() const {
        Matrix correlation = covariance();
        Array variances = correlation.diagonal();
        for (Size i=0; i<dimension_; i++){
            for (Size j=0; j<dimension_; j++){
                if (i==j) {
                    if (variances[i]==0.0) {
                        correlation[i][j] = 1.0;
                    } else {
                        correlation[i][j] *=
                            1.0/std::sqrt(variances[i]*variances[j]);
                    }
                } else {
                    if (variances[i]==0.0 && variances[j]==0) {
                        correlation[i][j] = 1.0;
                    } else if (variances[i]==0.0 || variances[j]==0.0) {
                        correlation[i][j] = 0.0;
                    } else {
                        correlation[i][j] *=
                            1.0/std::sqrt(variances[i]*variances[j]);
                    }
                }
            } // j for
        } // i for

        return correlation;
    }


    template <class Stat>
    Real GenericSequenceStatistics<Stat>::likelihoodratiotest(const Matrix& expectedCovariance) const {
        QL_REQUIRE(
            expectedCovariance.columns() == dimension_ &&
            expectedCovariance.rows() == dimension_, 
            "The given (expected) covariance matrix has a wrong size:"
            << expectedCovariance.rows() 
            << " x "
            << expectedCovariance.columns() 
            << ", while dimension = "
            << dimension_);
        Real p = dimension_;
        Real n = samples();
        QL_REQUIRE(
            n > 1+p,
            "The sample size"
            " (n = " << n << ")"
            "should be larger than one plus dimension"
            " (p = " << p << ")"
        );
        Matrix expCovInv = inverse(expectedCovariance);
        // This should be multiplied by (n-1) to match the notation of Jiang&Yang:
        Matrix A_cov = covariance();
        Matrix A = expCovInv * A_cov; 
        const Array& diag = A.diagonal();
        Real trace = 0.0;
        for (Size i = 0; i < dimension_; ++i)
            trace += diag[i];
        trace *= n - 1;
        Real logdet = std::log(determinant(A)) + p * std::log(n-1);
        Real logn = std::log(n);
        std::vector<Real> ms = mean();
        Real means2 = std::inner_product(ms.begin(),ms.end(),ms.begin(),Real(0.0));
        Real log_Lambda_n_by_n = p/2*(1-logn) + logdet/2 - trace/2/n - means2/2;
        Real mu_n_by_n = -0.25;
        mu_n_by_n *= (2*n-2*p-3)*std::log(1-p/(n-1)) + 2*p + 2*p/n;

        Real sigma_n_sq = -0.5;
        sigma_n_sq *= p/(n-1) + std::log(1-p/(n-1));
        Real sigma_n = std::sqrt(sigma_n_sq);

        return (log_Lambda_n_by_n - mu_n_by_n) / sigma_n; 
    }
}


#endif
