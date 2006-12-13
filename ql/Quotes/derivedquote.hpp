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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file derivedquote.hpp
*/

#ifndef quantlib_derived_quote_hpp
#define quantlib_derived_quote_hpp

#include <ql/quote.hpp>
#include <ql/types.hpp>
#include <ql/handle.hpp>
#include <ql/errors.hpp>
#include <ql/index.hpp>
#include <ql/option.hpp>

namespace QuantLib {

#ifdef QL_DISABLE_DEPRECATED
    //! market element whose value depends on another market element
    /*! \test the correctness of the returned values is tested by
              checking them against numerical calculations.
    */
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
        QL_REQUIRE(!element_.empty(), "null market element set");
        return f_(element_->value());
    }

    template <class UnaryFunction>
    inline void DerivedQuote<UnaryFunction>::update() {
        notifyObservers();
    }

#endif

    class ForwardValueQuote : public Quote,
                              public Observer {
      public:
        ForwardValueQuote(const boost::shared_ptr<Index>& index,
                          const Date& fixingDate);
        Real value() const;
        void update();
      private:
        boost::shared_ptr<Index> index_;
        Date fixingDate_;
    };

    class ImpliedStdDevQuote : public Quote,
                               public Observer {
      public:
        ImpliedStdDevQuote(Option::Type optionType,
                           const Handle<Quote>& forward,
                           const Handle<Quote>& price,
                           Real strike,
                           Real guess = Null<Real>(),
                           Real accuracy = 1.0e-6);
        Real value() const;
        void update();
      protected:
        mutable Volatility impliedVolatility_;
        Option::Type optionType_;
        Real strike_;
        Real accuracy_;
        Handle<Quote> forward_;
        Handle<Quote> price_;
    };

    class EurodollarFuturesImpliedStdDevQuote : public Quote,
                                                public Observer {
      public:
        EurodollarFuturesImpliedStdDevQuote(const Handle<Quote>& forward,
                                            const Handle<Quote>& callPrice,
                                            const Handle<Quote>& putPrice,
                                            Real strike,
                                            Real guess = Null<Real>(),
                                            Real accuracy = 1.0e-6);
        Real value() const;
        void update();
      protected:
        mutable Volatility impliedVolatility_;
        Real strike_;
        Real accuracy_;
        Handle<Quote> forward_;
        Handle<Quote> callPrice_;
        Handle<Quote> putPrice_;
    };


    // inline

    inline Real ForwardValueQuote::value() const {
        return index_->fixing(fixingDate_);
    }

    inline void ForwardValueQuote::update() {
        notifyObservers();
    }

    inline void ImpliedStdDevQuote::update(){
        notifyObservers();
    }

    inline void EurodollarFuturesImpliedStdDevQuote::update() {
        notifyObservers();
    }
}

#endif
