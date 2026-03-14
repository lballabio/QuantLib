/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007, 2015 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2015 Paolo Mazzocchi

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

#include <ql/math/abcdmathfunction.hpp>
#include <utility>

namespace QuantLib {

    void AbcdMathFunction::validate(Real a,
                                    Real b,
                                    Real c,
                                    Real d) {
        QL_REQUIRE(c>0, "c (" << c << ") must be positive");
        QL_REQUIRE(d>=0, "d (" << d << ") must be non negative");
        QL_REQUIRE(a+d>=0,
                   "a+d (" << a << "+" << d << ") must be non negative");

        if (b>=0.0)
            return;

        // the one and only stationary point...
        Time zeroFirstDerivative = 1.0/c-a/b;
        if (zeroFirstDerivative>=0.0) {
            // ... is a minimum
            // must be abcd(zeroFirstDerivative)>=0
            QL_REQUIRE(b>=-(d*c)/std::exp(c*a/b-1.0),
                       "b (" << b << ") less than " <<
                       -(d*c)/std::exp(c*a/b-1.0) << ": negative function"
                       " value at stationary point " << zeroFirstDerivative);
        }

    }

    void AbcdMathFunction::initialize_() {
        validate(a_, b_, c_, d_);
        da_ = b_ - c_*a_;
        db_ = -c_*b_;
        dabcd_[0]=da_;
        dabcd_[1]=db_;
        dabcd_[2]=c_;
        dabcd_[3]=0.0;

        pa_ = -(a_ + b_/c_)/c_;
        pb_ = -b_/c_;
        K_ = 0.0;

        dibc_ = b_/c_;
        diacplusbcc_ = a_/c_ + dibc_/c_;
    }

    AbcdMathFunction::AbcdMathFunction(Real aa, Real bb, Real cc, Real dd)
    : a_(aa), b_(bb), c_(cc), d_(dd), abcd_(4), dabcd_(4) {
        abcd_[0]=a_;
        abcd_[1]=b_;
        abcd_[2]=c_;
        abcd_[3]=d_;
        initialize_();
    }

    AbcdMathFunction::AbcdMathFunction(std::vector<Real> abcd) : abcd_(std::move(abcd)), dabcd_(4) {
        a_=abcd_[0];
        b_=abcd_[1];
        c_=abcd_[2];
        d_=abcd_[3];
        initialize_();
    }

    Time AbcdMathFunction::maximumLocation() const {
        if (b_==0.0) {
            if (a_>=0.0)
                return 0.0;
            else
                return QL_MAX_REAL;
        }

        // stationary point
        // TODO check if minimum
        // TODO check if maximum at +inf
        Real zeroFirstDerivative = 1.0/c_-a_/b_;
        return (zeroFirstDerivative>0.0 ? zeroFirstDerivative : 0.0);
    }

    Real AbcdMathFunction::definiteIntegral(Time t1,
                                            Time t2) const {
        return primitive(t2)-primitive(t1);
    }

    std::vector<Real>
    AbcdMathFunction::definiteIntegralCoefficients(Time t,
                                                   Time t2) const {
        Time dt = t2 - t;
        Real expcdt = std::exp(-c_*dt);
        std::vector<Real> result(4);
        result[0] = diacplusbcc_ - (diacplusbcc_ + dibc_*dt)*expcdt;
        result[1] = dibc_ * (1.0 - expcdt);
        result[2] = c_;
        result[3] = d_*dt;
        return result;
    }

    std::vector<Real>
    AbcdMathFunction::definiteDerivativeCoefficients(Time t,
                                                     Time t2) const {
        Time dt = t2 - t;
        Real expcdt = std::exp(-c_*dt);
        std::vector<Real> result(4);
        result[1] = b_*c_/(1.0-expcdt);
        result[0] = a_*c_ - b_ + result[1]*dt*expcdt;
        result[0] /= 1.0-expcdt;
        result[2] = c_;
        result[3] = d_/dt;
        return result;
    }

}
