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

    struct MultiCurveImpl : public Observer, public Observable {};

    class MultiCurve : public MultiCurveImpl, public ext::enable_shared_from_this<MultiCurve> {
      public:
        explicit MultiCurve(ext::shared_ptr<MultiCurveBootstrap> multiCurveBootstrap);
        /* addCurve() takes an internal handle and returns an external handle.
           Internal handle, which must be an empty RelinkableHandle, should be
           used within the cycle. External handle should be used outside of the
           cycle. */
        Handle<YieldTermStructure> addCurve(RelinkableHandle<YieldTermStructure>& internalHandle,
                                            ext::shared_ptr<YieldTermStructure> curve);


      private:
        void update() override;
        std::vector<ext::shared_ptr<YieldTermStructure>> curves_;
        ext::shared_ptr<MultiCurveBootstrap> multiCurveBootstrap_;
    };
}

#endif
