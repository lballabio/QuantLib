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

    */
    class Abcd : public std::unary_function<Real,Real> {
    public:
        Abcd(Real a, Real b, Real c, Real d);
        //! instantaneous volatility at time to maturity u: \f[ f(u) \f]
        Real operator()(Time u) const;
        /*! instantaneous covariance at time u between T and S fixing rates:
            \f[ f(T-u)f(S-u) \f]
        */
        Real instantaneousCovariance(Time u, Time T, Time S) const;
        /*! covariance in [tMin,tMax] between T and S fixing rates:
            \f[ \int_{tMin}^{tMax} f(T-u)f(S-u)du \f]
        */
        Real covariance(Time tMin, Time tMax, Time T, Time S) const;
        /*! variance in [tMin,tMax] of T fixing rate:
            \f[ \int_{tMin}^{tMax} f^2(T-u)du \f]
        */
        Real variance(Time tMin, Time tMax, Time T) const;
        //! instantaneous volatility when time to maturity = 0.0
        Real shortTermVolatility() const;
        //! instantaneous volatility when time to maturity = +inf
        Real longTermVolatility() const;
        //! time to maturity at which the instantaneous volatility reaches maximum (if any)
        Real maximumLocation() const;
        //! maximum of the instantaneous volatility
        Real maximumVolatility() const;
    private:
        //! indefinite integral \f[ \int f(T-t)f(S-t)dt \f]
        Real primitive(Time u, Time T, Time S) const;
        //! Parameters
        Real a_, b_, c_, d_;
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

    //! Abcd Squared functional. Used only in test-suite.
    class AbcdSquared : public std::unary_function<Real,Real> {
    public:  
        AbcdSquared(Real a, Real b, Real c, Real d, Time S, Time T)
        : abcd_(new Abcd(a,b,c,d)), S_(S), T_(T) {
            QL_REQUIRE(a+d>=0, "a+d must be non negative");
            QL_REQUIRE(d>=0, "d must be non negative");
            QL_REQUIRE(c>=0, "c must be non negative");
        }
        Real operator()(Time u) const {
            if (u>T_ || u>S_)
                return 0.0;
            else
                return (*abcd_)(T_-u)*(*abcd_)(S_-u);        
        }

    private:
        //! Parameters
        boost::shared_ptr<Abcd>  abcd_;
        Time S_, T_;

    };

}

#endif
