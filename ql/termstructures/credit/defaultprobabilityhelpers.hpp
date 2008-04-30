/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Jose Aparicio
 Copyright (C) 2008 Chris Kenyon
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file defaultprobabilityhelpers.hpp
    \brief bootstrap helpers for default-probability term structures
*/

#ifndef quantlib_default_probability_helpers_hpp
#define quantlib_default_probability_helpers_hpp

#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/termstructures/bootstraphelper.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class CreditDefaultSwap;

    //! alias for default-probability bootstrap helpers
    typedef BootstrapHelper<DefaultProbabilityTermStructure>
                                                     DefaultProbabilityHelper;

    //! Default-probability bootstrap helper based on quoted CDS spreads
    class CdsHelper : public DefaultProbabilityHelper {
      public:
        CdsHelper(const Handle<Quote>& spread,
                  const Period& tenor,
                  Integer settlementDays,
                  const Calendar& calendar,
                  Frequency frequency,
                  BusinessDayConvention paymentConvention,
                  const DayCounter& dayCounter,
                  Real recoveryRate,
                  const Handle<YieldTermStructure>& discountCurve,
                  bool settlesAccrual = true,
                  bool paysAtDefaultTime = true);
        CdsHelper(Rate spread,
                  const Period& tenor,
                  Integer settlementDays,
                  const Calendar& calendar,
                  Frequency frequency,
                  BusinessDayConvention paymentConvention,
                  const DayCounter& dayCounter,
                  Real recoveryRate,
                  const Handle<YieldTermStructure>& discountCurve,
                  bool settlesAccrual = true,
                  bool paysAtDefaultTime = true);
        Real impliedQuote() const;
        void setTermStructure(DefaultProbabilityTermStructure*);
      private:
        void update();
        void initializeDates();
        Period tenor_;
        Integer settlementDays_;
        Calendar calendar_;
        Frequency frequency_;
        BusinessDayConvention paymentConvention_;
        DayCounter dayCounter_;
        Real recoveryRate_;
        Handle<YieldTermStructure> discountCurve_;
        bool settlesAccrual_;
        bool paysAtDefaultTime_;

        Date evaluationDate_;
        boost::shared_ptr<CreditDefaultSwap> swap_;
        RelinkableHandle<DefaultProbabilityTermStructure> probability_;
    };

}


#endif

