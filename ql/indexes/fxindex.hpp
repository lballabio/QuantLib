/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file fxindex.hpp
    \brief Historical foreign-exchange fixing index
*/

#ifndef quantlib_fx_index_hpp
#define quantlib_fx_index_hpp

#include <ql/currency.hpp>
#include <ql/index.hpp>

namespace QuantLib {

//! Historical foreign-exchange fixing index
/*! A record of past FX fixings, quoted as units of the \e target currency per
    one unit of the \e source currency (e.g. a EUR-source / USD-target index
    stores the observed number of USD per EUR).

    This index holds \b only historical fixings (added through \c addFixing): it
    owns no spot quote and no curves, and does not forecast.  It exists to supply
    the one realized FX reset that a seasoned mark-to-market cross-currency swap
    needs for its in-progress period; every future reset is projected
    analytically by the pricing engine from the two discount curves, so the index
    is never asked to look forward.  Requesting a fixing for a future date raises
    an error, and a missing historical fixing raises the standard missing-fixing
    error.

    \ingroup indexes
*/
class FxIndex : public Index {
  public:
    /*! \param familyName     Name of the fixing source (e.g. "ECB").
        \param sourceCurrency Currency in the denominator of the quote.
        \param targetCurrency Currency in the numerator of the quote.
        \param fixingCalendar Calendar defining valid fixing dates.
    */
    FxIndex(std::string familyName,
            Currency sourceCurrency,
            Currency targetCurrency,
            Calendar fixingCalendar);

    //! \name Index interface
    //@{
    std::string name() const override;
    Calendar fixingCalendar() const override;
    bool isValidFixingDate(const Date& fixingDate) const override;
    //! returns the historical fixing for the given (past or today's) date
    /*! \warning raises an error if the date is in the future, since the index
                 does not forecast, or if no fixing was stored for the date.
    */
    Real fixing(const Date& fixingDate, bool forecastTodaysFixing = false) const override;
    //@}

    //! \name Inspectors
    //@{
    const std::string& familyName() const { return familyName_; }
    const Currency& sourceCurrency() const { return sourceCurrency_; }
    const Currency& targetCurrency() const { return targetCurrency_; }
    //@}

  private:
    std::string familyName_;
    Currency sourceCurrency_;
    Currency targetCurrency_;
    Calendar fixingCalendar_;
    std::string name_;
};

}

#endif
