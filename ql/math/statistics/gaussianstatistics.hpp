/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file gaussianstatistics.hpp
    \brief statistics tool for gaussian-assumption risk measures
*/

#ifndef quantlib_gaussian_statistics_h
#define quantlib_gaussian_statistics_h

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/statistics/generalstatistics.hpp>

namespace QuantLib {

    //! Statistics tool for gaussian-assumption risk measures
    /*! This class wraps a somewhat generic statistic tool and adds
        a number of gaussian risk measures (e.g.: value-at-risk, expected
        shortfall, etc.) based on the mean and variance provided by
        the underlying statistic tool.
    */
    template<class Stat>
    class GenericGaussianStatistics : public Stat {
      public:
        typedef typename Stat::value_type value_type;
        GenericGaussianStatistics() = default;
        explicit GenericGaussianStatistics(const Stat& s) : Stat(s) {}
        //! \name Gaussian risk measures
        //@{
        /*! returns the downside variance, defined as
            \f[ \frac{N}{N-1} \times \frac{ \sum_{i=1}^{N}
                \theta \times x_i^{2}}{ \sum_{i=1}^{N} w_i} \f],
            where \f$ \theta \f$ = 0 if x > 0 and
            \f$ \theta \f$ =1 if x <0
        */
        Real gaussianDownsideVariance() const {
            return gaussianRegret(0.0);
        }

        /*! returns the downside deviation, defined as the
            square root of the downside variance.
        */
        Real gaussianDownsideDeviation() const {
            return std::sqrt(gaussianDownsideVariance());
        }

        /*! returns the variance of observations below target
            \f[ \frac{\sum w_i (min(0, x_i-target))^2 }{\sum w_i}. \f]

            See Dembo, Freeman "The Rules Of Risk", Wiley (2001)
        */
        Real gaussianRegret(Real target) const;


        /*! gaussian-assumption y-th percentile, defined as the value x
            such that \f[ y = \frac{1}{\sqrt{2 \pi}}
                                      \int_{-\infty}^{x} \exp (-u^2/2) du \f]
        */
        Real gaussianPercentile(Real percentile) const;
        Real gaussianTopPercentile(Real percentile) const;

        //! gaussian-assumption Potential-Upside at a given percentile
        Real gaussianPotentialUpside(Real percentile) const;

        //! gaussian-assumption Value-At-Risk at a given percentile
        Real gaussianValueAtRisk(Real percentile) const;

        //! gaussian-assumption Expected Shortfall at a given percentile
        /*! Assuming a gaussian distribution it
            returns the expected loss in case that the loss exceeded
            a VaR threshold,

            \f[ \mathrm{E}\left[ x \;|\; x < \mathrm{VaR}(p) \right], \f]

            that is the average of observations below the
            given percentile \f$ p \f$.
            Also know as conditional value-at-risk.

            See Artzner, Delbaen, Eber and Heath,
            "Coherent measures of risk", Mathematical Finance 9 (1999)
        */
        Real gaussianExpectedShortfall(Real percentile) const;

        //! gaussian-assumption Shortfall (observations below target)
        Real gaussianShortfall(Real target) const;

        //! gaussian-assumption Average Shortfall (averaged shortfallness)
        Real gaussianAverageShortfall(Real target) const;
        //@}
    };

    //! default gaussian statistic tool
    typedef GenericGaussianStatistics<GeneralStatistics> GaussianStatistics;


    //! Helper class for precomputed distributions
    class StatsHolder {
      public:
        typedef Real value_type;
        StatsHolder(Real mean,
                    Real standardDeviation)
                    : mean_(mean), standardDeviation_(standardDeviation) {}
        ~StatsHolder() = default;
        Real mean() const { return mean_; }
        Real standardDeviation() const { return standardDeviation_; }
      private:
        Real mean_, standardDeviation_;
    };


    // inline definitions

