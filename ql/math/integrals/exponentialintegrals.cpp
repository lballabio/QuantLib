/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Klaus Spanderen

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

/*! \file exponentialintegrals.cpp
*/


#include <ql/errors.hpp>
#include <ql/mathconstants.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/integrals/exponentialintegrals.hpp>

#include <boost/math/special_functions/sign.hpp>
#include <cmath>
#include <iostream>
#include <iomanip>

namespace QuantLib {
    namespace exponential_integrals_helper {

        // Reference:
        // Rowe et al: GALSIM: The modular galaxy image simulation toolkit
        // https://arxiv.org/abs/1407.7676

        Real f(Real x) {
            const Real x2 = 1.0/(x*x);

            return (
                1 + x2*(7.44437068161936700618e2 + x2*(1.96396372895146869801e5
                  + x2*(2.37750310125431834034e7 + x2*(1.43073403821274636888e9
                  + x2*(4.33736238870432522765e10 + x2*(6.40533830574022022911e11
                  + x2*(4.20968180571076940208e12 + x2*(1.00795182980368574617e13
                  + x2*(4.94816688199951963482e12 - x2*4.94701168645415959931e11)))))))))
                )/(x *(
                1 + x2*(7.46437068161927678031e2 + x2*(1.97865247031583951450e5
                  + x2*(2.41535670165126845144e7 + x2*(1.47478952192985464958e9
                  + x2*(4.58595115847765779830e10 + x2*(7.08501308149515401563e11
                  + x2*(5.06084464593475076774e12 + x2*(1.43468549171581016479e13
                  + x2*1.11535493509914254097e13))))))))
                ) );
        }

        Real g(Real x) {
            const Real x2 = 1.0/(x*x);

            return x2*(
                1 + x2*(8.1359520115168615e2 + x2*(2.35239181626478200e5
                  + x2*(3.12557570795778731e7 + x2*(2.06297595146763354e9
                  + x2*(6.83052205423625007e10 + x2*(1.09049528450362786e12
                  + x2*(7.57664583257834349e12 + x2*(1.81004487464664575e13
                  + x2*(6.43291613143049485e12 - x2*1.36517137670871689e12)))))))))
                )/(
                1 + x2*(8.19595201151451564e2 + x2*(2.40036752835578777e5
                  + x2*(3.26026661647090822e7 + x2*(2.23355543278099360e9
                  + x2*(7.87465017341829930e10 + x2*(1.39866710696414565e12
                  + x2*(1.17164723371736605e13 + x2*(4.01839087307656620e13
                  + x2*3.99653257887490811e13))))))))
                );
        }
    }

    namespace ExponentialIntegral {
        Real Si(Real x) {
            if (x < 0)
                return -Si(Real(-x));
            else if (x <= 4.0) {
                const Real x2 = x*x;

                return x*
                    ( 1 + x2*(-4.54393409816329991e-2 + x2*(1.15457225751016682e-3
                        + x2*(-1.41018536821330254e-5 + x2*(9.43280809438713025e-8
                        + x2*(-3.53201978997168357e-10 + x2*(7.08240282274875911e-13
                        - x2*6.05338212010422477e-16))))))
                    ) / (
                      1 + x2*(1.01162145739225565e-2 + x2*(4.99175116169755106e-5
                        + x2*(1.55654986308745614e-7 + x2*(3.28067571055789734e-10
                        + x2*(4.5049097575386581e-13 + x2*3.21107051193712168e-16)))))
                    );
            }
            else {
                using namespace exponential_integrals_helper;
                return M_PI_2 - f(x)*std::cos(x) - g(x)*std::sin(x);
            }
        }

        Real Ci(Real x) {
            QL_REQUIRE(x >= 0, "x < 0 => Ci(x) = Ci(-x) + i*pi");

            if (x <= 4.0) {
                const Real x2 = x*x;

                return M_EULER_MASCHERONI + std::log(x) +
                    x2* ( -0.25 + x2*(7.51851524438898291e-3 +x2*(-1.27528342240267686e-4
                                + x2*(1.05297363846239184e-6 +x2*(-4.68889508144848019e-9
                                + x2*(1.06480802891189243e-11 - x2*9.93728488857585407e-15)))))
                    ) / (
                         1 + x2*(1.1592605689110735e-2 + x2*(6.72126800814254432e-5
                           + x2*(2.55533277086129636e-7 + x2*(6.97071295760958946e-10
                           + x2*(1.38536352772778619e-12 + x2*(1.89106054713059759e-15
                           + x2*1.39759616731376855e-18))))))
                    );
            }
            else {
                using namespace exponential_integrals_helper;
                return f(x)*std::sin(x) - g(x)*std::cos(x);
            }
        }

