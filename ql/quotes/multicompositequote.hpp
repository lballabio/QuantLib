/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Eugene Toder

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_multi_composite_quote_hpp
#define quantlib_multi_composite_quote_hpp

#include <ql/errors.hpp>
#include <ql/handle.hpp>
#include <ql/math/array.hpp>
#include <ql/quote.hpp>
#include <ql/types.hpp>
#include <ql/utilities/null.hpp>
#include <algorithm>
#include <vector>
#include <utility>

namespace QuantLib {

    template <class ArrayFunction>
    class MultiCompositeQuote : public Quote, public Observer {
      public:
        MultiCompositeQuote(std::vector<Handle<Quote>> elements, ArrayFunction f);
        //! \name inspectors
        //@{
        Real inputValue(Size i) const { return elements_.at(i)->value(); }
        //@}
        //! \name Quote interface
        //@{
        Real value() const override;
        bool isValid() const override;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
      private:
        std::vector<Handle<Quote>> elements_;
        mutable Real value_ = Null<Real>();
        ArrayFunction f_;
    };

    // inline definitions

    template <class ArrayFunction>
    inline MultiCompositeQuote<ArrayFunction>::MultiCompositeQuote(
        std::vector<Handle<Quote>> elements,
        ArrayFunction f)
    : elements_(std::move(elements)), f_(std::move(f)) {
        for (auto& elem : elements_)
            registerWith(elem);
    }

    template <class ArrayFunction>
    inline Real MultiCompositeQuote<ArrayFunction>::value() const {
        if (value_ == Null<Real>()) {
            QL_ENSURE(isValid(), "invalid MultiCompositeQuote");
            Array args(elements_.size());
            std::transform(elements_.begin(), elements_.end(), args.begin(),
                           [](const Handle<Quote>& elem) { return elem->value(); });
            value_ = f_(std::move(args));
        }
        return value_;
    }

    template <class ArrayFunction>
    inline bool MultiCompositeQuote<ArrayFunction>::isValid() const {
        return std::all_of(elements_.begin(), elements_.end(), [](const Handle<Quote>& elem) {
            return !elem.empty() && elem->isValid();
        });
    }

    template <class ArrayFunction>
    inline void MultiCompositeQuote<ArrayFunction>::update() {
        value_ = Null<Real>();
        notifyObservers();
    }

}

#endif
