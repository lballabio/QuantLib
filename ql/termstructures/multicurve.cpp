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

    MultiCurve::MultiCurve(ext::shared_ptr<MultiCurveBootstrap> multiCurveBootstrap)
    : multiCurveBootstrap_(std::move(multiCurveBootstrap)) {}

    Handle<YieldTermStructure>
    MultiCurve::addCurve(RelinkableHandle<YieldTermStructure>& internalHandle,
                         ext::shared_ptr<YieldTermStructure> curve,
                         const MultiCurveBootstrapContributor* bootstrap) {

        QL_REQUIRE(internalHandle.empty(),
                   "internal handle must be empty; was the curve added already?");
        QL_REQUIRE(curve != nullptr, "curve must not be null");

        multiCurveBootstrap_->add(bootstrap);

        internalHandle.linkTo(ext::shared_ptr<YieldTermStructure>(curve.get(), null_deleter()),
                              false);
        Handle<YieldTermStructure> externalHandle(
            ext::shared_ptr<YieldTermStructure>(shared_from_this(), curve.get()));

        curves_.push_back({std::move(curve), ext::make_shared<Updater>()});

        curves_.back().updater->addObservable(curves_.back().ptr);

        for(Size i=0;i<curves_.size()-1;++i) {
            curves_.back().updater->addObserver(curves_[i].ptr);
            curves_[i].updater->addObserver(curves_.back().ptr);
        }

        return externalHandle;
    }

    void MultiCurve::Updater::addObservable(const ext::shared_ptr<Observable>& curve) {
        QL_REQUIRE(curve != nullptr, "Updater::addObservable(): got null curve");
        registerWith(curve);
    }

    void MultiCurve::Updater::addObserver(ext::shared_ptr<Observer> curve) {
        QL_REQUIRE(curve != nullptr, "Updater::addObserver(): got null curve");
        observers_.push_back(std::move(curve));
    }

    void MultiCurve::Updater::update() {
        for (auto const& c : observers_)
            c->update();
    }

}
