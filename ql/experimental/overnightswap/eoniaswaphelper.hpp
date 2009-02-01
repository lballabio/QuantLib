/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters

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

/*! \file eoniaswaphelper.hpp
    \brief Eonia swap rate helpers
*/

#ifndef quantlib_eoniaswaphelper_hpp
#define quantlib_eoniaswaphelper_hpp

#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/instruments/eoniaswap.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! Rate helper for bootstrapping over Eonia swap rates
    class EoniaSwapHelper : public RelativeDateRateHelper {
      public:
        EoniaSwapHelper(const Handle<Quote>& fixedRate,
                        const Period& tenor, // swap maturity
                        Natural settlementDays,
                        const Calendar& calendar,
                        // eonia leg
                        const Period& eoniaPeriod,
                        BusinessDayConvention eoniaConvention,
                        const boost::shared_ptr<Eonia>& index,
                        // fixed leg
                        const Period& fixedPeriod,
                        BusinessDayConvention fixedConvention,
                        const DayCounter& fixedDayCount);
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        void setTermStructure(YieldTermStructure*);
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
    protected:
        void initializeDates();
        Period tenor_;
        Natural settlementDays_;
        Calendar calendar_;
        Period eoniaPeriod_;
        BusinessDayConvention eoniaConvention_;
        boost::shared_ptr<Eonia> index_;
        Period fixedPeriod_;
        BusinessDayConvention fixedConvention_;
        DayCounter fixedDayCount_;

        boost::shared_ptr<EoniaSwap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

}

#endif
