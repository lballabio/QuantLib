
/*
 Copyright (C) 2003 RiskMap srl

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

/*! \file riskstatistics.hpp
    \brief empirical-distribution risk measures
*/

#ifndef quantlib_risk_statistics_h
#define quantlib_risk_statistics_h

#include <ql/Math/functional.hpp>
#include <ql/Math/generalstatistics.hpp>
#include <ql/Math/gaussianstatistics.hpp>

namespace QuantLib {

    //! empirical-distribution risk measures
    /*! This class wraps a somewhat generic statistic tool and adds
        a number of risk measures (e.g.: value-at-risk, expected 
        shortfall, etc.) based on the data distribution as reported by
        the underlying tool.

        \todo add historical annualized volatility

    */
    template <class S>
    class GenericRiskStatistics : public S {
      public:
        /*! returns the variance of observations below the mean, 
            \f[ \frac{N}{N-1} 
                \mathrm{E}\left[ (x-\langle x \rangle)^2 \;|\;
                                  x < \langle x \rangle \right]. \f]

            See Markowitz (1959).
        */
        double semiVariance() const;

        /*! returns the semi deviation, defined as the
            square root of the semi variance.
        */
        double semiDeviation() const;

        /*! returns the variance of observations below 0.0,
            \f[ \frac{N}{N-1} 
                \mathrm{E}\left[ x^2 \;|\; x < 0\right]. \f]
        */
        double downsideVariance() const;

        /*! returns the downside deviation, defined as the
            square root of the downside variance.
        */
        double downsideDeviation() const;

        /*! returns the variance of observations below target,
            \f[ \frac{N}{N-1} 
                \mathrm{E}\left[ (x-t)^2 \;|\;
                                  x < t \right]. \f]

            See Dembo and Freeman, "The Rules Of Risk", Wiley (2001).
        */
        double regret(double target) const;

        //! potential upside (the reciprocal of VAR) at a given percentile
        double potentialUpside(double percentile) const;

        //! value-at-risk at a given percentile
        double valueAtRisk(double percentile) const;

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
        double expectedShortfall(double percentile) const;

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
        double shortfall(double target) const;

        /*! averaged shortfallness, defined as
            \f[ \mathrm{E}\left[ t-x \;|\; x<t \right] \f]
        */
        double averageShortfall(double target) const;
    };


    //! default risk measures tool
    typedef GaussianStatistics<GenericRiskStatistics<GeneralStatistics> > 
                                                               RiskStatistics;



    // inline definitions

    template <class S>
    inline double GenericRiskStatistics<S>::semiVariance() const {
        return regret(mean());
    }

    template <class S>
    inline double GenericRiskStatistics<S>::semiDeviation() const {
        return QL_SQRT(semiVariance());
    }

    template <class S>
    inline double GenericRiskStatistics<S>::downsideVariance() const {
        return regret(0.0);
    }
 
    template <class S>
    inline double GenericRiskStatistics<S>::downsideDeviation() const {
        return QL_SQRT(downsideVariance());
    }

    // template definitions

    template <class S>
    double GenericRiskStatistics<S>::regret(double target) const {
        // average over the range below the target
        std::pair<double,Size> result =
            expectationValue(compose(square<double>(),
                                     std::bind2nd(
                                                  std::minus<double>(),
                                                  target)),
                             std::bind2nd(std::less<double>(),
                                          target));
        double x = result.first;
        Size N = result.second;
        QL_REQUIRE(N > 1,
                   "GenericRiskStatistics::regret() : "
                   "samples under target <= 1, unsufficient");
        return (N/(N-1.0))*x;
    }

    /*! \pre percentile must be in range [90%-100%) */
    template <class S>
    double GenericRiskStatistics<S>::potentialUpside(double centile)
        const {
        QL_REQUIRE(centile>=0.9 && centile<1.0,
                   "GenericRiskStatistics::potentialUpside() : "
                   "percentile (" +
                   DoubleFormatter::toString(centile) +
                   ") must be in [0.9,1.0)");

        // must be a gain, i.e., floored at 0.0
        return QL_MAX(topPercentile(1.0-centile), 0.0);
    }

    /*! \pre percentile must be in range [90%-100%) */
    template <class S>
    double GenericRiskStatistics<S>::valueAtRisk(double centile) const {

        QL_REQUIRE(centile>=0.9 && centile<1.0,
                   "GenericRiskStatistics::valueAtRisk() : "
                   "percentile (" +
                   DoubleFormatter::toString(centile) +
                   ") must be in [0.9,1.0)");

        // must be a loss, i.e., capped at 0.0 and negated
        return -QL_MIN(percentile(1.0-centile), 0.0);
    }

    /*! \pre percentile must be in range [90%-100%) */
    template <class S>
    double GenericRiskStatistics<S>::expectedShortfall(double centile) const {
        QL_REQUIRE(centile>=0.9 && centile<1.0,
                   "GenericRiskStatistics::expectedShortfall() : "
                   "percentile (" +
                   DoubleFormatter::toString(centile) +
                   ") must be in [0.9,1.0)");
        QL_ENSURE(samples() != 0,
                  "RiskStatistics::expectedShortfall() : "
                  "empty sample set");
        double target = -valueAtRisk(centile);
        std::pair<double,Size> result = 
            expectationValue(identity<double>(),
                             std::bind2nd(std::less<double>(),
                                          target));
        double x = result.first;
        Size N = result.second;
        QL_ENSURE(N != 0,
                  "GenericRiskStatistics::expectedShortfall() : "
                  "no data below the target");
        // must be a loss, i.e., capped at 0.0 and negated
        return -QL_MIN(x, 0.0);
    }

    template <class S>
    double GenericRiskStatistics<S>::shortfall(double target) const {
        QL_ENSURE(samples() != 0,
                  "GenericRiskStatistics::shortfall() : "
                  "empty sample set");
        return expectationValue(clip(constant<double,double>(1.0),
                                     std::bind2nd(std::less<double>(),
                                                  target)),
                                everywhere()).first;
    }

    template <class S>
    double GenericRiskStatistics<S>::averageShortfall(double target) 
        const {
        std::pair<double,Size> result = 
            expectationValue(std::bind1st(std::minus<double>(),
                                          target),
                             std::bind2nd(std::less<double>(),
                                          target));
        double x = result.first;
        Size N = result.second;
        QL_ENSURE(N != 0,
                  "GenericRiskStatistics::averageShortfall() : "
                  "no data below the target");
        return x;
    }

}


#endif

