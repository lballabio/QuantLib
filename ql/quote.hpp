
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file quote.hpp
    \brief purely virtual base class for market observables
*/

#ifndef quantlib_quote_hpp
#define quantlib_quote_hpp

#include <ql/relinkablehandle.hpp>

namespace QuantLib {

    //! purely virtual base class for market observables
    class Quote : public Observable {
      public:
        virtual ~Quote() {}
        //! returns the current value
        virtual Real value() const = 0;
    };


    //! market element returning a stored value
    class SimpleQuote : public Quote {
      public:
        SimpleQuote(Real value);
        //! \name Quote interface
        //@{
        Real value() const;
        //@}
        //! \name Modifiers
        //@{
        void setValue(Real value);
        //@}
      private:
        Real value_;
    };


    //! market element whose value depends on another market element
    template <class UnaryFunction>
    class DerivedQuote : public Quote, public Observer {
      public:
        DerivedQuote(const Handle<Quote>& element,
                     const UnaryFunction& f);
        //! \name Market element interface
        //@{
        Real value() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      private:
        Handle<Quote> element_;
        UnaryFunction f_;
    };


    //! market element whose value depends on two other market element
    template <class BinaryFunction>
    class CompositeQuote : public Quote, public Observer {
      public:
        CompositeQuote(
            const Handle<Quote>& element1,
            const Handle<Quote>& element2,
            const BinaryFunction& f);
        //! \name Quote interface
        //@{
        Real value() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      private:
        Handle<Quote> element1_, element2_;
        BinaryFunction f_;
    };


    // inline definitions

    // simple quote

    inline SimpleQuote::SimpleQuote(Real value)
    : value_(value) {}

    inline Real SimpleQuote::value() const {
        return value_;
    }

    inline void SimpleQuote::setValue(Real value) {
        if (value_ == value)
            return;
        value_ = value;
        notifyObservers();
    }


    // derived quote

    template <class UnaryFunction>
    inline DerivedQuote<UnaryFunction>::DerivedQuote(
                                                 const Handle<Quote>& element,
                                                 const UnaryFunction& f)
    : element_(element), f_(f) {
        registerWith(element_);
    }

    template <class UnaryFunction>
    inline Real DerivedQuote<UnaryFunction>::value() const {
        QL_REQUIRE(!element_.empty(), "null market element set");
        return f_(element_->value());
    }

    template <class UnaryFunction>
    inline void DerivedQuote<UnaryFunction>::update() {
        notifyObservers();
    }


    // composite quote

    template <class BinaryFunction>
    inline CompositeQuote<BinaryFunction>::CompositeQuote(
                                                const Handle<Quote>& element1,
                                                const Handle<Quote>& element2,
                                                const BinaryFunction& f)
    : element1_(element1), element2_(element2), f_(f) {
        registerWith(element1_);
        registerWith(element2_);
    }

    template <class BinaryFunction>
    inline Real CompositeQuote<BinaryFunction>::value() const {
        QL_REQUIRE(!element1_.empty() && !element2_.empty(),
                   "null quote set");
        return f_(element1_->value(),element2_->value());
    }

    template <class BinaryFunction>
    inline void CompositeQuote<BinaryFunction>::update() {
        notifyObservers();
    }

}


#endif

