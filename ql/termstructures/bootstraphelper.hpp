/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007 Ferdinando Ametrano

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

/*! \file bootstraphelper.hpp
    \brief base helper class used for bootstrapping
*/

#ifndef quantlib_bootstrap_helper_hpp
#define quantlib_bootstrap_helper_hpp

#include <ql/quote.hpp>
#include <ql/time/date.hpp>
#include <ql/handle.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/quotes/simplequote.hpp>

namespace QuantLib {

    //! Base helper class for bootstrapping
    /*! This class provides an abstraction for the instruments used to
        bootstrap a term structure.

        It is advised that a bootstrap helper for an instrument
        contains an instance of the actual instrument class to ensure
        consistancy between the algorithms used during bootstrapping
        and later instrument pricing. This is not yet fully enforced
        in the available rate helpers.
    */
    template <class TS>
    class BootstrapHelper : public Observer, public Observable {
      public:
        BootstrapHelper(const Handle<Quote>& quote);
        BootstrapHelper(Real quote);
        virtual ~BootstrapHelper() {}
        //! \name BootstrapHelper interface
        //@{
        Real quoteError() const;
        Real quoteValue() const;
        bool quoteIsValid() const;
        virtual Real impliedQuote() const = 0;
        //! sets the term structure to be used for pricing
        /*! \warning Being a pointer and not a shared_ptr, the term
                     structure is not guaranteed to remain allocated
                     for the whole life of the rate helper. It is
                     responsibility of the programmer to ensure that
                     the pointer remains valid. It is advised that
                     this method is called only inside the term
                     structure being bootstrapped, setting the pointer
                     to <b>this</b>, i.e., the term structure itself.
        */
        virtual void setTermStructure(TS*);
        //! earliest relevant date
        /*! The earliest date at which data are needed by the
            helper in order to provide a quote.
        */
        virtual Date earliestDate() const;
        //! latest relevant date
        /*! The latest date at which data are needed by the
            helper in order to provide a quote. It does not
            necessarily equal the maturity of the underlying
            instrument.
        */
        virtual Date latestDate() const;
        //@}
        //! \name Observer interface
        //@{
        virtual void update();
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        Handle<Quote> quote_;
        TS* termStructure_;
        Date earliestDate_, latestDate_;
    };


    template <class TS>
    BootstrapHelper<TS>::BootstrapHelper(const Handle<Quote>& quote)
    : quote_(quote), termStructure_(0) {
        registerWith(quote_);
    }

    template <class TS>
    BootstrapHelper<TS>::BootstrapHelper(Real quote)
    : quote_(Handle<Quote>(boost::shared_ptr<Quote>(new SimpleQuote(quote)))),
      termStructure_(0) {}

    template <class TS>
    Real BootstrapHelper<TS>::quoteError() const {
        return quote_->value()-impliedQuote();
    }

    template <class TS>
    Real BootstrapHelper<TS>::quoteValue() const {
        return quote_->value();
    }

    template <class TS>
    bool BootstrapHelper<TS>::quoteIsValid() const {
        return quote_->isValid();
    }

    template <class TS>
    void BootstrapHelper<TS>::setTermStructure(TS* t) {
        QL_REQUIRE(t != 0, "null term structure given");
        termStructure_ = t;
    }

    template <class TS>
    Date BootstrapHelper<TS>::earliestDate() const {
        return earliestDate_;
    }

    template <class TS>
    Date BootstrapHelper<TS>::latestDate() const {
        return latestDate_;
    }

    template <class TS>
    void BootstrapHelper<TS>::update() {
        notifyObservers();
    }

    template <class TS>
    void BootstrapHelper<TS>::accept(AcyclicVisitor& v) {
        Visitor<BootstrapHelper<TS> >* v1 =
            dynamic_cast<Visitor<BootstrapHelper<TS> >*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("not a bootstrap-helper visitor");
    }


    namespace detail {

        class BootstrapHelperSorter {
          public:
            template <class Helper>
            bool operator()(
                    const boost::shared_ptr<Helper>& h1,
                    const boost::shared_ptr<Helper>& h2) const {
                return (h1->latestDate() < h2->latestDate());
            }
        };

    }

}

#endif
