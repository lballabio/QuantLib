/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006, 2007 Chiara Fornarola

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

/*! \file cmsratebond.hpp
    \brief CMS-rate bond
*/

#ifndef quantlib_cms_rate_bond_hpp
#define quantlib_cms_rate_bond_hpp

#include <ql/instruments/bond.hpp>

namespace QuantLib {

    class Schedule;
    class SwapIndex;

    //! CMS-rate bond
    /*! \ingroup instruments

        \test calculations are tested by checking results against
              cached values.
    */
    class CmsRateBond : public Bond {
      public:
        CmsRateBond(Natural settlementDays,
                    Real faceAmount,
                    const Schedule& schedule,
                    const ext::shared_ptr<SwapIndex>& index,
                    const DayCounter& paymentDayCounter,
                    BusinessDayConvention paymentConvention = Following,
                    Natural fixingDays = Null<Natural>(),
                    const std::vector<Real>& gearings = { 1.0 },
                    const std::vector<Spread>& spreads = { 0.0 },
                    const std::vector<Rate>& caps = {},
                    const std::vector<Rate>& floors = {},
                    bool inArrears = false,
                    Real redemption = 100.0,
                    const Date& issueDate = Date());
    };

}

#endif
