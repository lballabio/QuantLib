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

/*! \file overnightswaphelper.hpp
    \brief Overnight Indexed Swap (aka OIS) rate helpers
*/

#ifndef quantlib_overnightswaphelper_hpp
#define quantlib_overnightswaphelper_hpp

#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/experimental/overnightswap/overnightindexedswap.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! Rate helper for bootstrapping over Overnight Indexed Swap rates
    class OISRateHelper : public RelativeDateRateHelper {
      public:
        OISRateHelper(const Handle<Quote>& fixedRate,
                      const Period& tenor, // swap maturity
                      Natural settlementDays,
                      const Calendar& calendar,
                      // Overnight Index floating leg
                      const Period& overnightPeriod,
                      BusinessDayConvention overnightConvention,
                      const boost::shared_ptr<OvernightIndex>& overnightIndex,
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
        Period overnightPeriod_;
        BusinessDayConvention overnightConvention_;
        boost::shared_ptr<OvernightIndex> overnightIndex_;
        Period fixedPeriod_;
        BusinessDayConvention fixedConvention_;
        DayCounter fixedDayCount_;

        boost::shared_ptr<OvernightIndexedSwap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

    //! Rate helper for bootstrapping over Overnight Indexed Swap rates
    class DatedOISRateHelper : public RateHelper {
      public:
        DatedOISRateHelper(
                    const Handle<Quote>& fixedRate,
                    const Date& startDate,
                    const Date& endDate,
                    const Calendar& calendar,
                    // Overnight Index floating leg
                    const Period& overnightPeriod,
                    BusinessDayConvention overnightConvention,
                    const boost::shared_ptr<OvernightIndex>& overnightIndex,
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
        boost::shared_ptr<OvernightIndexedSwap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

}

#endif
