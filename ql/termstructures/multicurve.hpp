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

/*! \file multicurve.hpp
    \brief utility class to manage multi curves
*/

#ifndef quantlib_multicurve_hpp
#define quantlib_multicurve_hpp

#include <ql/handle.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/termstructures/globalbootstrap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/utilities/null_deleter.hpp>

namespace QuantLib {

    class MultiCurve : public Observer {
      public:
        explicit MultiCurve(Real accuracy);
        explicit MultiCurve(ext::shared_ptr<OptimizationMethod> optimizer = nullptr,
                            ext::shared_ptr<EndCriteria> endCriteria = nullptr);

        /* add observer that should be updated during the bootstrap */
        void addBootstrapObserver(Observer* o);

      private:
        template <class Traits, class Interpolator, template <class> class Bootstrap>
        friend class PiecewiseYieldCurve;
        Handle<YieldTermStructure> addCurve(ext::shared_ptr<MultiCurve> multiCurve,
                                            RelinkableHandle<YieldTermStructure>& internalHandle,
                                            ext::shared_ptr<YieldTermStructure> curve,
                                            MultiCurveBootstrapContributor* bootstrap);
        void update() override;
        ext::shared_ptr<MultiCurveBootstrap> multiCurveBootstrap_;
        std::vector<ext::shared_ptr<YieldTermStructure>> curves_;
    };
}

#endif
