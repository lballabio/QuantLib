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

#include <ql/instruments/yearonyearinflationswap.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>

namespace QuantLib {

    //! Zero-coupon inflation-swap bootstrap helper
    class ZeroCouponInflationSwapHelper : public BootstrapHelper<ZeroInflationTermStructure> {
      public:
        ZeroCouponInflationSwapHelper(
            const Handle<Quote>& quote,
            const Period& swapObsLag, // lag on swap observation of index
            const Date& maturity,
            Calendar calendar, // index may have null calendar as valid on every day
            BusinessDayConvention paymentConvention,
            const DayCounter& dayCounter,
            ext::shared_ptr<ZeroInflationIndex> zii,
            CPI::InterpolationType observationInterpolation);

        /*! \deprecated Use the overload that does not take a nominal curve.
                        Deprecated in version 1.39.
        */
        [[deprecated("Use the overload that does not take a nominal curve.")]]
        ZeroCouponInflationSwapHelper(
            const Handle<Quote>& quote,
            const Period& swapObsLag,
            const Date& maturity,
            Calendar calendar,
            BusinessDayConvention paymentConvention,
            DayCounter dayCounter,
            ext::shared_ptr<ZeroInflationIndex> zii,
            CPI::InterpolationType observationInterpolation,
            Handle<YieldTermStructure> nominalTermStructure);

        void setTermStructure(ZeroInflationTermStructure*) override;
        Real impliedQuote() const override;
        //! \name inspectors
        //@{
        // NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,performance-noexcept-swap)
        ext::shared_ptr<ZeroCouponInflationSwap> swap() const { return zciis_; }
        //@}
      protected:
        Period swapObsLag_;
        Date maturity_;
        Calendar calendar_;
        BusinessDayConvention paymentConvention_;
        DayCounter dayCounter_;
        ext::shared_ptr<ZeroInflationIndex> zii_;
        CPI::InterpolationType observationInterpolation_;
        ext::shared_ptr<ZeroCouponInflationSwap> zciis_;
        Handle<YieldTermStructure> nominalTermStructure_;
    };


    //! Year-on-year inflation-swap bootstrap helper
    class YearOnYearInflationSwapHelper : public BootstrapHelper<YoYInflationTermStructure> {
      public:
        YearOnYearInflationSwapHelper(const Handle<Quote>& quote,
                                      const Period& swapObsLag_,
                                      const Date& maturity,
                                      Calendar calendar,
                                      BusinessDayConvention paymentConvention,
                                      DayCounter dayCounter,
                                      ext::shared_ptr<YoYInflationIndex> yii,
                                      CPI::InterpolationType interpolation,
                                      Handle<YieldTermStructure> nominalTermStructure);

        /*! \deprecated Use the overload that passes an interpolation type instead.
                        Deprecated in version 1.36.
        */
        [[deprecated("Use the overload that passes an interpolation type instead")]]
        YearOnYearInflationSwapHelper(const Handle<Quote>& quote,
                                      const Period& swapObsLag_,
                                      const Date& maturity,
                                      Calendar calendar,
                                      BusinessDayConvention paymentConvention,
                                      DayCounter dayCounter,
                                      ext::shared_ptr<YoYInflationIndex> yii,
                                      Handle<YieldTermStructure> nominalTermStructure);

        void setTermStructure(YoYInflationTermStructure*) override;
        Real impliedQuote() const override;
        //! \name inspectors
        //@{
        // NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,performance-noexcept-swap)
        ext::shared_ptr<YearOnYearInflationSwap> swap() const { return yyiis_; }
        //@}
      protected:
        Period swapObsLag_;
        Date maturity_;
        Calendar calendar_;
        BusinessDayConvention paymentConvention_;
        DayCounter dayCounter_;
        ext::shared_ptr<YoYInflationIndex> yii_;
        CPI::InterpolationType interpolation_;
        ext::shared_ptr<YearOnYearInflationSwap> yyiis_;
        Handle<YieldTermStructure> nominalTermStructure_;
    };

}


#endif
