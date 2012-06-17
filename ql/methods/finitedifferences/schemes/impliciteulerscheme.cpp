/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Andreas Gaida
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

#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/methods/finitedifferences/schemes/impliciteulerscheme.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace QuantLib {

    ImplicitEulerScheme::ImplicitEulerScheme(
        const boost::shared_ptr<FdmLinearOpComposite>& map,
        const bc_set& bcSet,
        Real relTol)
    : dt_    (Null<Real>()),
      relTol_(relTol),
      map_   (map),
      bcSet_ (bcSet) {
    }

    Disposable<Array> ImplicitEulerScheme::apply(const Array& r) const {
        return r - dt_*map_->apply(r);
    }

    void ImplicitEulerScheme::step(array_type& a, Time t) {
        QL_REQUIRE(t-dt_ > -1e-8, "a step towards negative time given");
        map_->setTime(std::max(0.0, t-dt_), t);
        bcSet_.setTime(std::max(0.0, t-dt_));

        bcSet_.applyBeforeSolving(*map_, a);

        a = BiCGstab(
                boost::function<Disposable<Array>(const Array&)>(
                    boost::bind(&ImplicitEulerScheme::apply, this, _1)), 
                10*a.size(), relTol_,
                boost::function<Disposable<Array>(const Array&)>(
                    boost::bind(&FdmLinearOpComposite::preconditioner, 
                                map_, _1, -dt_))
            ).solve(a).x;
        
        bcSet_.applyAfterSolving(a);
    }

    void ImplicitEulerScheme::setStep(Time dt) {
        dt_=dt;
    }
}
