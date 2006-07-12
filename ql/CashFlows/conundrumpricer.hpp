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
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file conundrumpricer.hpp
    \brief 
*/

#if !defined(quantlib_conundrum_pricer_hpp)
#define quantlib_conundrum_pricer_hpp

#include <ql\cashflows\cmscoupon.hpp>

namespace QuantLib {

	class ConundrumPricer {

	protected:

		boost::shared_ptr<YieldTermStructure> mRateCurve;
		const boost::shared_ptr<CMSCoupon> mCoupon;

		double mSwapRateValue, mExpiry;
		double mDiscount, mAnnuity, mMin, mMax, mGearing, mSpread;

		virtual double optionLetPrice(bool isCall, double strike) const = 0;
		virtual double swapLetPrice() const = 0;

		static double functionG(double x, int q, double delta, int swapLength);
		static double firstDerivativeOfG(double x, int q, double delta, int swapLength);
		static double secondDerivativeOfG(double x, int q, double delta, int swapLength);

	public:

		ConundrumPricer(const boost::shared_ptr<CMSCoupon> coupon);
		virtual ~ConundrumPricer() {
		}
		double price() const;

	};

	/*
	class ConundrumPricerByBlack : public ConundrumPricer {

		const double mSigma;
		double mDelta;
		int mSwapLength;
		int mQ;
		models::Black *mBlack;
		double mFirstDerivativeOfGAtForwardValue;

	protected:

		virtual double optionLetPrice(bool isCall, double strike) const;
		virtual double swapLetPrice() const;

	public:

		ConundrumPricerByBlack(const instruments::CmsCoupon& coupon, const market::RateCurve& rateCurve, double sigma);
		virtual ~ConundrumPricerByBlack();
	};
	
	class ConundrumPricerByNumericalIntegration : public  ConundrumPricer {

		class ConundrumIntegrand : public Function<double, double> {

			friend class ConundrumPricerByNumericalIntegration;

		protected:

			const muffin::VanillaOptionPricer& mVanillaOptionPricer;
			const double mForwardValue, mExpiry, mStrike, mAnnuity, mPaymentTime;
			const int mSwapLength;
			const bool mIsCaplet, mIsPayer;

			virtual double functionG (const double x) const = 0;
			virtual double firstDerivativeOfG (const double x) const = 0;	
			virtual double secondDerivativeOfG (const double x) const = 0;
		
			virtual double functionF (const double x) const;
			virtual double firstDerivativeOfF (const double x) const;	
			virtual double secondDerivativeOfF (const double x) const;

			double strike() const;
			double annuity() const;
			double expiry() const;

		public:

			ConundrumIntegrand(const muffin::VanillaOptionPricer& o,
				const underlyings::SwapRate& swapRate,	
				const market::RateCurve& rateCurve,
				const double paymentTime,
				const double strike,
				const bool isCaplet);

			virtual double operator()(const double& x) const;
		};

		class ConundrumIntegrandStandard : public ConundrumIntegrand {

			const double mDelta;
			int mQ;

			virtual double functionG (const double x) const;
			virtual double firstDerivativeOfG (const double x) const;	
			virtual double secondDerivativeOfG (const double x) const;

		public:
				
			ConundrumIntegrandStandard(
				const muffin::VanillaOptionPricer& o,
				const underlyings::SwapRate& swapRate,	
				const market::RateCurve& rateCurve,
				const double paymentTime,
				const double strike,
				const bool isCaplet);
		};
				

		const muffin::VanillaOptionPricer& mVanillaOptionPricer;
		
		std::auto_ptr<const ConundrumIntegrand> mIntegrandForCap, mIntegrandForFloor;
		double mInfinity;
	
		double integrate(double a, double b, const ConundrumIntegrand& integrand) const;
		virtual double optionLetPrice(bool isCap, double strike) const;
		virtual double swapLetPrice() const;

	public:
		///
		///Prices a CMS coupon via static replication as in HAGAN's "Conundrums..." article
		///
		ConundrumPricerByNumericalIntegration(const muffin::VanillaOptionPricer& o, const instruments::CmsCoupon& coupon, const market::RateCurve& rateCurve);
		virtual ~ConundrumPricerByNumericalIntegration();
		
	};
*/

}

#endif
