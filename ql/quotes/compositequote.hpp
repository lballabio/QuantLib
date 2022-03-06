/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file compositequote.hpp
    \brief purely virtual base class for market observables
*/

#ifndef quantlib_composite_quote_hpp
#define quantlib_composite_quote_hpp

#include <ql/errors.hpp>
#include <ql/handle.hpp>
#include <ql/quote.hpp>
#include <ql/types.hpp>
#include <utility>

namespace QuantLib {

    //! market element whose value depends on two other market element
    /*! \test the correctness of the returned values is tested by
              checking them against numerical calculations.
    */
    template <class BinaryFunction>
    class CompositeQuote : public Quote, public Observer {
      public:
        CompositeQuote(Handle<Quote> element1, Handle<Quote> element2, const BinaryFunction& f);
        //! \name inspectors
        //@{
        Real value1() const { return element1_->value(); }
        Real value2() const { return element2_->value(); }
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
        Handle<Quote> element1_, element2_;
        BinaryFunction f_;
    };

    //! creator method
    template <class BinaryFunction>
    CompositeQuote<BinaryFunction> makeCompositeQuote(const Handle<Quote>& element1,
                                                      const Handle<Quote>& element2,
                                                      const BinaryFunction& f) {
        return CompositeQuote<BinaryFunction>(element1, element2, f);
    }

    // inline definitions

    template <class BinaryFunction>
    inline CompositeQuote<BinaryFunction>::CompositeQuote(Handle<Quote> element1,
                                                          Handle<Quote> element2,
                                                          const BinaryFunction& f)
    : element1_(std::move(element1)), element2_(std::move(element2)), f_(f) {
        registerWith(element1_);
        registerWith(element2_);
    }

    template <class BinaryFunction>
    inline Real CompositeQuote<BinaryFunction>::value() const {
        QL_ENSURE(isValid(), "invalid CompositeQuote");
        return f_(element1_->value(),element2_->value());
    }

    template <class BinaryFunction>
    inline bool CompositeQuote<BinaryFunction>::isValid() const {
        return !element1_.empty()    && !element2_.empty() &&
                element1_->isValid() &&  element2_->isValid();
    }

    template <class BinaryFunction>
    inline void CompositeQuote<BinaryFunction>::update() {
        notifyObservers();
    }

}

#endif


#ifndef id_95fc8c5cbc9897d3826994bb83278bc6
#define id_95fc8c5cbc9897d3826994bb83278bc6
inline bool test_95fc8c5cbc9897d3826994bb83278bc6(int* i) { return i != 0; }
#endif
