/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Marcin Rybacki

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file equityindex.hpp
    \brief base class for equity indexes
*/

#ifndef quantlib_equityindex_hpp
#define quantlib_equityindex_hpp

#include <ql/index.hpp>
#include <ql/time/calendar.hpp>
#include <ql/currency.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! Base class for equity indexes
    /*! The equity index object allows to retrieve past fixings,
        as well as project future fixings using either both
        the risk free interest rate term structure and the dividend
        term structure, or just the interest rate term structure
        in which case one can provide a term structure of equity
        forwards implied from, e.g. option prices.

        In case of the first method, the forward is calculated as:
        \f[
        I(t, T) = I(t, t) \frac{P_{D}(t, T)}{P_{R}(t, T)},
        \f]
        where \f$ I(t, t) \f$ is today's value of the index,
        \f$ P_{D}(t, T) \f$ is a discount factor of the dividend
        curve at future time \f$ T \f$, and \f$ P_{R}(t, T) \f$ is
        a discount factor of the risk free curve at future time
        \f$ T \f$.

        In case of the latter method, the forward is calculated as:
        \f[
        I(t, T) = I(t, t) \frac{1}{P_{F}(t, T)},
        \f]
        where \f$ P_{F}(t, T) \f$ is a discount factor of the equity
        forward term structure.

        To forecast future fixings, the user can either provide a
        handle to the current index spot. If spot handle is empty,
        today's fixing will be used, instead.

        The formulas above apply to price indexes, which is the
        default return type. The return type determines how the
        index level is published, and therefore how its forward
        is projected:
        - \c Price: the index level excludes dividends, so the
          forward is as described above. The interest rate curve
          is required, the dividend curve is optional.
        - \c TotalReturn: dividends are reinvested in the index,
          so they are not deducted from the forward:
          \f[
          I(t, T) = I(t, t) \frac{1}{P_{R}(t, T)}.
          \f]
          The interest rate curve is required, the dividend curve
          is ignored.
        - \c ExcessReturn: the cost of funding is already deducted
          from the index level, so the forward is the spot level:
          \f[
          I(t, T) = I(t, t).
          \f]
          Neither the interest rate nor the dividend curve is
          required.

        Past fixings do not depend on the return type.

        \warning EquityQuantoCashFlowPricer applies the quanto
                 adjustment through the dividend curve of the index,
                 so it only supports price return indexes.
    */
    class EquityIndex : public Index {
      public:
        //! Determines how the forward of the index is projected
        enum ReturnType {
            Price,       //!< dividends are not reinvested
            TotalReturn, //!< dividends are reinvested in the index
            ExcessReturn //!< dividends are reinvested and funding is deducted
        };

        EquityIndex(std::string name,
                    Calendar fixingCalendar,
                    Currency currency,
                    Handle<YieldTermStructure> interest = {},
                    Handle<YieldTermStructure> dividend = {},
                    Handle<Quote> spot = {},
                    ReturnType returnType = Price);

        //! \name Index interface
        //@{
        std::string name() const override { return name_; }
        Calendar fixingCalendar() const override { return fixingCalendar_; }
        bool isValidFixingDate(const Date& fixingDate) const override;
        Real fixing(const Date& fixingDate, bool forecastTodaysFixing = false) const override;
        //@}
        //! \name Inspectors
        //@{
        //! The index currency
        Currency currency() const { return currency_; }
        //! the rate curve used to forecast fixings
        const Handle<YieldTermStructure>& equityInterestRateCurve() const { return interest_; }
        //! the dividend curve used to forecast fixings
        const Handle<YieldTermStructure>& equityDividendCurve() const { return dividend_; }
        //! index spot value
        const Handle<Quote>& spot() const { return spot_; }
        //! the return type used to forecast fixings
        ReturnType returnType() const { return returnType_; }
        //@}
        //! \name Fixing calculations
        //@{
        //! It can be overridden to implement particular conventions
        virtual Real forecastFixing(const Date& fixingDate) const;
        // @}
        //! \name Other methods
        //@{
        //! returns a copy of itself linked to different interest, dividend curves
        //! or spot quote; the return type is preserved
        virtual ext::shared_ptr<EquityIndex> clone(const Handle<YieldTermStructure>& interest,
                                                   const Handle<YieldTermStructure>& dividend,
                                                   const Handle<Quote>& spot) const;
        // @}
      private:
        std::string name_;
        Calendar fixingCalendar_;
        Currency currency_;
        Handle<YieldTermStructure> interest_;
        Handle<YieldTermStructure> dividend_;
        Handle<Quote> spot_;
        ReturnType returnType_;
    };

    inline bool EquityIndex::isValidFixingDate(const Date& d) const {
        return fixingCalendar().isBusinessDay(d);
    }
}

#endif
