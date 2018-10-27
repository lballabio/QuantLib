/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

#include <ql/math/ode/adaptiverungekutta.hpp>
#include <ql/methods/finitedifferences/schemes/methodoflinesscheme.hpp>

#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
#include <ql/bind.hpp>
#if defined(__GNUC__) && (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif

namespace QuantLib {

    MethodOfLinesScheme::MethodOfLinesScheme(
        const Real eps,
        const Real relInitStepSize,
        const ext::shared_ptr<FdmLinearOpComposite> & map,
        const bc_set& bcSet)
    : dt_(Null<Real>()),
      eps_(eps),
      relInitStepSize_(relInitStepSize),
      map_(map),
      bcSet_(bcSet) {
    }


    Disposable<std::vector<Real> >
    MethodOfLinesScheme::apply(Time t, const std::vector<Real>& u) const {
        map_->setTime(t, t + 0.0001);
        bcSet_.applyBeforeApplying(*map_);

        const Array dxdt = -map_->apply(Array(u.begin(), u.end()));

        std::vector<Real> retVal(dxdt.begin(), dxdt.end());
        return retVal;
    }

    void MethodOfLinesScheme::step(array_type& a, Time t) {
        using namespace ext::placeholders;
        QL_REQUIRE(t-dt_ > -1e-8, "a step towards negative time given");

        const std::vector<Real> v =
           AdaptiveRungeKutta<Real>(eps_, relInitStepSize_*dt_)(
               ext::bind(&MethodOfLinesScheme::apply, this, _1, _2),
               std::vector<Real>(a.begin(), a.end()),
               t, std::max(0.0, t-dt_));

        Array y(v.begin(), v.end());

        bcSet_.applyAfterSolving(y);

        a = y;
    }

    void MethodOfLinesScheme::setStep(Time dt) {
        dt_ = dt;
    }
}