    template<class Stat>
    inline
    Real GenericGaussianStatistics<Stat>::gaussianRegret(Real target) const {
        Real m = this->mean();
        Real std = this->standardDeviation();
        Real variance = std*std;
        CumulativeNormalDistribution gIntegral(m, std);
        NormalDistribution g(m, std);
        Real firstTerm = variance + m*m - 2.0*target*m + target*target;
        Real alfa = gIntegral(target);
        Real secondTerm = m - target;
        Real beta = variance*g(target);
        Real result = alfa*firstTerm - beta*secondTerm;
        return result/alfa;
    }

    /*! \pre percentile must be in range (0%-100%) extremes excluded */
    template<class Stat>
    inline Real GenericGaussianStatistics<Stat>::gaussianPercentile(
                                                     Real percentile) const {

        QL_REQUIRE(percentile>0.0,
                   "percentile (" << percentile << ") must be > 0.0");
        QL_REQUIRE(percentile<1.0,
                   "percentile (" << percentile << ") must be < 1.0");

        InverseCumulativeNormal gInverse(Stat::mean(),
                                         Stat::standardDeviation());
        return gInverse(percentile);
    }

    /*! \pre percentile must be in range (0%-100%) extremes excluded */
    template<class Stat>
    inline Real GenericGaussianStatistics<Stat>::gaussianTopPercentile(
                                                     Real percentile) const {

        return gaussianPercentile(1.0-percentile);
    }

    /*! \pre percentile must be in range [90%-100%) */
    template<class Stat>
    inline Real GenericGaussianStatistics<Stat>::gaussianPotentialUpside(
                                                    Real percentile) const {

        QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                   "percentile (" << percentile << ") out of range [0.9, 1)");

        Real result = gaussianPercentile(percentile);
        // potential upside must be a gain, i.e., floored at 0.0
        return std::max<Real>(result, 0.0);
    }


    /*! \pre percentile must be in range [90%-100%) */
    template<class Stat>
    inline Real GenericGaussianStatistics<Stat>::gaussianValueAtRisk(
                                                    Real percentile) const {

        QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                   "percentile (" << percentile << ") out of range [0.9, 1)");

        Real result = gaussianPercentile(1.0-percentile);
        // VAR must be a loss
        // this means that it has to be MIN(dist(1.0-percentile), 0.0)
        // VAR must also be a positive quantity, so -MIN(*)
        return -std::min<Real>(result, 0.0);
    }


    /*! \pre percentile must be in range [90%-100%) */
    template<class Stat>
    inline Real GenericGaussianStatistics<Stat>::gaussianExpectedShortfall(
                                                    Real percentile) const {
        QL_REQUIRE(percentile<1.0 && percentile>=0.9,
                   "percentile (" << percentile << ") out of range [0.9, 1)");

        Real m = this->mean();
        Real std = this->standardDeviation();
        InverseCumulativeNormal gInverse(m, std);
        Real var = gInverse(1.0-percentile);
        NormalDistribution g(m, std);
        Real result = m - std*std*g(var)/(1.0-percentile);
        // expectedShortfall must be a loss
        // this means that it has to be MIN(result, 0.0)
        // expectedShortfall must also be a positive quantity, so -MIN(*)
        return -std::min<Real>(result, 0.0);
    }


    template<class Stat>
    inline Real GenericGaussianStatistics<Stat>::gaussianShortfall(
                                                        Real target) const {
        CumulativeNormalDistribution gIntegral(this->mean(),
                                               this->standardDeviation());
        return gIntegral(target);
    }


    template<class Stat>
    inline Real GenericGaussianStatistics<Stat>::gaussianAverageShortfall(
                                                        Real target) const {
        Real m = this->mean();
        Real std = this->standardDeviation();
        CumulativeNormalDistribution gIntegral(m, std);
        NormalDistribution g(m, std);
        return ( (target-m) + std*std*g(target)/gIntegral(target) );
    }

}


#endif


#ifndef id_2e9632ca55ce0773d8291ccfe06b62f6
#define id_2e9632ca55ce0773d8291ccfe06b62f6
inline bool test_2e9632ca55ce0773d8291ccfe06b62f6(int* i) { return i != 0; }
#endif
