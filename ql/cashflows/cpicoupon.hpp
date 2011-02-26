/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Chris Kenyon

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

/*! \file cpicoupon.hpp
 \brief Coupon paying a zero inflation index
 */

#ifndef quantlib_cpicoupon_hpp
#define quantlib_cpicoupon_hpp

#include <boost/shared_ptr.hpp>
#include <ql/cashflows/inflationcoupon.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/cashflows/baseindexedcashflow.hpp>
#include <ql/time/schedule.hpp>


namespace QuantLib {
	
    class CPICouponPricer;

    //! %Coupon paying a cpi aka zero-inflation type index relative to an index value on the baseDate
	//! This is more sophisticated than an %IndexedCashFlow because it does date calculations
	//! itself.  
	//!
	//! \todo N.B. we do not do any convexity adjustment for lags different to the natural ZCIIS
	//! lag that was used to create the forward inflation curve.
	//!
	//! The other inflation value is taken from the refPeriodEnd date with observation lag,
	//! so any roll/callendar etc will be built in by the caller.  By default this is done
	//! in the InflationCoupon which uses ModifiedPreceeding with fixing days assumed positive
	//! meaning earlier, i.e. always stay in same month (relative to referencePeriodEnd).
    class CPICoupon : public InflationCoupon {
    public:
		CPICoupon(const Real baseCPI,	// user provided, could be arbitrary
							const Date& paymentDate,
							Real nominal,
							const Date& startDate,
							const Date& endDate,
							Natural fixingDays,
							const boost::shared_ptr<ZeroInflationIndex>& index,
							const Period& observationLag,
							indexInterpolationType observationInterpolation,
							const DayCounter& dayCounter,
							Real fixedRate, // aka gearing
							Spread spread = 0.0,
							const Date& refPeriodStart = Date(),
							const Date& refPeriodEnd = Date()
							);
		
		
        //! \name Inspectors
        //@{
        //! fixed rate that will be inflated by the index ratio
        Real fixedRate() const;
        //! spread paid over the fixing of the underlying index
        Spread spread() const;

		//! adjusted fixing has already divided by the baseFixing() from the baseDate
        Rate adjustedFixing() const;
		//! you may want a different interpolation from the index, so reimplement here
		Rate indexFixing() const;
		//! \wrning make sure that the interpolation used to create this is what you 
		//! are using for the fixing, i.e. the observationInterpolation.
		Rate baseCPI() const;
		//! how do you observe the index?  as-is, flat, linear?
		indexInterpolationType observationInterpolation() const;
		//! utility method, calls indexFixing
		Rate indexObservation(const Date& onDate) const;
		//! index used (cannot be a reference type because uses a dynamic_pointer_cast)
        boost::shared_ptr<ZeroInflationIndex> cpiIndex() const;
        //@}
		
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
		
    protected:
		Real baseCPI_;
        Real fixedRate_;
        Spread spread_;
        indexInterpolationType observationInterpolation_;
        
		bool checkPricerImpl(const boost::shared_ptr<InflationCouponPricer>&) const;
		
		//! use to calculate for fixing date, allows change of interpolation w.r.t. index
		//! can also be used ahead of time
		Rate indexFixing(const Date &) const;
    };

	
	inline Real CPICoupon::fixedRate() const 
	{ return fixedRate_; }
	
	inline Real CPICoupon::spread() const 
	{ return spread_; }
	
    inline Rate CPICoupon::adjustedFixing() const 
	{ return (rate()-spread())/fixedRate(); }

	inline Rate CPICoupon::indexFixing() const 
	{ return indexFixing(fixingDate()); }
	
	inline Rate CPICoupon::baseCPI() const
	{ return baseCPI_; }
	
	inline indexInterpolationType CPICoupon::observationInterpolation() const 
	{ return observationInterpolation_; }
	
	inline Rate CPICoupon::indexObservation(const Date& onDate) const 
	{ return indexFixing(onDate); }
	
    inline boost::shared_ptr<ZeroInflationIndex> CPICoupon::cpiIndex() const 
	{ return boost::dynamic_pointer_cast<ZeroInflationIndex>(index()); }
	
	
	
    //! Helper class building a sequence of capped/floored zero inflation coupons
	//! also allowing for the inflated notional at the end ... especially if there is 
	//! only one date in the schedule.  If a fixedRate is zero you get a FixedRateCoupon
	//! otherwise you get a ZeroInflationCoupon.
    //! payoff is: spread + fixedRate x index
    class CPILeg {
    public:
		CPILeg(const Schedule& schedule, const boost::shared_ptr<ZeroInflationIndex>& index,
			   const Real baseCPI, const Period& observationLag);
        CPILeg& withNotionals(Real notional);
        CPILeg& withNotionals(const std::vector<Real>& notionals);
		CPILeg& withFixedRates(Real fixedRate);
		CPILeg& withFixedRates(const std::vector<Real>& fixedRates);
        CPILeg& withPaymentDayCounter(const DayCounter&);
        CPILeg& withPaymentAdjustment(BusinessDayConvention);
		CPILeg& withFixingDays(Natural fixingDays);
		CPILeg& withFixingDays(const std::vector<Natural>& fixingDays);
		CPILeg& withObservationInterplation(indexInterpolationType);
		CPILeg& withSubtractInflationNominal(bool);
        CPILeg& withSpreads(Spread spread);
        CPILeg& withSpreads(const std::vector<Spread>& spreads);
        CPILeg& withCaps(Rate cap);
        CPILeg& withCaps(const std::vector<Rate>& caps);
        CPILeg& withFloors(Rate floor);
        CPILeg& withFloors(const std::vector<Rate>& floors);
        operator Leg() const;

    private:
		Schedule schedule_;
		boost::shared_ptr<ZeroInflationIndex> index_;
		Real baseCPI_;
        Period observationLag_;
		std::vector<Real> notionals_;
		std::vector<Real> fixedRates_;	// aka gearing
		DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
		std::vector<Natural> fixingDays_;
		indexInterpolationType observationInterpolation_;
		bool subtractInflationNominal_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
    };



}

#endif

