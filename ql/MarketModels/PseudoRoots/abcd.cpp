/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MarketModels/PseudoRoots/abcd.hpp>


namespace QuantLib {


    Abcd::Abcd(Real a, Real b, Real c, Real d)
    : a_(a), b_(b), c_(c), d_(d) {
        QL_REQUIRE(a+d>=0, "a+d must be non negative");
        QL_REQUIRE(d>=0, "d must be non negative");
        QL_REQUIRE(c>=0, "c must be non negative");
    }

    Real Abcd::operator()(Time u) const {
        if (u<=0)
            return 0.0;
        else
            return (a_ + b_*u)*std::exp(-c_*u) + d_;
    }

    Real Abcd::instantaneousCovariance(Time u, Time T, Time S) const {
        if (u>T || u>S)
            return 0.0;
        else
            return (*this)(T-u)*(*this)(S-u);
    }

    Real Abcd::covariance(Time t1, Time t2, Time T, Time S) const {
        QL_REQUIRE(t2>=t1, "integrations bounds are in reverse order");
        if (t1>S || t1>T) {
            return 0.0;
        } else {
            t2 = std::min(t2,std::min(S,T));
            return primitive(t2, T, S) - primitive(t1, T, S);
        }
    }

    Real Abcd::variance(Time tMin, Time tMax, Time T) const {
        return covariance(tMin, tMax, T, T);
    }

    Real Abcd::primitive(Time u, Time T, Time S) const {

        if(T<u) return 0.0;
        if(S<u) return 0.0;

        const Real k1=std::exp(c_*u);
        const Real k2=std::exp(c_*S);
        const Real k3=std::exp(c_*T);

        return (b_*b_*(-1 - 2*c_*c_*S*T - c_*(S + T)
                     + k1*k1*(1 + c_*(S + T - 2*u) + 2*c_*c_*(S - u)*(T - u)))
                + 2*c_*c_*(2*d_*a_*(k2 + k3)*(k1 - 1)
                         +a_*a_*(k1*k1 - 1)+2*c_*d_*d_*k2*k3*u)
                + 2*b_*c_*(a_*(-1 - c_*(S + T) + k1*k1*(1 + c_*(S + T - 2*u)))
                         -2*d_*(k3*(1 + c_*S) + k2*(1 + c_*T)
                               - k1*k3*(1 + c_*(S - u))
                               - k1*k2*(1 + c_*(T - u)))
                         )
                ) / (4*c_*c_*c_*k2*k3);
}


}
