/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2006 StatPro Italia srl

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

/*! \file ratehelper.hpp
    \brief rate helpers base class
*/

#ifndef quantlib_ratehelper_hpp
#define quantlib_ratehelper_hpp

#include <ql/quote.hpp>
#include <ql/yieldtermstructure.hpp>

namespace QuantLib {

    //! Base class for rate helpers
    /*! This class provides an abstraction for the instruments used to
        bootstrap a term structure.
        It is advised that a rate helper for an instrument contains an
        instance of the actual instrument class to ensure consistancy
        between the algorithms used during bootstrapping and later
        instrument pricing. This is not yet fully enforced in the
        available rate helpers, though - only SwapRateHelper and
        FixedCouponBondHelper contain their corresponding instrument
        for the time being.
    */
    class RateHelper : public Observer, public Observable {
      public:
        RateHelper(const Handle<Quote>& quote);
        RateHelper(Real quote);
        virtual ~RateHelper() {}
        //! \name RateHelper interface
        //@{
        Real quoteError() const;
        Real referenceQuote() const { return quote_->value(); }
        virtual Real impliedQuote() const = 0;
        virtual DiscountFactor discountGuess() const {
            return Null<Real>();
        }
        //! sets the term structure to be used for pricing
        /*! \warning Being a pointer and not a shared_ptr, the term
                     structure is not guaranteed to remain allocated
                     for the whole life of the rate helper. It is
                     responsibility of the programmer to ensure that
                     the pointer remains valid. It is advised that
                     rate helpers be used only in term structure
                     constructors, setting the term structure to
                     <b>this</b>, i.e., the one being constructed.
        */
        virtual void setTermStructure(YieldTermStructure*);
        //! earliest relevant date
        /*! The earliest date at which discounts are needed by the
            helper in order to provide a quote.
        */
        virtual Date earliestDate() const { return earliestDate_;}
        //! latest relevant date
        /*! The latest date at which discounts are needed by the
            helper in order to provide a quote. It does not
            necessarily equal the maturity of the underlying
            instrument.
        */
        virtual Date latestDate() const { return latestDate_;}
        //@}
        //! \name Observer interface
        //@{
        virtual void update() { notifyObservers(); }
        //@}
      protected:
        Handle<Quote> quote_;
        YieldTermStructure* termStructure_;
        Date earliestDate_, latestDate_;
    };

    // helper class
    namespace detail {

        class RateHelperSorter {
          public:
            bool operator()(const boost::shared_ptr<RateHelper>& h1,
                            const boost::shared_ptr<RateHelper>& h2) const {
                return (h1->latestDate() < h2->latestDate());
            }
        };

    }

}


#endif
