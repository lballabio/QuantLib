/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2010 Kakhkhor Abdijalilov

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

/*! \file normaldistribution.hpp
    \brief normal, cumulative and inverse cumulative distributions
*/

#ifndef quantlib_normal_distribution_hpp
#define quantlib_normal_distribution_hpp

#include <ql/math/errorfunction.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    //! Normal distribution function
    /*! Given x, it returns its probability in a Gaussian normal distribution.
        It provides the first derivative too.

        \test the correctness of the returned value is tested by
              checking it against numerical calculations. Cross-checks
              are also performed against the
              CumulativeNormalDistribution and InverseCumulativeNormal
              classes.
    */
    class NormalDistribution {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        NormalDistribution(Real average = 0.0,
                           Real sigma = 1.0);
        // function
        Real operator()(Real x) const;
        Real derivative(Real x) const;
      private:
        Real average_, sigma_, normalizationFactor_, denominator_,
            derNormalizationFactor_;
    };

    typedef NormalDistribution GaussianDistribution;


    //! Cumulative normal distribution function
    /*! Given x it provides an approximation to the
        integral of the gaussian normal distribution:
        formula here ...

        For this implementation see M. Abramowitz and I. Stegun,
        Handbook of Mathematical Functions,
        Dover Publications, New York (1972)
    */
    class CumulativeNormalDistribution {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        CumulativeNormalDistribution(Real average = 0.0,
                                     Real sigma   = 1.0);
        // function
        Real operator()(Real x) const;
        Real derivative(Real x) const;
      private:
        Real average_, sigma_;
        NormalDistribution gaussian_;
        ErrorFunction errorFunction_;
    };


    //! Inverse cumulative normal distribution function
    /*! Given x between zero and one as
      the integral value of a gaussian normal distribution
      this class provides the value y such that
      formula here ...

      It use Acklam's approximation:
      by Peter J. Acklam, University of Oslo, Statistics Division.
      URL: http://home.online.no/~pjacklam/notes/invnorm/index.html

      This class can also be used to generate a gaussian normal
      distribution from a uniform distribution.
      This is especially useful when a gaussian normal distribution
      is generated from a low discrepancy uniform distribution:
      in this case the traditional Box-Muller approach and its
      variants would not preserve the sequence's low-discrepancy.

    */
    class InverseCumulativeNormal {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        InverseCumulativeNormal(Real average = 0.0,
                                Real sigma   = 1.0);
        // function
        Real operator()(Real x) const {
            return average_ + sigma_*standard_value(x);
        }
        // value for average=0, sigma=1
        /* Compared to operator(), this method avoids 2 floating point
           operations (we use average=0 and sigma=1 most of the
           time). The speed difference is noticeable.
        */
        static Real standard_value(Real x) {
            Real z;
            if (x < x_low_ || x_high_ < x) {
                z = tail_value(x);
            } else {
                z = x - 0.5;
                Real r = z*z;
                z = (((((a1_*r+a2_)*r+a3_)*r+a4_)*r+a5_)*r+a6_)*z /
                    (((((b1_*r+b2_)*r+b3_)*r+b4_)*r+b5_)*r+1.0);
            }

            // The relative error of the approximation has absolute value less
            // than 1.15e-9.  One iteration of Halley's rational method (third
            // order) gives full machine precision.
            // #define REFINE_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
            #ifdef REFINE_TO_FULL_MACHINE_PRECISION_USING_HALLEYS_METHOD
            // error (f_(z) - x) divided by the cumulative's derivative
            const Real r = (f_(z) - x) * M_SQRT2 * M_SQRTPI * exp(0.5 * z*z);
            //  Halley's method
            z -= r/(1+0.5*z*r);
            #endif

            return z;
        }
      private:
        /* Handling tails moved into a separate method, which should
           make the inlining of operator() and standard_value method
           easier. tail_value is called rarely and doesn't need to be
           inlined.
        */
        static Real tail_value(Real x);
        #if defined(QL_PATCH_SOLARIS)
        CumulativeNormalDistribution f_;
        #else
        static const CumulativeNormalDistribution f_;
        #endif
        Real average_, sigma_;
        static const Real a1_;
        static const Real a2_;
        static const Real a3_;
        static const Real a4_;
        static const Real a5_;
        static const Real a6_;
        static const Real b1_;
        static const Real b2_;
        static const Real b3_;
        static const Real b4_;
        static const Real b5_;
        static const Real c1_;
        static const Real c2_;
        static const Real c3_;
        static const Real c4_;
        static const Real c5_;
        static const Real c6_;
        static const Real d1_;
        static const Real d2_;
        static const Real d3_;
        static const Real d4_;
        static const Real x_low_;
        static const Real x_high_;
    };

    // backward compatibility
    typedef InverseCumulativeNormal InvCumulativeNormalDistribution;

    //! Moro Inverse cumulative normal distribution class
    /*! Given x between zero and one as
        the integral value of a gaussian normal distribution
        this class provides the value y such that
        formula here ...

        It uses Beasly and Springer approximation, with an improved
        approximation for the tails. See Boris Moro,
        "The Full Monte", 1995, Risk Magazine.

        This class can also be used to generate a gaussian normal
        distribution from a uniform distribution.
        This is especially useful when a gaussian normal distribution
        is generated from a low discrepancy uniform distribution:
        in this case the traditional Box-Muller approach and its
        variants would not preserve the sequence's low-discrepancy.

        Peter J. Acklam's approximation is better and is available
        as QuantLib::InverseCumulativeNormal
    */
    class MoroInverseCumulativeNormal {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        MoroInverseCumulativeNormal(Real average = 0.0,
                                    Real sigma   = 1.0);
        // function
        Real operator()(Real x) const;
      private:
        Real average_, sigma_;
        static const Real a0_;
        static const Real a1_;
        static const Real a2_;
        static const Real a3_;
        static const Real b0_;
        static const Real b1_;
        static const Real b2_;
        static const Real b3_;
        static const Real c0_;
        static const Real c1_;
        static const Real c2_;
        static const Real c3_;
        static const Real c4_;
        static const Real c5_;
        static const Real c6_;
        static const Real c7_;
        static const Real c8_;
    };

    //! Maddock's Inverse cumulative normal distribution class
    /*! Given x between zero and one as
        the integral value of a gaussian normal distribution
        this class provides the value y such that
        formula here ...

        From the boost documentation:
         These functions use a rational approximation devised by
         John Maddock to calculate an initial approximation to the
         result that is accurate to ~10^-19, then only if that has
         insufficient accuracy compared to the epsilon for type double,
         do we clean up the result using Halley iteration.
    */
    class MaddockInverseCumulativeNormal {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        MaddockInverseCumulativeNormal(Real average = 0.0,
                                       Real sigma   = 1.0);
        Real operator()(Real x) const;

      private:
        const Real average_, sigma_;
    };

    //! Maddock's cumulative normal distribution class
    class MaddockCumulativeNormal {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        MaddockCumulativeNormal(Real average = 0.0,
                                       Real sigma   = 1.0);
        Real operator()(Real x) const;

      private:
        const Real average_, sigma_;
    };


    // inline definitions

    inline NormalDistribution::NormalDistribution(Real average,
                                                  Real sigma)
    : average_(average), sigma_(sigma) {

        QL_REQUIRE(sigma_>0.0,
                   "sigma must be greater than 0.0 ("
                   << sigma_ << " not allowed)");

        normalizationFactor_ = M_SQRT_2*M_1_SQRTPI/sigma_;
        derNormalizationFactor_ = sigma_*sigma_;
        denominator_ = 2.0*derNormalizationFactor_;
    }

    inline Real NormalDistribution::operator()(Real x) const {
        Real deltax = x-average_;
        Real exponent = -(deltax*deltax)/denominator_;
        // debian alpha had some strange problem in the very-low range
        return exponent <= -690.0 ? 0.0 :  // exp(x) < 1.0e-300 anyway
            normalizationFactor_*std::exp(exponent);
    }

    inline Real NormalDistribution::derivative(Real x) const {
        return ((*this)(x) * (average_ - x)) / derNormalizationFactor_;
    }

    inline CumulativeNormalDistribution::CumulativeNormalDistribution(
                                                 Real average, Real sigma)
    : average_(average), sigma_(sigma) {

        QL_REQUIRE(sigma_>0.0,
                   "sigma must be greater than 0.0 ("
                   << sigma_ << " not allowed)");
    }

    inline Real CumulativeNormalDistribution::derivative(Real x) const {
        Real xn = (x - average_) / sigma_;
        return gaussian_(xn) / sigma_;
    }

    inline InverseCumulativeNormal::InverseCumulativeNormal(
                                                 Real average, Real sigma)
    : average_(average), sigma_(sigma) {

        QL_REQUIRE(sigma_>0.0,
                   "sigma must be greater than 0.0 ("
                   << sigma_ << " not allowed)");
    }

    inline MoroInverseCumulativeNormal::MoroInverseCumulativeNormal(
                                                 Real average, Real sigma)
    : average_(average), sigma_(sigma) {

        QL_REQUIRE(sigma_>0.0,
                   "sigma must be greater than 0.0 ("
                   << sigma_ << " not allowed)");
    }

}


#endif


#ifndef id_156d3c2014cb0ac913bd3a8c09cf872e
#define id_156d3c2014cb0ac913bd3a8c09cf872e
inline bool test_156d3c2014cb0ac913bd3a8c09cf872e(int* i) { return i != 0; }
#endif
