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
            : fh_(fh), ft_(ft), fs_(fs), t_(t), s_(s) { }

            Real operator()(Real k) const {
                return      ft_ + (ft_-fh_)/(std::pow(t_, k)-1.0)
                        - ( fs_ + (fs_-fh_)/(std::pow(s_, k)-1.0));
            }
          private:
            const Real fh_, ft_, fs_, t_, s_;
        };

    }

    RichardsonExtrapolation::RichardsonExtrapolation(
        Real h, const boost::function<Real (Real)>& f)
    : h_(h), fh_(f(h)), f_(f) { }


    Real RichardsonExtrapolation::formula(Real t, Real k) const {

        QL_REQUIRE(t > 1, "scaling factor must be greater than 1");
        const Real tk = std::pow(t,k);

        return (tk*f_(h_/t)-fh_)/(tk-1.0);
    }

    Real RichardsonExtrapolation::formulaUnknownConvergence(Real t, Real s)
    const {
        QL_REQUIRE(t > 1 && s > 1, "scaling factors must be greater than 1");

        const Real ft = f_(h_/t);
        const Real fs = f_(h_/s);

        const Real k = Brent().solve(RichardsonEqn(fh_, ft, fs, t, s),
                                     1e-8, 0.05, 10);

        const Real ts = std::pow(s,k);

        return (ts*fs-fh_)/(ts-1.0);
    }
}
