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

	 //! VanillaOptionPricer
    /*! 
		Interface 
    */
	class VanillaOptionPricer {
	public:
		
		virtual double operator()(double expiry, double strike, bool isCall, double deflator) const = 0;
	};

	//! ConundrumPricer
    /*! 
		Base class for the pricing of a CMS coupon via static replication as in Hagan's "Conundrums..." article
	*/
	class ConundrumPricer {

	protected:

		boost::shared_ptr<YieldTermStructure> mRateCurve;
		const boost::shared_ptr<CMSCoupon> mCoupon;

		double mSwapRateValue, mExpiryTime;
		double mDiscount, mAnnuity, mMin, mMax, mGearing, mSpread, mPaymentTime;
		const double mCutoffForCaplet, mCutoffForFloorlet;

		virtual double optionLetPrice(bool isCall, double strike) const = 0;
		virtual double swapLetPrice() const = 0;

		static double functionG_Standard(double x, int q, double delta, int swapLength);
		static double firstDerivativeOfG_Standard(double x, int q, double delta, int swapLength);
		static double secondDerivativeOfG_Standard(double x, int q, double delta, int swapLength);

	public:

		ConundrumPricer(const boost::shared_ptr<CMSCoupon> coupon);
		virtual ~ConundrumPricer() {
		}
		double price() const;
	};


	//! ConundrumPricerByNumericalIntegration
    /*! 
		Prices a CMS coupon via static replication as in Hagan's "Conundrums..." article 
		via numerical integration based on prices of vanilla swaptions
	*/
	class ConundrumPricerByNumericalIntegration : public ConundrumPricer {

		
		class Function : public std::unary_function<double, double> {
		public:
			virtual double operator()(const double& x) const = 0;
		};

		//! ConundrumIntegrand
	    /*! 
			Base class for the definition of the integrand for Hagan's integral
		*/
		class ConundrumIntegrand : public Function {

			friend class ConundrumPricerByNumericalIntegration;

		protected:

			const boost::shared_ptr<VanillaOptionPricer> mVanillaOptionPricer;
			const double mForwardValue, mExpiryTime, mStrike, mAnnuity, mPaymentTime;
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
			double expiryTime() const;

		public:

			ConundrumIntegrand(const boost::shared_ptr<VanillaOptionPricer> o,
				const boost::shared_ptr<SwapIndex> swapRate,	
				const boost::shared_ptr<YieldTermStructure> rateCurve,
				double expiryTime,											
				double paymentTime,
				double annuity,
				double forwardValue,
				double strike,
				bool isCaplet);

			virtual double operator()(const double& x) const;
		};

		//! ConundrumIntegrandStandard
	    /*! 
			Corresponds to Standard Model in Hagan's paper
		*/
		class ConundrumIntegrandStandard : public ConundrumIntegrand {

			double mDelta;
			int mQ;

			virtual double functionG (const double x) const;
			virtual double firstDerivativeOfG (const double x) const;	
			virtual double secondDerivativeOfG (const double x) const;

		public:
				
			ConundrumIntegrandStandard(
				const boost::shared_ptr<VanillaOptionPricer> o,
				const boost::shared_ptr<SwapIndex> swapRate,	
				const boost::shared_ptr<YieldTermStructure> rateCurve,
				double expiryTime,
				double paymentTime,
				double annuity,
				double forwardValue,
				double strike,
				bool isCaplet);
		};

		const boost::shared_ptr<VanillaOptionPricer> mVanillaOptionPricer;
		
		boost::shared_ptr<ConundrumIntegrand> mIntegrandForCap, mIntegrandForFloor;
		double mInfinity;
	
		double integrate(double a, double b, const ConundrumIntegrand& integrand) const;

		virtual double optionLetPrice(bool isCap, double strike) const;
		virtual double swapLetPrice() const;

	public:

		ConundrumPricerByNumericalIntegration(const boost::shared_ptr<VanillaOptionPricer> o, 
			const boost::shared_ptr<CMSCoupon> coupon);
		virtual ~ConundrumPricerByNumericalIntegration() {}
	};

}

#endif
