/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/distributions/gammadistribution.hpp>
#include <boost/math/special_functions/gamma.hpp>

namespace QuantLib {

    Real CumulativeGammaDistribution::operator()(Real x) const {
        if (x <= 0.0) return 0.0;

        return boost::math::gamma_p(a_, x);
    }

    const Real GammaFunction::c1_ = 76.18009172947146;
    const Real GammaFunction::c2_ = -86.50532032941677;
    const Real GammaFunction::c3_ = 24.01409824083091;
    const Real GammaFunction::c4_ = -1.231739572450155;
    const Real GammaFunction::c5_ = 0.1208650973866179e-2;
    const Real GammaFunction::c6_ = -0.5395239384953e-5;

    Real GammaFunction::logValue(Real x) const {
        QL_REQUIRE(x>0.0, "positive argument required");
        Real temp = x + 5.5;
        temp -= (x + 0.5)*std::log(temp);
        Real ser=1.000000000190015;
        ser += c1_/(x + 1.0);
        ser += c2_/(x + 2.0);
        ser += c3_/(x + 3.0);
        ser += c4_/(x + 4.0);
        ser += c5_/(x + 5.0);
        ser += c6_/(x + 6.0);

        return -temp+std::log(2.5066282746310005*ser/x);
    }

    Real GammaFunction::value(Real x) const {
        if (x >= 1.0) {
            return std::exp(logValue(x));
        }
        else {
            if (x > -20.0) {
                // \Gamma(x) = \frac{\Gamma(x+1)}{x}
                return value(x+1.0)/x;
            }
            else {
                // \Gamma(-x) = -\frac{\pi}{\Gamma(x)\sin(\pi x) x}
                return -M_PI/(value(-x)*x*std::sin(M_PI*x));
            }
        }
    }
}
