/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Eugene Toder
 Copyright (C) 2025 Peter Caspers

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

#include <ql/termstructures/multicurve.hpp>

namespace QuantLib {

    MultiCurve::MultiCurve(Real accuracy)
    : multiCurveBootstrap_(ext::make_shared<MultiCurveBootstrap>(accuracy)) {}

    MultiCurve::MultiCurve(ext::shared_ptr<OptimizationMethod> optimizer,
                           ext::shared_ptr<EndCriteria> endCriteria)
    : multiCurveBootstrap_(ext::make_shared<MultiCurveBootstrap>(optimizer, endCriteria)) {}

    Handle<YieldTermStructure>
    MultiCurve::addBootstrappedCurve(RelinkableHandle<YieldTermStructure>& internalHandle,
                                     ext::shared_ptr<YieldTermStructure>&& curve) {
        QL_REQUIRE(internalHandle.empty(),
                   "internal handle must be empty; was the curve added already?");
        auto mcProv = ext::dynamic_pointer_cast<MultiCurveBootstrapProvider>(curve);
        QL_REQUIRE(mcProv != nullptr, "curve must not be a MultiCurveBootstrapProvider");
        auto bootstrap = mcProv->multiCurveBootstrapContributor();
        QL_REQUIRE(bootstrap, "curve does not provide a valid multi curve bootstrap contributor");
        multiCurveBootstrap_->add(bootstrap);
        return addCurve(internalHandle, std::move(curve));
    }

    Handle<YieldTermStructure>
    MultiCurve::addNonBootstrappedCurve(RelinkableHandle<YieldTermStructure>& internalHandle,
                                        ext::shared_ptr<YieldTermStructure>&& curve) {
        QL_REQUIRE(internalHandle.empty(),
                   "internal handle must be empty; was the curve added already?");
        QL_REQUIRE(curve != nullptr, "curve must not be null");
        multiCurveBootstrap_->addObserver(curve.get());
        return addCurve(internalHandle, std::move(curve));
    }

    Handle<YieldTermStructure>
    MultiCurve::addCurve(RelinkableHandle<YieldTermStructure>& internalHandle,
                         ext::shared_ptr<YieldTermStructure>&& curve) {
        internalHandle.linkTo(ext::shared_ptr<YieldTermStructure>(curve.get(), null_deleter()),
                              false);
        Handle<YieldTermStructure> externalHandle(ext::shared_ptr<YieldTermStructure>(
#ifdef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
            ext::static_pointer_cast<MultiCurve>(shared_from_this())
#else
            shared_from_this()
#endif
                ,
            curve.get()));
        registerWithObservables(curve);
        curves_.push_back(curve);
        return externalHandle;
    }

    void MultiCurve::update() {
        for (auto const& c : curves_)
            c->update();
    }

}
