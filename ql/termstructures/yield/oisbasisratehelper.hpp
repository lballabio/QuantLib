/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Yue Tian

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

/*! \file oisbasisratehelper.hpp
    \brief ibor vs. Overnight Indexed basis Swap rate helpers
*/

#ifndef quantlib_oisbasisratehelper_hpp
#define quantlib_oisbasisratehelper_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/instruments/iboroisbasisswap.hpp>
#include <ql/instruments/overnightindexedswap.hpp>

namespace QuantLib {

    //! Rate helper for bootstrapping over Ibor vs. Overnight Indexed basis Swap rates
    class IBOROISBasisRateHelper : public RelativeDateRateHelper {
      public:
        IBOROISBasisRateHelper(Natural settlementDays,
						  const Period& tenor, // swap maturity
						  const Handle<Quote>& overnightSpread,
						  const boost::shared_ptr<IborIndex>& iborIndex,
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
        boost::shared_ptr<IBOROISBasisSwap> swap() const { return swap_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
    protected:
        void initializeDates();

        Natural settlementDays_;
        Period tenor_;
		boost::shared_ptr<IborIndex> iborIndex_;
        boost::shared_ptr<OvernightIndex> overnightIndex_;

        boost::shared_ptr<IBOROISBasisSwap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;

        Handle<YieldTermStructure> discountHandle_;
        RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;
    };

	//! Rate helper for bootstrapping over Fixed vs. Overnight Indexed basis Swap rates
    class FixedOISBasisRateHelper : public RelativeDateRateHelper {
      public:
        FixedOISBasisRateHelper(Natural settlementDays,
						  const Period& tenor, // swap maturity
						  const Handle<Quote>& overnightSpread,
						  const Handle<Quote>& fixedRate,
						  Frequency fixedFrequency,
                          BusinessDayConvention fixedConvention,
                          const DayCounter& fixedDayCount,
						  const boost::shared_ptr<OvernightIndex>& overnightIndex,
						  Frequency overnightFrequency, 
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
        boost::shared_ptr<Swap> swap() const { return swap_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
		//! \name Observer interface
        //@{
        void update();
        //@}
    protected:
        void initializeDates();

        Natural settlementDays_;
        Period tenor_;
		Handle<Quote> fixedRate_;
		Real usedFixedRate_;
		Frequency fixedFrequency_;
        BusinessDayConvention fixedConvention_;
        DayCounter fixedDayCount_;
        boost::shared_ptr<OvernightIndex> overnightIndex_;
		Frequency overnightFrequency_;

        boost::shared_ptr<Swap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;

        Handle<YieldTermStructure> discountHandle_;
        RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;
    };
}

#endif
