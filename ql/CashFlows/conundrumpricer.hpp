/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 Giorgio Facchinetti

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

#ifndef quantlib_conundrum_pricer_hpp
#define quantlib_conundrum_pricer_hpp

#include <ql\cashflows\cmscoupon.hpp>
#include <ql/PricingEngines/blackmodel.hpp>

namespace QuantLib
{
	class VanillaOptionPricer {
      public:
		virtual ~VanillaOptionPricer() {};
        virtual Real operator()(Date expiryDate,
                                Real strike, 
                                bool isCall, 
                                Real deflator) const = 0;
	};

	class BlackVanillaOptionPricer : public VanillaOptionPricer {
      public:
        BlackVanillaOptionPricer(
            Rate forwardValue, 
            const Period& swapTenor,
            const boost::shared_ptr<SwaptionVolatilityStructure>& volatilityStructure)
        : forwardValue_(forwardValue), swapTenor_(swapTenor),
          volatilityStructure_(volatilityStructure) {};

        Real operator()(Date expiryDate,
                        Real strike,
                        bool isCall,
                        Real deflator) const;
      private:
        const Rate forwardValue_;
        const Period swapTenor_;
        const boost::shared_ptr<SwaptionVolatilityStructure> volatilityStructure_;
	};

    class GFunction {
      public:
		virtual ~GFunction() {};
        virtual Real operator()(Real x) = 0;
        virtual Real firstDerivative(Real x) = 0;
        virtual Real secondDerivative(Real x) = 0;
	};

	class GFunctionFactory
    {
      public:
        enum ModelOfYieldCurve { standard,
                                 exactYield,
                                 parallelShifts,
                                 nonParallelShifts };

		static boost::shared_ptr<GFunction> newGFunctionStandard(Size q,
                                                                 Real delta,
                                                                 Size swapLength);
        static boost::shared_ptr<GFunction> newGFunctionWithShifts(const CMSCoupon& coupon,
                                                                  Real meanReversion);     
		private:
		GFunctionFactory();

        /*! Corresponds to Standard Model in Hagan's paper */		
		class GFunctionStandard : public GFunction {
          public:
            GFunctionStandard(Size q,
                              Real delta,
                              Size swapLength)
            : q_(q), delta_(delta), swapLength_(swapLength) {};
			Real operator()(Real x) ;
            Real firstDerivative(Real x);
            Real secondDerivative(Real x);
          protected:
			/** number of period per year */
			const int q_;
			/** fraction of a period between the swap start date and the pay date  */
			Real delta_;
			/** length of swap*/
			Size swapLength_;
		};

		class GFunctionWithShifts : public GFunction {
			
			Time swapStartTime_;
			
			Time shapedPaymentTime_;
			std::vector<Time> shapedSwapPaymentTimes_;

			std::vector<Time> accruals_;
			std::vector<Real> swapPaymentDiscounts_;
			Real discountAtStart_, discountRatio_;

			/** value determinated implicitly  */
			Real shift_;

			Real swapRateValue_;
			Real meanReversion_;

			Real calibratedShift_, accuracy_, tmpRs_;;

			//* function describing the non-parallel shape of the curve shift*/
			Real shapeOfShift(Real s) const;
            //* calibration of shift*/
            Real calibrationOfShift(Real Rs);
            Real functionZ(Real x);
            Real derRs_derX(Real x);
            Real derZ_derX(Real x);
            Real der2Rs_derX2(Real x);
            Real der2Z_derX2(Real x);

			class ObjectiveFunction : public std::unary_function<Real, Real> {
				
				const GFunctionWithShifts& o_;
                const Real Rs_;

			public:
				ObjectiveFunction(const GFunctionWithShifts& o, const Real Rs) : o_(o), Rs_(Rs){
				}
				virtual Real operator()(const Real& x) const;
			};

          public:
            
			  GFunctionWithShifts(const CMSCoupon& coupon, Real meanReversion);
			
			  Real operator()(Real x) ;
			  Real firstDerivative(Real x);
			  Real secondDerivative(Real x);

		protected:

		};

    };

    // forward declaration
    class CMSCoupon;

    //! ConundrumPricer
    /*! Base class for the pricing of a CMS coupon via static replication
        as in Hagan's "Conundrums..." article
	*/
    class ConundrumPricer: public VanillaCMSCouponPricer {
      public:
		ConundrumPricer(const GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve);
		Real price() const;
        Real rate() const;
        void initialize(const CMSCoupon& coupon);
      protected:
		virtual Real optionLetPrice(bool isCall, Real strike) const = 0;
		virtual Real swapLetPrice() const = 0;

		boost::shared_ptr<YieldTermStructure> rateCurve_;
        GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve_;
		boost::shared_ptr<GFunction> gFunction_;
        const CMSCoupon* coupon_;
        Date paymentDate_, fixingDate_;
		Real swapRateValue_;
		Real discount_, annuity_, min_, max_, gearing_, spread_;
		const Real cutoffForCaplet_, cutoffForFloorlet_;
        Period swapTenor_;
        boost::shared_ptr<VanillaOptionPricer> vanillaOptionPricer_;
	};


	//! ConundrumPricerByNumericalIntegration
    /*! Prices a CMS coupon via static replication as in Hagan's "Conundrums..." article 
		via numerical Integration based on prices of vanilla swaptions
	*/
	class ConundrumPricerByNumericalIntegration : public ConundrumPricer
    {
      public:
		ConundrumPricerByNumericalIntegration( 
            const GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve = 
            GFunctionFactory::standard);
      private:
		class Function : public std::unary_function<Real, Real> {
          public:
    		virtual ~Function() {};
			virtual Real operator()(Real x) const = 0;
		};
		//! ConundrumIntegrand
	    /*!	Base class for the definition of the Integrand for Hagan's Integral */
		class ConundrumIntegrand : public Function
        {
          friend class ConundrumPricerByNumericalIntegration;
          public:
			ConundrumIntegrand(const boost::shared_ptr<VanillaOptionPricer>& o,
                               const boost::shared_ptr<YieldTermStructure>& rateCurve,
                               const boost::shared_ptr<GFunction>& gFunction,
                               Date fixingDate,											
                               Date paymentDate,
                               Real annuity,
                               Real forwardValue,
                               Real strike,
                               bool isCaplet);
			Real operator()(Real x) const;
          protected:
			Real functionF(const Real x) const;
			Real firstDerivativeOfF(const Real x) const;	
			Real secondDerivativeOfF(const Real x) const;

			Real strike() const;
			Real annuity() const;
			Date fixingDate() const;
            void setStrike(Real strike);

			const boost::shared_ptr<VanillaOptionPricer> vanillaOptionPricer_;
			const Real forwardValue_, annuity_;
            const Date fixingDate_, paymentDate_;
            Real strike_;
			const bool isCaplet_, isPayer_;
            boost::shared_ptr<GFunction> gFunction_;
		};

		Real integrate(Real a,
                       Real b,
                       const ConundrumIntegrand& Integrand) const;
		virtual Real optionLetPrice(bool isCap, 
                                    Real strike) const;
		virtual Real swapLetPrice() const;
		
        Real mInfinity_;
	};

    class ConundrumPricerByBlack : public ConundrumPricer
    {
      public:
		ConundrumPricerByBlack(
                    GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve = 
                    GFunctionFactory::standard);
      protected:
		Real optionLetPrice(bool isCall,
                            Real strike) const;
		Real swapLetPrice() const;

	};

}

#endif
