/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 StatPro Italia srl

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

/*! \file cdi.hpp
    \brief %CDI index
*/

#ifndef quantlib_cdi_hpp
#define quantlib_cdi_hpp

#include <ql/currencies/america.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/time/calendars/brazil.hpp>
#include <ql/time/daycounters/business252.hpp>

namespace QuantLib {

    //! %Brazilian CDI Overnight Index.
    /* Reference: https://www.cmegroup.com/education/files/otc-irs-brl-overview.pdf
     * "Overnight Brazilian Interbank Deposit Rate Annualized as
     * the average of the DI-OVER-EXTRA Grupo as published by CETIP"
     * "A business day in any of San Paulo, Rio de Janeiro or Brasilia
     * not otherwise declared as a financial market holiday by the BM&F"
     */
    class Cdi : public OvernightIndex {
      public:
        explicit Cdi(const Handle<YieldTermStructure>& h = {})
        : OvernightIndex("CDI", 0, BRLCurrency(), Brazil(), Business252(), h) {}

        Rate forecastFixing(const Date& fixingDate) const override;
    };

}


#endif