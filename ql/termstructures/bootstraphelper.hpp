/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2009, 2015 Ferdinando Ametrano
 Copyright (C) 2015 Paolo Mazzocchi

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

#include <ql/handle.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/quote.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/time/date.hpp>
#include <utility>

namespace QuantLib {

    struct Pillar {
        //! Alternatives ways of determining the pillar date
        enum Choice {
            MaturityDate,     //! instruments maturity date
            LastRelevantDate, //! last date relevant for instrument pricing
            CustomDate        //! custom choice
        };
    };

    std::ostream& operator<<(std::ostream& out, Pillar::Choice type);

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
        explicit BootstrapHelper(Handle<Quote> quote);
        explicit BootstrapHelper(Real quote);
        ~BootstrapHelper() override = default;
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

        //! instrument's maturity date
        virtual Date maturityDate() const;

        //! latest relevant date
        /*! The latest date at which data are needed by the helper
            in order to provide a quote. It does not necessarily
            equal the maturity of the underlying instrument.
        */
        virtual Date latestRelevantDate() const;

        //! pillar date
        virtual Date pillarDate() const;

        //! latest date
        /*! equal to pillarDate()
        */
        virtual Date latestDate() const;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        Handle<Quote> quote_;
        TS* termStructure_;
        Date earliestDate_, latestDate_;
        Date maturityDate_, latestRelevantDate_, pillarDate_;
    };

    //! Bootstrap helper with date schedule relative to global evaluation date
    /*! Derived classes must takes care of rebuilding the date schedule when
        the global evaluation date changes
    */
    template <class TS>
    class RelativeDateBootstrapHelper : public BootstrapHelper<TS> {
      public:
        explicit RelativeDateBootstrapHelper(const Handle<Quote>& quote,
                                             bool updateDates = true);
        explicit RelativeDateBootstrapHelper(Real quote,
                                             bool updateDates = true);
        //! \name Observer interface
        //@{
        void update() override {
            if (updateDates_ && evaluationDate_ != Settings::instance().evaluationDate()) {
                evaluationDate_ = Settings::instance().evaluationDate();
                initializeDates();
            }
            BootstrapHelper<TS>::update();
        }
        //@}
      protected:
        virtual void initializeDates() = 0;
        Date evaluationDate_;
      private:
        bool updateDates_;
    };

    // template definitions

    template <class TS>
    BootstrapHelper<TS>::BootstrapHelper(Handle<Quote> quote)
    : quote_(std::move(quote)), termStructure_(nullptr) {
        registerWith(quote_);
    }

    template <class TS>
    BootstrapHelper<TS>::BootstrapHelper(Real quote)
    : quote_(makeQuoteHandle(quote)), termStructure_(nullptr) {}

    template <class TS>
    void BootstrapHelper<TS>::setTermStructure(TS* t) {
        QL_REQUIRE(t != nullptr, "null term structure given");
        termStructure_ = t;
    }

    template <class TS>
    Date BootstrapHelper<TS>::earliestDate() const {
        return earliestDate_;
    }

    template <class TS>
    Date BootstrapHelper<TS>::maturityDate() const {
        if (maturityDate_ == Date())
            return latestRelevantDate();
        return maturityDate_;
    }

    template <class TS>
    Date BootstrapHelper<TS>::latestRelevantDate() const {
        if (latestRelevantDate_ == Date())
            return latestDate();
        return latestRelevantDate_;
    }

    template <class TS>
    Date BootstrapHelper<TS>::pillarDate() const {
        if (pillarDate_==Date())
            return latestDate();
        return pillarDate_;
    }

    template <class TS>
    Date BootstrapHelper<TS>::latestDate() const {
        if (latestDate_ == Date())
            return pillarDate_;
        return latestDate_;
    }

    template <class TS>
    void BootstrapHelper<TS>::update() {
        notifyObservers();
    }

    template <class TS>
    void BootstrapHelper<TS>::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<BootstrapHelper<TS> >*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            QL_FAIL("not a bootstrap-helper visitor");
    }


    template <class TS>
    RelativeDateBootstrapHelper<TS>::RelativeDateBootstrapHelper(
        const Handle<Quote>& quote, bool updateDates)
    : BootstrapHelper<TS>(quote), updateDates_(updateDates) {
        if (updateDates) {
            this->registerWith(Settings::instance().evaluationDate());
            evaluationDate_ = Settings::instance().evaluationDate();
        }
    }

    template <class TS>
    RelativeDateBootstrapHelper<TS>::RelativeDateBootstrapHelper(
        Real quote, bool updateDates)
    : RelativeDateBootstrapHelper<TS>(makeQuoteHandle(quote), updateDates) {}


    inline std::ostream& operator<<(std::ostream& out,
                                    Pillar::Choice t) {
        switch (t) {
        case Pillar::MaturityDate:
            return out << "MaturityPillarDate";
        case Pillar::LastRelevantDate:
            return out << "LastRelevantPillarDate";
        case Pillar::CustomDate:
            return out << "CustomPillarDate";
        default:
            QL_FAIL("unknown Pillar::Choice(" << Integer(t) << ")");
        }
    }

    namespace detail {

        class BootstrapHelperSorter {
          public:
            template <class Helper>
            bool operator()(
                    const ext::shared_ptr<Helper>& h1,
                    const ext::shared_ptr<Helper>& h2) const {
                return (h1->pillarDate() < h2->pillarDate());
            }
        };

    }

}

#endif
