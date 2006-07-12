/*
 Copyright (C) 2006 Mario Pucci

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. 
 */

/*! \file conundrumpricer.hpp
    \brief 
*/

#include <ql/CashFlows/conundrumpricer.hpp>

namespace QuantLib {

	ConundrumPricer::ConundrumPricer(const boost::shared_ptr<CMSCoupon> coupon) : 
	mCoupon(coupon)  {

		const boost::shared_ptr<SwapRate>& index = mCoupon->index();

		mRateCurve = index->termStructure(); 

		static const Spread basisPoint = 1.0e-4;

		DayCounter dc = mRateCurve->dayCounter();

		Date fixingDate(coupon->fixingDate());

		mExpiry = dc.yearFraction(mRateCurve->referenceDate(), fixingDate);
		mDiscount = mRateCurve->discount(coupon->date());
		
		boost::shared_ptr<VanillaSwap> swap(index->underlyingSwap(fixingDate));
		mSwapRateValue = swap->fairRate();
		mAnnuity = swap->floatingLegBPS()/basisPoint;

	}

}