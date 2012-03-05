/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file inflationhelpers.hpp
 \brief Bootstrap helpers for inflation term structures
 */

#ifndef quantlib_inflation_helpers_hpp
#define quantlib_inflation_helpers_hpp

#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/instruments/yearonyearinflationswap.hpp>

namespace QuantLib {

    //! Zero-coupon inflation-swap bootstrap helper
    class ZeroCouponInflationSwapHelper
    : public BootstrapHelper<ZeroInflationTermStructure> {
    public:
        ZeroCouponInflationSwapHelper(
            const Handle<Quote>& quote,
            const Period& swapObsLag,   // lag on swap observation of index
            const Date& maturity,
            const Calendar& calendar,   // index may have null calendar as valid on every day
            BusinessDayConvention paymentConvention,
            const DayCounter& dayCounter,
            const boost::shared_ptr<ZeroInflationIndex>& zii);

        void setTermStructure(ZeroInflationTermStructure*);
        Real impliedQuote() const;
    protected:
        Period swapObsLag_;
        Date maturity_;
        Calendar calendar_;
        BusinessDayConvention paymentConvention_;
        DayCounter dayCounter_;
        boost::shared_ptr<ZeroInflationIndex> zii_;
        boost::shared_ptr<ZeroCouponInflationSwap> zciis_;
    };


    //! Year-on-year inflation-swap bootstrap helper
    class YearOnYearInflationSwapHelper
    : public BootstrapHelper<YoYInflationTermStructure> {
    public:
        YearOnYearInflationSwapHelper(const Handle<Quote>& quote,
                                      const Period& swapObsLag_,
                                      const Date& maturity,
                                      const Calendar& calendar,
                                      BusinessDayConvention paymentConvention,
                                      const DayCounter& dayCounter,
                                      const boost::shared_ptr<YoYInflationIndex>& yii);

        void setTermStructure(YoYInflationTermStructure*);
        Real impliedQuote() const;
    protected:
        Period swapObsLag_;
        Date maturity_;
        Calendar calendar_;
        BusinessDayConvention paymentConvention_;
        DayCounter dayCounter_;
        boost::shared_ptr<YoYInflationIndex> yii_;
        boost::shared_ptr<YearOnYearInflationSwap> yyiis_;
    };

}


#endif
