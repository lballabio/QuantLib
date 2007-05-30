/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
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

/*! \file derivedquote.hpp
    \brief market quote whose value depends on another quote
*/

#ifndef quantlib_derived_quote_hpp
#define quantlib_derived_quote_hpp

#include <ql/quote.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    //! market quote whose value depends on another quote
    /*! \test the correctness of the returned values is tested by
              checking them against numerical calculations.
    */
    template <class UnaryFunction>
    class DerivedQuote : public Quote, public Observer {
      public:
        DerivedQuote(const Handle<Quote>& element,
                     const UnaryFunction& f);
        //! \name Quote interface
        //@{
        Real value() const;
        bool isValid() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      private:
        Handle<Quote> element_;
        UnaryFunction f_;
    };

    // inline definitions
    template <class UnaryFunction>
    inline DerivedQuote<UnaryFunction>::DerivedQuote(
                                                 const Handle<Quote>& element,
                                                 const UnaryFunction& f)
    : element_(element), f_(f) {
        registerWith(element_);
    }

    template <class UnaryFunction>
    inline Real DerivedQuote<UnaryFunction>::value() const {
        QL_ENSURE(isValid(), "invalid DerivedQuote");
        return f_(element_->value());
    }

    template <class UnaryFunction>
    inline bool DerivedQuote<UnaryFunction>::isValid() const {
        return !element_.empty() && element_->isValid();
    }

    template <class UnaryFunction>
    inline void DerivedQuote<UnaryFunction>::update() {
        notifyObservers();
    }

}

#endif