        std::complex<Real> _Ei(
            const std::complex<Real>& z, const std::complex<Real>& acc) {

            if (z.real() == 0.0 && z.imag() == 0.0
                && std::numeric_limits<Real>::has_infinity) {
                return std::complex<Real>(-std::numeric_limits<Real>::infinity());
            }


            constexpr Real DIST = 4.5;
            constexpr Real MAX_ERROR = 5*QL_EPSILON;

            const Real z_inf = std::log(0.01*QL_MAX_REAL) + std::log(100.0);
            QL_REQUIRE(z.real() < z_inf, "argument error " << z);

            const Real z_asym = 2.0 - 1.035*std::log(MAX_ERROR);

            using boost::math::sign;
            const Real abs_z = std::abs(z);

            const auto match = [=](
                const std::complex<Real>& z1, const std::complex<Real>& z2)
                -> bool {
                    const std::complex<Real> d = z1 - z2;
                    return std::abs(d.real()) <= MAX_ERROR*std::abs(z1.real())
                        && std::abs(d.imag()) <= MAX_ERROR*std::abs(z1.imag());
            };

            if (z.real() > z_inf)
                return std::complex<Real>(std::exp(z)/z) + acc;

            if (abs_z > 1.1*z_asym) {
                std::complex<Real> ei = acc + std::complex<Real>(Real(0.0), sign(z.imag())*M_PI);
                std::complex<Real> s(std::exp(z)/z);
                for (Size i=1; i <= std::floor(abs_z)+1; ++i) {
                    if (match(ei+s, ei))
                        return ei+s;
                    ei += s;
                    s *= Real(i)/z;
                }
                QL_FAIL("series conversion issue for Ei(" << z << ")");
            }

            if (abs_z > DIST && (z.real() < 0 || std::abs(z.imag()) > DIST)) {
                std::complex<Real> ei(0.0);
                for (Size k = 47; k >=1; --k) {
                    ei = - Real(k*k)/(2.0*k + 1.0 - z + ei);
                }
                return (acc + std::complex<Real>(0.0, sign(z.imag())*M_PI))
                        - std::exp(z)/ (1.0 - z + ei);

                QL_FAIL("series conversion issue for Ei(" << z << ")");
            }

            std::complex<Real> s(0.0), sn=z;
            Real nn=1.0;

            Size n;
            for (n=2; n < 1000 && s+sn*nn != s; ++n) {
                s+=sn*nn;

                if ((n & 1) != 0U)
                    nn += 1/(2.0*(n/2) + 1); // NOLINT(bugprone-integer-division)

                sn *= -z / Real(2*n);
            }

            QL_REQUIRE(n < 1000, "series conversion issue for Ei(" << z << ")");

            const std::complex<Real> r
                = (M_EULER_MASCHERONI + acc) + std::log(z) + std::exp(0.5*z)*s;

            if (z.imag() != Real(0.0))
                return r;
            else
                return std::complex<Real>(r.real(), acc.imag());
        }

        std::complex<Real> Ei(const std::complex<Real>&z) {
            return _Ei(z, std::complex<Real>(0.0));
        }

        std::complex<Real> E1(const std::complex<Real>& z) {
            if (z.imag() < 0.0) {
                return -_Ei(-z, std::complex<Real>(0.0, -M_PI));
            }
            else if (z.imag() > 0.0 || z.real() < 0.0) {
                return -_Ei(-z, std::complex<Real>(0.0, M_PI));
            }
            else {
                return -Ei(-z);
            }
        }

        // Reference:
        // https://functions.wolfram.com/GammaBetaErf/ExpIntegralEi/introductions/ExpIntegrals/ShowAll.html
        std::complex<Real> Si(const std::complex<Real>& z) {
            if (std::abs(z) <= 0.2) {
                std::complex<Real> s(0), nn(z);
                Size k;
                for (k=2; k < 100 && s != s+nn; ++k) {
                    s += nn;
                    nn *= -z*z/((2.0*k-2)*(2*k-1)*(2*k-1))*(2.0*k-3);
                }
                QL_REQUIRE(k < 100, "series conversion issue for Si(" << z << ")");

                return s;
            }
            else {
                const std::complex<Real> i(0.0, 1.0);
                return 0.5*i*(E1(-i*z) - E1(i*z)
                        - std::complex<Real>(0.0, ((z.real() >= 0 && z.imag() >= 0)
                                || (z.real() > 0 && z.imag() < 0) )? M_PI : -M_PI));
            }
        }

        std::complex<Real> Ci(const std::complex<Real>& z) {
            const std::complex<Real> i(0.0, 1.0);

            std::complex<Real> acc(0.0);
            if (z.real() < 0.0 && z.imag() >= 0.0)
                acc.imag(M_PI);
            else if (z.real() <= 0.0 && z.imag() <= 0.0)
                acc.imag(-M_PI);

            return -0.5*(E1(-i*z) + E1(i*z)) + acc;
        }
    }
}
