/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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

/*! \file simplequote.hpp
    \brief simple quote class
*/

#ifndef quantlib_simple_quote_hpp
#define quantlib_simple_quote_hpp

#include <ql/quote.hpp>

namespace QuantLib {

    //! market element returning a stored value
    class SimpleQuote : public Quote {
      public:
        SimpleQuote(Real value = Null<Real>());
        //! \name Quote interface
        //@{
        Real value() const override;
        bool isValid() const override;
        //@}
        //! \name Modifiers
        //@{
        //! returns the difference between the new value and the old value
        Real setValue(Real value = Null<Real>());
        void reset();
        //@}
      private:
        Real value_;
    };

    RelinkableHandle<Quote> handle(Real value);


    // inline definitions

    inline RelinkableHandle<Quote> handle(Real value) {
        return RelinkableHandle<Quote>(ext::make_shared<SimpleQuote>(value));
    }

    inline SimpleQuote::SimpleQuote(Real value)
    : value_(value) {}

    inline Real SimpleQuote::value() const {
        QL_ENSURE(isValid(), "invalid SimpleQuote");
        return value_;
    }

    inline bool SimpleQuote::isValid() const {
        return value_!=Null<Real>();
    }

    inline Real SimpleQuote::setValue(Real value) {
        Real diff = value-value_;
        if (diff != 0.0) {
            value_ = value;
            notifyObservers();
        }
        return diff;
    }

    inline void SimpleQuote::reset() {
        setValue(Null<Real>());
    }

}

#endif
