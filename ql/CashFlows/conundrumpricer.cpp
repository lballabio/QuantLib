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
	mCoupon(coupon), mCutoffForCaplet(2), mCutoffForFloorlet(0)  {

		const boost::shared_ptr<SwapRate>& index = mCoupon->index();

		mRateCurve = index->termStructure(); 

		static const Spread basisPoint = 1.0e-4;

		DayCounter dc = mRateCurve->dayCounter();

		Date fixingDate(coupon->fixingDate());

		mExpiry = dc.yearFraction(mRateCurve->referenceDate(), fixingDate);
		mDiscount = mRateCurve->discount(coupon->date());
		
		{
			const boost::shared_ptr<VanillaSwap>& swap(index->underlyingSwap(fixingDate));
			mSwapRateValue = swap->fairRate();
			mAnnuity = swap->floatingLegBPS()/basisPoint;
		}

		mMin = coupon->floor();
		mMax = coupon->cap();
		mGearing = coupon->multiplier();
		mSpread = coupon->spread();

	}

	double ConundrumPricer::price() const {
	
		const double swapLetPrice_ = swapLetPrice(); 
		const double spreadLegValue = mSpread*mCoupon->accrualPeriod()*mDiscount;

		const double effectiveStrikeForMax = (mMax-mSpread)/mGearing;
		double capLetPrice = 0;

		if(mMax < mCutoffForCaplet) {
			if(effectiveStrikeForMax<=mSwapRateValue) {
				capLetPrice = optionLetPrice(false, effectiveStrikeForMax) + (swapLetPrice_ - effectiveStrikeForMax*mCoupon->accrualPeriod()*mDiscount);
			}
			else {
				capLetPrice = optionLetPrice(true, effectiveStrikeForMax);
			}
		}

		const double effectiveStrikeForMin = (mMin-mSpread)/mGearing;
		double floorLetPrice = 0;

		if(mMin > mCutoffForFloorlet) { 
			if(effectiveStrikeForMin<=mSwapRateValue) {
				floorLetPrice = optionLetPrice(false, effectiveStrikeForMin);
			}
			else {
				floorLetPrice = optionLetPrice(true, effectiveStrikeForMin) - (swapLetPrice_ - effectiveStrikeForMin*mCoupon->accrualPeriod()*mDiscount);
			}
		}
		const double price = mGearing*(swapLetPrice_ + floorLetPrice - capLetPrice) + spreadLegValue;
		return price;
	}

	double ConundrumPricer::functionG(double x, int q, double delta, int swapLength) {


		const double n = swapLength *  q; 
		const double g = x / std::pow((1.0 + x/q), delta) * 1.0 / (1.0 - 1.0 / std::pow((1.0 + x/q), n));
		return  g;
	}

	double ConundrumPricer::firstDerivativeOfG(double x, int q, double delta, int swapLength) {

		const double n = swapLength * q; 
		const double a = 1.0 + x / q;
		const double AA = a - delta/q * x;
		const double B = std::pow(a,(n - delta - 1.0))/(std::pow(a,n) - 1.0);

		const double secNum = n * x * std::pow(a,(n-1.0));
		const double secDen = q * std::pow(a, delta) * (std::pow(a, n) - 1.0) * (std::pow(a, n) - 1.0);
		const double sec = secNum / secDen;

		const double g1 = AA * B - sec;
		return  g1;
	}

	double ConundrumPricer::secondDerivativeOfG(double x, int q, double delta, int swapLength) {
		
		const double n = swapLength * q; 
		const double a = 1.0 + x / q;
		const double AA = a - delta/q * x;
		const double A1 = (1.0 - delta)/q;
		const double B = std::pow(a,(n - delta - 1.0))/(std::pow(a,n) - 1.0);
		const double Num = (1.0 + delta - n) * std::pow(a, (n-delta-2.0) ) - (1.0 + delta) * std::pow(a, (2.0*n-delta-2.0)); 
		const double Den = (std::pow(a, n) - 1.0) * (std::pow(a, n) - 1.0);
		const double B1 = 1.0 / q * Num / Den;

		const double C =  x / std::pow(a, delta);
		const double C1 = (std::pow(a, delta) 
			- delta /q * x * std::pow(a, (delta - 1.0))) / std::pow(a, 2 * delta);

		const double D =  std::pow(a, (n-1.0))/ ((std::pow(a, n) - 1.0) * (std::pow(a, n) - 1.0));
		const double D1 = ((n - 1.0) * std::pow(a, (n-2.0)) * (std::pow(a, n) - 1.0) 
			- 2 * n * std::pow(a, (2 * (n-1.0)))) 
			/ (q * (std::pow(a, n) - 1.0)*(std::pow(a, n) - 1.0)*(std::pow(a, n) - 1.0));

		return A1 * B + AA * B1 - n/q * (C1 * D + C * D1);

	}


}