/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

/*! \file riskstatistics.hpp
    \brief empirical-distribution risk measures
*/

#ifndef quantlib_risk_statistics_h
#define quantlib_risk_statistics_h

#include <ql/math/functional.hpp>
#include <ql/math/statistics/gaussianstatistics.hpp>

namespace QuantLib {

    //! empirical-distribution risk measures
    /*! This class wraps a somewhat generic statistic tool and adds
        a number of risk measures (e.g.: value-at-risk, expected
        shortfall, etc.) based on the data distribution as reported by
        the underlying statistic tool.

        \todo add historical annualized volatility

    */
    template <class S>
    class GenericRiskStatistics : public S {
      public:
        typedef typename S::value_type value_type;

        /*! returns the variance of observations below the mean,
            \f[ \frac{N}{N-1}
                \mathrm{E}\left[ (x-\langle x \rangle)^2 \;|\;
                                  x < \langle x \rangle \right]. \f]

            See Markowitz (1959).
        */
        Real semiVariance() const;

        /*! returns the semi deviation, defined as the
            square root of the semi variance.
        */
        Real semiDeviation() const;

        /*! returns the variance of observations below 0.0,
            \f[ \frac{N}{N-1}
                \mathrm{E}\left[ x^2 \;|\; x < 0\right]. \f]
        */
        Real downsideVariance() const;

        /*! returns the downside deviation, defined as the
            square root of the downside variance.
        */
        Real downsideDeviation() const;

        /*! returns the variance of observations below target,
            \f[ \frac{N}{N-1}
                \mathrm{E}\left[ (x-t)^2 \;|\;
                                  x < t \right]. \f]

            See Dembo and Freeman, "The Rules Of Risk", Wiley (2001).
        */
        Real regret(Real target) const;

        //! potential upside (the reciprocal of VAR) at a given percentile
        Real potentialUpside(Real percentile) const;

        //! value-at-risk at a given percentile
        Real valueAtRisk(Real percentile) const;

        //! expected shortfall at a given percentile
        /*! returns the expected loss in case that the loss exceeded
            a VaR threshold,

            \f[ \mathrm{E}\left[ x \;|\; x < \mathrm{VaR}(p) \right], \f]

            that is the average of observations below the
            given percentile \f$ p \f$.
            Also know as conditional value-at-risk.

            See Artzner, Delbaen, Eber and Heath,
            "Coherent measures of risk", Mathematical Finance 9 (1999)
        */
        Real expectedShortfall(Real percentile) const;

        /*! probability of missing the given target, defined as
            \f[ \mathrm{E}\left[ \Theta \;|\; (-\infty,\infty) \right] \f]
            where
            \f[ \Theta(x) = \left\{
                \begin{array}{ll}
                1 & x < t \\
                0 & x \geq t
                \end{array}
                \right. \f]
        */
        Real shortfall(Real target) const;

        /*! averaged shortfallness, defined as
            \f[ \mathrm{E}\left[ t-x \;|\; x<t \right] \f]
        */
        Real averageShortfall(Real target) const;
    };


    //! default risk measures tool
    /*! \test the correctness of the returned values is tested by
              checking them against numerical calculations.
    */
    typedef GenericRiskStatistics<GaussianStatistics> RiskStatistics;



    // inline definitions

    template <class S>
    inline Real GenericRiskStatistics<S>::semiVariance() const {
        return regret(this->mean());
    }

    template <class S>
    inline Real GenericRiskStatistics<S>::semiDeviation() const {
        return std::sqrt(semiVariance());
    }

    template <class S>
    inline Real GenericRiskStatistics<S>::downsideVariance() const {
        return regret(0.0);
    }

    template <class S>
    inline Real GenericRiskStatistics<S>::downsideDeviation() const {
        return std::sqrt(downsideVariance());
    }

    // template definitions

    template <class S>
    Real GenericRiskStatistics<S>::regret(Real target) const {
        // average over the range below the target
        std::pair<Real,Size> result =
            this->expectationValue(compose(square<Real>(),
                                           subtract<Real>(target)),
                                   less_than<Real>(target));
        Real x = result.first;
        Size N = result.second;
        QL_REQUIRE(N > 1,
                   "samples under target <= 1, unsufficient");
        return (N/(N-1.0))*x;
    }

    /*! \pre percentile must be in range [90%-100%) */
    template <class S>
    Real GenericRiskStatistics<S>::potentialUpside(Real centile)
        const {
        QL_REQUIRE(centile>=0.9 && centile<1.0,
                   "percentile (" << centile << ") out of range [0.9, 1.0)");

        // potential upside must be a gain, i.e., floored at 0.0
        return std::max<Real>(this->percentile(centile), 0.0);
    }

    /*! \pre percentile must be in range [90%-100%) */
    template <class S>
    Real GenericRiskStatistics<S>::valueAtRisk(Real centile) const {

        QL_REQUIRE(centile>=0.9 && centile<1.0,
                   "percentile (" << centile << ") out of range [0.9, 1.0)");

        // must be a loss, i.e., capped at 0.0 and negated
        return -std::min<Real>(this->percentile(1.0-centile), 0.0);
    }

    /*! \pre percentile must be in range [90%-100%) */
    template <class S>
    Real GenericRiskStatistics<S>::expectedShortfall(Real centile) const {
        QL_REQUIRE(centile>=0.9 && centile<1.0,
                   "percentile (" << centile << ") out of range [0.9, 1.0)");

        QL_ENSURE(this->samples() != 0, "empty sample set");
        Real target = -valueAtRisk(centile);
        std::pair<Real,Size> result =
            this->expectationValue(identity<Real>(),
                                   less_than<Real>(target));
        Real x = result.first;
        Size N = result.second;
        QL_ENSURE(N != 0, "no data below the target");
        // must be a loss, i.e., capped at 0.0 and negated
        return -std::min<Real>(x, 0.0);
    }

    template <class S>
    Real GenericRiskStatistics<S>::shortfall(Real target) const {
        QL_ENSURE(this->samples() != 0, "empty sample set");
        return this->expectationValue(clip(constant<Real,Real>(1.0),
                                           less_than<Real>(target)),
                                      everywhere()).first;
    }

    template <class S>
    Real GenericRiskStatistics<S>::averageShortfall(Real target)
        const {
        std::pair<Real,Size> result =
            this->expectationValue(subtract_from<Real>(target),
                                   less_than<Real>(target));
        Real x = result.first;
        Size N = result.second;
        QL_ENSURE(N != 0, "no data below the target");
        return x;
    }

}


#endif



#ifndef id_cc8b66422c7e1dbd1708003559f00a14
#define id_cc8b66422c7e1dbd1708003559f00a14
inline bool test_cc8b66422c7e1dbd1708003559f00a14(int* i) { return i != 0; }
#endif
