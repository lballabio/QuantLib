/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006 Giorgio Facchinetti

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

/*! \file futuresconvadjustmentquote.hpp
    \brief quote for the futures-convexity adjustment of an index
*/

#ifndef quantlib_futures_conv_adjustment_quote_hpp
#define quantlib_futures_conv_adjustment_quote_hpp

#include <ql/quote.hpp>
#include <ql/types.hpp>
#include <ql/handle.hpp>
#include <ql/indexes/iborindex.hpp>

namespace QuantLib {

    //! %quote for the futures-convexity adjustment of an index
    class FuturesConvAdjustmentQuote : public Quote,
                                       public Observer {
      public:
        FuturesConvAdjustmentQuote(const boost::shared_ptr<IborIndex>& index,
                                   const Date& futuresDate,
                                   const Handle<Quote>& futuresQuote,
                                   const Handle<Quote>& volatility,
                                   const Handle<Quote>& meanReversion);
        FuturesConvAdjustmentQuote(const boost::shared_ptr<IborIndex>& index,
                                   const std::string& immCode,
                                   const Handle<Quote>& futuresQuote,
                                   const Handle<Quote>& volatility,
                                   const Handle<Quote>& meanReversion);
        //! \name Quote interface
        //@{
        Real value() const;
        bool isValid() const;
        //@}
        void update();
        //! \name Inspectors
        //@{
        Real futuresValue() const { return futuresQuote_->value(); }
        Real volatility() const { return volatility_->value(); }
        Real meanReversion() const { return meanReversion_->value(); }
        Date immDate() const { return futuresDate_; }
        //@}
      protected:
        DayCounter dc_;
        const Date futuresDate_, indexMaturityDate_;
        Handle<Quote> futuresQuote_;
        Handle<Quote> volatility_;
        Handle<Quote> meanReversion_;
    };

    // inline

    inline void FuturesConvAdjustmentQuote::update(){
        notifyObservers();
    }

}

#endif
