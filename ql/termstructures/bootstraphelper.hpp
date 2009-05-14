/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2009 Ferdinando Ametrano

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
#include <ql/settings.hpp>

namespace QuantLib {

    //! Base helper class for bootstrapping
    /*! This class provides an abstraction for the instruments used to
        bootstrap a term structure.

        It is advised that a bootstrap helper for an instrument
        contains an instance of the actual instrument class to ensure
        consistancy between the algorithms used during bootstrapping
        and later instrument pricing. This is not yet fully enforced
        in the available bootstrap helpers.
    */
    template <class TS>
    class BootstrapHelper : public Observer, public Observable {
      public:
        BootstrapHelper(const Handle<Quote>& quote);
        BootstrapHelper(Real quote);
        virtual ~BootstrapHelper() {}
        //! \name BootstrapHelper interface
        //@{
        const Handle<Quote>& quote() const { return quote_; }
        virtual Real impliedQuote() const = 0;
        Real quoteError() const { return quote_->value() - impliedQuote(); }
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

    //! Bootstrap helper with date schedule relative to global evaluation date
    /*! Derived classes must takes care of rebuilding the date schedule when
        the global evaluation date changes
    */
    template <class TS>
    class RelativeDateBootstrapHelper : public BootstrapHelper<TS> {
      public:
        RelativeDateBootstrapHelper(const Handle<Quote>& quote);
        RelativeDateBootstrapHelper(Real quote);
        //! \name Observer interface
        //@{
        void update() {
            if (evaluationDate_ != Settings::instance().evaluationDate()) {
                evaluationDate_ = Settings::instance().evaluationDate();
                initializeDates();
            }
            BootstrapHelper<TS>::update();
        }
        //@}
      protected:
        virtual void initializeDates() = 0;
        Date evaluationDate_;
    };

    // template definitions

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

    template <class TS>
    RelativeDateBootstrapHelper<TS>::RelativeDateBootstrapHelper(
                                                    const Handle<Quote>& quote)
    : BootstrapHelper<TS>(quote) {
        this->registerWith(Settings::instance().evaluationDate());
        evaluationDate_ = Settings::instance().evaluationDate();
    }

    template <class TS>
    RelativeDateBootstrapHelper<TS>::RelativeDateBootstrapHelper(Real quote)
    : BootstrapHelper<TS>(quote) {
        this->registerWith(Settings::instance().evaluationDate());
        evaluationDate_ = Settings::instance().evaluationDate();
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
