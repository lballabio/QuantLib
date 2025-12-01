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

    struct MultiCurveBootstrapProvider {
        virtual ~MultiCurveBootstrapProvider() = default;
        virtual const MultiCurveBootstrapContributor* multiCurveBootstrapContributor() const = 0;
    };

    /*! MultiCurve builds a set of curves that form a dependency cycle. MultiCurve builds such a
       cycle of curves by using an optimizer specified by one of its constructors. The steps to set
       up the member curves of the cycle is as follows:

       1. Create empty relinkable handles to a YieldTermStructure to represent each member. We call
          these handles 'internal', because they are used internally in the cycle, but not outside
          the cycle.

       2. Construct each member curve as a shared pointer, e.g. by calling

           a) make_shared<PiecewiseYieldCurve<...>>
           b) make_shared<ZeroSpreadedTermStructure>

           Rate helpers in a) or the base curve in b) underlying the spreaded curve should use the
           internal handles from 1. Curves using a bootstrapper as in a) must use a compatible
           boostrap class like GlobalBootstrap.

       3. Construct a MultiCurve instance. This must be a shared pointer.

        4. Add the cycle members to the MultiCurve instance using addBootstrappedCurve() for curves
           using a bootstrapper, as e.g. in a), resp. addNonBootstrappedCurve() for all other
           curves, as e.g. in b).

           Both methods take the internal handle of the curve from 1. and the shared pointer from 2
           as an argument. The latter has to be moved into the function and can not be used
           afterwards.

           Both functions return an external handle to the curve which should be used to reference
           the curve for all other purposes than the internal handle in 2.

           The internal handle is linked to the relevant curve, but the ownership and observability
           is removed to avoid cycles of shared pointers and notification cyclces.

           The external handles are constructed with ownership information shared with the
           MultiCurve instance, which ensures that all member curves are kept alive until none of
           the curves and the MultiCurve instance itself is referenced by any alive object.

        See the piecewise yield curve unit tests for examples. */
    class MultiCurve : public Observer
#ifndef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
    ,
                       public ext::enable_shared_from_this<MultiCurve>
#endif
    {
      public:
        explicit MultiCurve(Real accuracy);
        explicit MultiCurve(ext::shared_ptr<OptimizationMethod> optimizer = nullptr,
                            ext::shared_ptr<EndCriteria> endCriteria = nullptr);

        Handle<YieldTermStructure>
        addBootstrappedCurve(RelinkableHandle<YieldTermStructure>& internalHandle,
                             ext::shared_ptr<YieldTermStructure>&& curve);

        Handle<YieldTermStructure>
        addNonBootstrappedCurve(RelinkableHandle<YieldTermStructure>& internalHandle,
                                ext::shared_ptr<YieldTermStructure>&& curve);

      private:
        Handle<YieldTermStructure> addCurve(RelinkableHandle<YieldTermStructure>& internalHandle,
                                            ext::shared_ptr<YieldTermStructure>&& curve);
        void update() override;
        ext::shared_ptr<MultiCurveBootstrap> multiCurveBootstrap_;
        std::vector<ext::shared_ptr<YieldTermStructure>> curves_;
    };
}

#endif
