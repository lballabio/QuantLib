/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

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

/*! \file richardsonextrapolation.cpp
*/

#include <ql/errors.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/richardsonextrapolation.hpp>

#include <cmath>

namespace QuantLib {
    namespace {
        class RichardsonEqn {
          public:
            RichardsonEqn(Real fh, Real ft, Real fs, Real t, Real s)
            : fdelta_h_(fh), ft_(ft), fs_(fs), t_(t), s_(s) { }

            Real operator()(Real k) const {
                return      ft_ + (ft_-fdelta_h_)/(std::pow(t_, k)-1.0)
                        - ( fs_ + (fs_-fdelta_h_)/(std::pow(s_, k)-1.0));
            }
          private:
            const Real fdelta_h_, ft_, fs_, t_, s_;
        };

    }

    RichardsonExtrapolation::RichardsonExtrapolation(
        const std::function<Real (Real)>& f, Real delta_h, Real n)
    : delta_h_(delta_h),
      fdelta_h_(f(delta_h)),
      n_(n),
      f_(f) {
    }


    Real RichardsonExtrapolation::operator()(Real t) const {

        QL_REQUIRE(t > 1, "scaling factor must be greater than 1");
        QL_REQUIRE(n_ != Null<Real>(), "order of convergence must be known");

        const Real tk = std::pow(t, n_);

        return (tk*f_(delta_h_/t)-fdelta_h_)/(tk-1.0);
    }

    Real RichardsonExtrapolation::operator()(Real t, Real s)
    const {
        QL_REQUIRE(t > 1 && s > 1, "scaling factors must be greater than 1");
        QL_REQUIRE(t > s, "t must be greater than s");

        const Real ft = f_(delta_h_/t);
        const Real fs = f_(delta_h_/s);

        const RichardsonEqn eqn(fdelta_h_, ft, fs, t, s);

        const Real step = 0.1;
        Real left = 0.05;
        Real fr = eqn(left + step), fl = eqn(left);
        while (fr*fl > 0.0 && left < 15.1) {
            left+=step;
            fl = fr;
            fr = eqn(left + step);
        }

        QL_REQUIRE(left < 15.1,"could not estimate the order of convergence");

        const Real k = Brent().solve(eqn, 1e-8, left+0.5*step, left, left+step);

        const Real ts = std::pow(s, k);

        return (ts*fs-fdelta_h_)/(ts-1.0);
    }
}
