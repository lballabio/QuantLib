

/*
 Copyright (C) 2002 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file normaldistribution.hpp
    \brief normal, cumulative and inverse cumulative distributions

    \fullpath
    ql/Math/%normaldistribution.hpp
*/

// $Id$

#ifndef quantlib_normal_distribution_h
#define quantlib_normal_distribution_h

#include <ql/dataformatters.hpp>
#include <functional>

namespace QuantLib {

    namespace Math {

        //! Normal distribution class
        /*! formula here ...
            Given x it returns its probability in a gaussian
            normal distribution.
            It provides the first derivative too.
        */
        class NormalDistribution
        : public std::unary_function<double,double> {
          public:
            NormalDistribution(double average = 0.0,
                               double sigma = 1.0);
            // function
            double operator()(double x) const;
            double derivative(double x) const;
          private:
            static const double pi_;
            double average_, sigma_, normalizationFactor_, denominator_;
        };

        typedef NormalDistribution GaussianDistribution;


        //! Cumulative normal distribution class
        /*! Given x it provides an approximation to the
            integral of the gaussian normal distribution:
            formula here ...

            For this implementation see M. Abramowitz and I. Stegun,
            Handbook of Mathematical Functions,
            Dover Publications, New York (1972)
        */
        class CumulativeNormalDistribution
        : public std::unary_function<double,double> {
          public:
            CumulativeNormalDistribution(double average = 0.0,
                                         double sigma   = 1.0);
            // function
            double operator()(double x) const;
            double derivative(double x) const;
          private:
            double average_, sigma_;
            static const double a1_;
            static const double a2_;
            static const double a3_;
            static const double a4_;
            static const double a5_;
            static const double gamma_;
            static const double precision_;
            NormalDistribution gaussian_;
        };


        //! Inverse cumulative normal distribution class
        /*! Given x between zero and one as
            the integral value of a gaussian normal distribution
            this class provides the value y such that
            formula here ...

            For this implementation see Boris Moro,
            "The Full Monte", 1995, Risk Magazine.

            This class can also be used to generate a gaussian normal
            distribution from a uniform distribution.
            This is especially useful when a gaussian normal distribution
            is generated from a quasi-random uniform distribution: in this
            case the traditional Box-Muller approach and its variants
            would not preserve the sequence's low-discrepancy.
        */
        class InvCumulativeNormalDistribution
        : public std::unary_function<double,double> {
          public:
            InvCumulativeNormalDistribution(double average = 0.0,
                                            double sigma   = 1.0);
            // function
            double operator()(double x) const;
          private:
            double average_, sigma_;
            static const double a0_;
            static const double a1_;
            static const double a2_;
            static const double a3_;
            static const double b0_;
            static const double b1_;
            static const double b2_;
            static const double b3_;
            static const double c0_;
            static const double c1_;
            static const double c2_;
            static const double c3_;
            static const double c4_;
            static const double c5_;
            static const double c6_;
            static const double c7_;
            static const double c8_;
        };


        //! Inverse cumulative normal distribution class
        /*! Given x between zero and one as
            the integral value of a gaussian normal distribution
            this class provides the value y such that
            formula here ...

            Anyone able to identify the algorithm used in this implementation?
            It might be Hill and Davis (1973), or Odeh and Evans (1974), or
		    Beasley and Springer (1977)
        */
        class InvCumulativeNormalDistribution2
        : public std::unary_function<double,double> {
          public:
            InvCumulativeNormalDistribution2(double average = 0.0,
                                             double sigma   = 1.0);
            // function
            double operator()(double x) const;
          private:
            double average_, sigma_;
            static const double p0_;
            static const double p1_;
            static const double p2_;
            static const double q1_;
            static const double q2_;
            static const double q3_;
        };

        // inline definitions

        inline NormalDistribution::NormalDistribution(double average,
            double sigma)
        : average_(average), sigma_(sigma) {

            QL_REQUIRE(sigma_>0.0,
                "NormalDistribution: sigma must be greater than 0.0 (" +
                    DoubleFormatter::toString(sigma_) + " not allowed)");

            normalizationFactor_ = 1.0/(sigma_*QL_SQRT(2.0*pi_));
            denominator_ = 2.0*sigma_*sigma_;
        }

        inline double NormalDistribution::operator()(double x) const {
            double deltax = x-average_;
            double exponent = -deltax*deltax/denominator_;
            // debian alpha had some strange problem in the very-low range
            return exponent <= -690.0 ? 0.0 :  // exp(x) < 1.0e-300 anyway
                                        normalizationFactor_*QL_EXP(exponent);
        }

        inline double NormalDistribution::derivative(double x) const {
            return (*this)(x) * (average_ - x) / sigma_;
        }

        inline CumulativeNormalDistribution::CumulativeNormalDistribution(
            double average, double sigma)
        : average_(average), sigma_(sigma) {

            QL_REQUIRE(sigma_>0.0,
                "CumulativeNormalDistribution: "
                    "sigma must be greater than 0.0 (" +
                    DoubleFormatter::toString(sigma_) + " not allowed)");
        }

        inline double CumulativeNormalDistribution::derivative(double x) const {
            double xn = (x - average_) / sigma_;
            return gaussian_(xn) / sigma_;
        }

        inline InvCumulativeNormalDistribution::InvCumulativeNormalDistribution(
            double average, double sigma)
        : average_(average), sigma_(sigma) {

            QL_REQUIRE(sigma_>0.0,
              "InvCumulativeNormalDistribution: "
                "sigma must be greater than 0.0 (" +
                DoubleFormatter::toString(sigma_) + " not allowed)");
        }

        inline InvCumulativeNormalDistribution2::InvCumulativeNormalDistribution2(
            double average, double sigma)
        : average_(average), sigma_(sigma) {

            QL_REQUIRE(sigma_>0.0,
              "InvCumulativeNormalDistribution2: "
                "sigma must be greater than 0.0 (" +
                DoubleFormatter::toString(sigma_) + " not allowed)");
        }

    }

}


#endif
