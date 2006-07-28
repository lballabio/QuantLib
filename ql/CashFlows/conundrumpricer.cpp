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
#include <ql/math/gaussianquadratures.hpp>
#include <ql/math/simpsonintegral.hpp>

namespace QuantLib {

	ConundrumPricer::ConundrumPricer(const boost::shared_ptr<CMSCoupon> coupon) : 
	mCoupon(coupon), mCutoffForCaplet(2), mCutoffForFloorlet(0)  {

		const boost::shared_ptr<SwapIndex>& index = mCoupon->index();

		mRateCurve = index->termStructure(); 

		static const Spread basisPoint = 1.0e-4;

		DayCounter dc = mRateCurve->dayCounter();

		Date fixingDate(coupon->fixingDate());
		Date paymentDate(coupon->date());

		mExpiryTime = dc.yearFraction(mRateCurve->referenceDate(), fixingDate);
		mPaymentTime = dc.yearFraction(mRateCurve->referenceDate(), paymentDate);

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
				capLetPrice = optionLetPrice(false, effectiveStrikeForMax) 
					+ (swapLetPrice_ - effectiveStrikeForMax*mCoupon->accrualPeriod()*mDiscount);
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
				floorLetPrice = optionLetPrice(true, effectiveStrikeForMin) - 
					(swapLetPrice_ - effectiveStrikeForMin*mCoupon->accrualPeriod()*mDiscount);
			}
		}
		const double price = mGearing*(swapLetPrice_ + floorLetPrice - capLetPrice) + spreadLegValue;
		return price;
	}

	double ConundrumPricer::functionG_Standard(double x, int q, double delta, int swapLength) {


		const double n = swapLength *  q; 
		const double g = x / std::pow((1.0 + x/q), delta) * 1.0 / (1.0 - 1.0 / std::pow((1.0 + x/q), n));
		return  g;
	}

	double ConundrumPricer::firstDerivativeOfG_Standard(double x, int q, double delta, int swapLength) {

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

	double ConundrumPricer::secondDerivativeOfG_Standard(double x, int q, double delta, int swapLength) {
		
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

	ConundrumPricerByNumericalIntegration::ConundrumPricerByNumericalIntegration(
		const boost::shared_ptr<VanillaOptionPricer> o, 
		const boost::shared_ptr<CMSCoupon> coupon) : 
	ConundrumPricer(coupon), 
	mVanillaOptionPricer(o) {

		mInfinity = 1.0;

		const boost::shared_ptr<SwapIndex> swapRate = coupon->index();

		mIntegrandForCap = boost::shared_ptr<ConundrumIntegrandStandard>(new ConundrumIntegrandStandard(
			mVanillaOptionPricer, swapRate, mRateCurve, mExpiryTime, mPaymentTime, mAnnuity, mSwapRateValue, mSwapRateValue, true));
		mIntegrandForFloor =  boost::shared_ptr<ConundrumIntegrandStandard>(new ConundrumIntegrandStandard(
			mVanillaOptionPricer, swapRate, mRateCurve, mExpiryTime, mPaymentTime, mAnnuity, mSwapRateValue, mSwapRateValue, false));
	}

	double ConundrumPricerByNumericalIntegration::integrate(double a, double b, const ConundrumIntegrand& integrand) const {

		double integralValue = 0;

		//const int n = 25;//grado polinomi di Legendre - Questa variabile serve soltanto in caso di GaussLegendreQuadrature
		//GaussLegendre integral(n);

		SimpsonIntegral integral(.00000001, 10000);
		integralValue = integral(integrand,a , b);
		return integralValue;
	}

	double ConundrumPricerByNumericalIntegration::optionLetPrice(bool isCap, double strike) const {
		double integralValue, dFdK;
		if(isCap) {
			const double a = strike; 
			const double b = strike + mInfinity;
			integralValue = integrate(a, b, *mIntegrandForCap);
			dFdK = mIntegrandForCap->firstDerivativeOfF(strike);
		}
		else {
			const double a = 0.0; 
			const double b = strike;
			integralValue = -integrate(a, b, *mIntegrandForFloor);
			dFdK = mIntegrandForFloor->firstDerivativeOfF(strike);
		}
		const double swaptionPrice = (*mVanillaOptionPricer)(mExpiryTime, strike, isCap, mAnnuity);
		const double price = mCoupon->accrualPeriod() * (mDiscount/mAnnuity) * ((1 + dFdK) * swaptionPrice + integralValue); // v. HAGAN, Conundrums..., formule 2.17a, 2.18a
		return price;
	}

	double ConundrumPricerByNumericalIntegration::swapLetPrice() const {
		const double atmCapLetPrice = optionLetPrice(true, mSwapRateValue);
		const double atmFloorLetPrice = optionLetPrice(false, mSwapRateValue);
		const double price = mCoupon->accrualPeriod()*(mDiscount * mSwapRateValue) + atmCapLetPrice - atmFloorLetPrice;
		return price;
	}

	////////////////		ConundrumIntegrand

	ConundrumPricerByNumericalIntegration::ConundrumIntegrand::ConundrumIntegrand(const boost::shared_ptr<VanillaOptionPricer> o,
												 const boost::shared_ptr<SwapIndex> swapRate,	
												 const boost::shared_ptr<YieldTermStructure> rateCurve,
												 double expiryTime,
												 double paymentTime,
												 double annuity,
												 double forwardValue,
												 double strike,
												 bool isCaplet) :
	mVanillaOptionPricer(o), 
	mStrike(strike), 
	mPaymentTime(paymentTime), 
	mExpiryTime(expiryTime), 
	mAnnuity(annuity), 
	mIsCaplet(isCaplet),
	mForwardValue(forwardValue),
	mSwapLength(swapRate->fixedRateSchedule(rateCurve->referenceDate())->size()),
	mIsPayer(isCaplet) {
		
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrand::strike() const {
		return mStrike;
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrand::annuity() const {
		return mAnnuity;
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrand::expiryTime() const {
		return mExpiryTime;
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrand::functionF (const double x) const {
					
		const double Gx = functionG(x); 
		const double GR = functionG(mForwardValue); 
		const double f = (x - mStrike) * (Gx/GR - 1.0) ;  

		return  f;
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrand::firstDerivativeOfF (const double x) const {
					
		const double Gx = functionG(x); 
		const double GR = functionG(mForwardValue) ; 
		const double G1 = firstDerivativeOfG(x);
		const double f1 = (Gx/GR - 1.0) + G1/GR * (x - mStrike);

		return  f1;
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrand::secondDerivativeOfF (const double x) const {
					
		const double GR = functionG(mForwardValue) ; 
		const double G1 = firstDerivativeOfG(x);
		const double G2 = secondDerivativeOfG(x);
		const double f2 = 2.0 * G1/GR + (x - mStrike) * G2/GR; 

		return  f2;
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrand::operator()(const double& x) const { 

		const double option = (*mVanillaOptionPricer)(mExpiryTime, x, mIsCaplet, mAnnuity); 
		const double f2 = secondDerivativeOfF(x); 

		return option * f2;
	}


	////////////////////		ConundrumIntegrandStandard


	ConundrumPricerByNumericalIntegration::ConundrumIntegrandStandard::ConundrumIntegrandStandard(
				const boost::shared_ptr<VanillaOptionPricer> o,
				const boost::shared_ptr<SwapIndex> swapRate,	
				const boost::shared_ptr<YieldTermStructure> rateCurve,
				double expiryTime,
				double paymentTime,
				double annuity,
				double forwardValue,
				double strike,
				bool isCaplet) : 
	ConundrumPricerByNumericalIntegration::ConundrumIntegrand(o, swapRate, rateCurve, expiryTime, paymentTime, annuity, forwardValue, strike, isCaplet) {

		boost::shared_ptr<Schedule> schedule(swapRate->fixedRateSchedule(rateCurve->referenceDate()));
		DayCounter dc = rateCurve->dayCounter();
		const Time startTime = dc.yearFraction(rateCurve->referenceDate(), schedule->startDate() );
		const Time swapFirstPaymentTime = dc.yearFraction(rateCurve->referenceDate(), schedule->date(1) );

		mDelta = (paymentTime-startTime) / (swapFirstPaymentTime-startTime);
		mQ = schedule->frequency();
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrandStandard::functionG (const double x) const {
		return ConundrumPricer::functionG_Standard(x, mQ, mDelta, mSwapLength);
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrandStandard::firstDerivativeOfG (const double x) const {
		return ConundrumPricer::firstDerivativeOfG_Standard(x, mQ, mDelta, mSwapLength);
	}

	double ConundrumPricerByNumericalIntegration::ConundrumIntegrandStandard::secondDerivativeOfG (const double x) const {
		return ConundrumPricer::secondDerivativeOfG_Standard(x, mQ, mDelta, mSwapLength);
	}


}