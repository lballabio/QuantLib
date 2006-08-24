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


#ifndef quantlib_abcd_hpp
#define quantlib_abcd_hpp

#include <ql/types.hpp>
#include <ql/Math/matrix.hpp>
#include <vector>

namespace QuantLib
{
    //! Abcd functional form for instantaneous volatility
    /*!
        \f[
        f(T-t) = [ a + b(T-t) ] e^{-c(T-t)} + d
        \f]

        following Rebonato notation.

        The class instantiates:
        \f[
        f(T-t) = [ ( a + b(T-t) ) e^{-c(T-t)} + d ]  [ ( a + b(S-t) ) e^{-c(S-t)} + d ]
        \f]


    */
    class Abcd : public std::unary_function<Real,Real> {
    public:
        Abcd(Real a, Real b, Real c, Real d, Real T, Real S);
        Real operator()(Time u) const {
            if(T_<=u) return 0.0;
            if(S_<=u) return 0.0;
            return (  ((a_ + b_*(T_-u))*std::exp(-c_*(T_-u)) + d_ ) *
                      ((a_ + b_*(S_-u))*std::exp(-c_*(S_-u)) + d_ ) );
        }
        Real shortTermVolatility() const;
        Real longTermVolatility() const;
        Real maximumLocation() const;
        Real maximumVolatility() const;
        //! Returns  \f[ \int_{tMin}^{tMax} f(T-t)f(S-t)dt \f]
        Real covariance(Time tMin, Time tMax) const;
        //! Returns  \f[ \int_{0}^{T} f^2(T-t)dt \f]
        Real variance(Time T) const;
        //! Returns the primitive of the indefinite integral \f[ \int f(T-t)f(S-t)dt \f]
        Real primitive(Time u) const {

            if(T_<u) return 0.0;
            if(S_<u) return 0.0;

            const Real k1=std::exp(c_*u);
            const Real k2=std::exp(c_*S_);
            const Real k3=std::exp(c_*T_);

            return (b_*b_*(-1 - 2*c_*c_*S_*T_ - c_*(S_ + T_)
                         + k1*k1*(1 + c_*(S_ + T_ - 2*u) + 2*c_*c_*(S_ - u)*(T_ - u)))
                    + 2*c_*c_*(2*d_*a_*(k2 + k3)*(k1 - 1)
                             +a_*a_*(k1*k1 - 1)+2*c_*d_*d_*k2*k3*u)
                    + 2*b_*c_*(a_*(-1 - c_*(S_ + T_) + k1*k1*(1 + c_*(S_ + T_ - 2*u)))
                             -2*d_*(k3*(1 + c_*S_) + k2*(1 + c_*T_)
                                   - k1*k3*(1 + c_*(S_ - u))
                                   - k1*k2*(1 + c_*(T_ - u)))
                             )
                    ) / (4*c_*c_*c_*k2*k3);
        }

    private:
        //! Parameters
        Real a_, b_, c_, d_;
        //! Expiry times of forwards correnspondings to instantaneous volatility
        Time S_, T_;    
    };

    // inline

    inline Real Abcd::shortTermVolatility() const {
        return a_+d_;
    }

    inline Real Abcd::longTermVolatility() const {
        return d_;
    }

    inline Real Abcd::maximumLocation() const {
        return (b_>0.0 ? (b_-c_*a_)/(c_*b_) : 0.0);
    }

    inline Real Abcd::maximumVolatility() const {
        return (b_>0.0 ? b_/c_*std::exp(-1.0 +c_*a_/b_)+d_ : a_+d_);
    }

}

#endif
