/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file bicgstab.cpp
    \brief bi-conjugated gradient stableized algorithm
*/


#include <ql/errors.hpp>
#include <ql/math/matrixutilities/bicgstab.hpp>
#include <utility>

namespace QuantLib {

    BiCGstab::BiCGstab(BiCGstab::MatrixMult A,
                       Size maxIter,
                       Real relTol,
                       BiCGstab::MatrixMult preConditioner)
    : A_(std::move(A)), M_(std::move(preConditioner)), maxIter_(maxIter), relTol_(relTol) {}

    BiCGStabResult BiCGstab::solve(const Array& b, const Array& x0) const {
        Real bnorm2 = Norm2(b);
        if (bnorm2 == 0.0) {
            BiCGStabResult result = { 0, 0.0, b};
            return result;
        }

        Array x = ((!x0.empty()) ? x0 : Array(b.size(), 0.0));
        Array r = b - A_(x);

        Array rTld = r;
        Array p, pTld, v, s, sTld, t;
        Real omega = 1.0;
        Real rho, rhoTld=1.0;
        Real alpha = 0.0, beta;
        Real error = Norm2(r)/bnorm2;

        Size i;
        for (i=0; i < maxIter_ && error >= relTol_; ++i) {
           rho = DotProduct(rTld, r);
           if  (rho == 0.0 || omega == 0.0)
               break;

           if (i != 0U) {
               beta = (rho / rhoTld) * (alpha / omega);
               p = r + beta * (p - omega * v);
           } else {
               p = r;
           }

           pTld = (!M_ ? p : M_(p));
           v     = A_(pTld);

           alpha = rho/DotProduct(rTld, v);
           s     = r-alpha*v;
           if (Norm2(s) < relTol_*bnorm2) {
              x += alpha*pTld;
              error = Norm2(s)/bnorm2;
              break;
           }

           sTld = (!M_ ? s : M_(s));
           t = A_(sTld);
           omega = DotProduct(t,s)/DotProduct(t,t);
           x += alpha*pTld + omega*sTld;
           r = s - omega*t;
           error = Norm2(r)/bnorm2;
           rhoTld = rho;
        }

        QL_REQUIRE(i < maxIter_, "max number of iterations exceeded");
        QL_REQUIRE(error < relTol_, "could not converge");

        BiCGStabResult result = { i, error, x};
        return result;
    }

}
