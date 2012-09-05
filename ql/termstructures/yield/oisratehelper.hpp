/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2012 Roland Lichters
 Copyright (C) 2009, 2012 Ferdinando Ametrano

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

/*! \file oisratehelper.hpp
    \brief Overnight Indexed Swap (aka OIS) rate helpers
*/

#ifndef quantlib_oisratehelper_hpp
#define quantlib_oisratehelper_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/instruments/overnightindexedswap.hpp>

namespace QuantLib {

    //! Rate helper for bootstrapping over Overnight Indexed Swap rates
    class OISRateHelper : public RelativeDateRateHelper {
      public:
        OISRateHelper(Natural settlementDays,
                      const Period& tenor, // swap maturity
                      const Handle<Quote>& fixedRate,
                      const boost::shared_ptr<OvernightIndex>& overnightIndex,
                      // exogenous discounting curve
                      const Handle<YieldTermStructure>& discountingCurve
                                            = Handle<YieldTermStructure>());
        //! \name RateHelper interface
        //@{
        Real impliedQuote() const;
        void setTermStructure(YieldTermStructure*);
        //@}
        //! \name inspectors
        //@{
        boost::shared_ptr<OvernightIndexedSwap> swap() const { return swap_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
    protected:
        void initializeDates();

        Natural settlementDays_;
        Period tenor_;
        boost::shared_ptr<OvernightIndex> overnightIndex_;

        boost::shared_ptr<OvernightIndexedSwap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;

        Handle<YieldTermStructure> discountHandle_;
        RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;
    };

    //! Rate helper for bootstrapping over Overnight Indexed Swap rates
    class DatedOISRateHelper : public RateHelper {
      public:
        DatedOISRateHelper(
                    const Date& startDate,
                    const Date& endDate,
                    const Handle<Quote>& fixedRate,
                    const boost::shared_ptr<OvernightIndex>& overnightIndex,
                      // exogenous discounting curve
                      const Handle<YieldTermStructure>& discountingCurve
                                            = Handle<YieldTermStructure>());
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

        Handle<YieldTermStructure> discountHandle_;
        RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;
    };

}

#endif
