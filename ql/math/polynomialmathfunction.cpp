/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Ferdinando Ametrano
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

#include <ql/math/polynomialmathfunction.hpp>
#include <ql/math/pascaltriangle.hpp>

namespace QuantLib {

    PolynomialFunction::PolynomialFunction(const std::vector<Real>& coeff) {

        QL_REQUIRE(!coeff.empty(), "empty coefficient vector");
        order_ = coeff.size();
        c_ = coeff;
        derC_ = std::vector<Real>(order_-1);
        prC_ = std::vector<Real>(order_);
        K_ = 0.0;
        eqs_ = Matrix(order_, order_, 0.0);

        Size i;
        for (i=0; i<order_-1; ++i) {
            prC_[i] = c_[i]/(i+1);
            derC_[i] = c_[i+1]*(i+1);
        }
        prC_[i] = c_[i]/(i + 1);
    }

    Real PolynomialFunction::operator()(Time t) const {
        Real result=0.0, tPower=1.0;
        for (Size i=0; i<order_; ++i) {
            result += c_[i] * tPower;
            tPower *= t;
        }
        return result;
    }

    Real PolynomialFunction::derivative(Time t) const {
        Real result=0.0, tPower=1.0;
        for (Size i=0; i<order_-1; ++i) {
            result += derC_[i] * tPower;
            tPower *= t;
        }
        return result;
    }

    Real PolynomialFunction::primitive(Time t) const {
        Real result=K_, tPower=t;
        for (Size i=0; i<order_; ++i) {
            result += prC_[i] * tPower;
            tPower *= t;
        }
        return result;
    }

    Real PolynomialFunction::definiteIntegral(Time t1,
                                              Time t2) const {
        return primitive(t2)-primitive(t1);
    }

    void PolynomialFunction::initializeEqs_(Time t,
                                            Time t2) const {
        Time dt = t2 - t;
        Real tau;
        for (Size i=0; i<order_; ++i) {
            tau = 1.0;
            for (Size j=i; j<order_; ++j) {
                tau *= dt;
                eqs_[i][j] = (tau * PascalTriangle::get(j + 1)[i]) / (j + 1);
            }
        }
    }

    std::vector<Real> 
         PolynomialFunction::definiteIntegralCoefficients(Time t,
                                                          Time t2) const {
        Array k(c_.begin(), c_.end());
        initializeEqs_(t, t2);
        Array coeff = eqs_ * k;
        std::vector<Real> result(coeff.begin(), coeff.end());
        return result; 
    }

    std::vector<Real>
        PolynomialFunction::definiteDerivativeCoefficients(Time t,
                                                           Time t2) const {
        Array k(c_.begin(), c_.end());
        initializeEqs_(t, t2);
        Array coeff = inverse(eqs_) * k;
        std::vector<Real> result(coeff.begin(), coeff.end());
        return result;
    }

}
