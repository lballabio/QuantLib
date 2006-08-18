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
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 */

/*! \file conundrumpricer.hpp
    \brief
*/

#include <ql/CashFlows/conundrumpricer.hpp>
#include <ql/math/gaussianquadratures.hpp>
#include <ql/math/kronrodintegral.hpp>
#include <ql/Solvers1D/brent.hpp>

#include <ql/Currencies/all.hpp>
#include <ql/Calendars/all.hpp>
#include <ql/DayCounters/all.hpp>
#include <ql/Indexes/all.hpp>

#include <iostream>
#include <ql/TermStructures/all.hpp>

namespace QuantLib
{
//===========================================================================//
//                          BlackVanillaOptionPricer                         //
//===========================================================================//
    Real BlackVanillaOptionPricer::operator()(Date expiryDate,
                                              Real strike,
                                              bool isCall,
                                              Real deflator) const {
        const Real optionType = isCall ? 1.0 : -1.0;
        const Real variance = volatilityStructure_->blackVariance(expiryDate,
            swapTenor_, forwardValue_);
        return deflator * detail::blackFormula(forwardValue_, strike,
            std::sqrt(variance), optionType);
    };


//===========================================================================//
//                             ConundrumPricer                               //
//===========================================================================//
    ConundrumPricer::ConundrumPricer(
                const GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve)
    : modelOfYieldCurve_(modelOfYieldCurve), 
      cutoffForCaplet_(2), cutoffForFloorlet_(0){	}
    
    void ConundrumPricer::initialize(const CMSCoupon& coupon){
        coupon_ = &coupon; 
        fixingDate_ = coupon_->fixingDate();
        paymentDate_ = coupon_->date(); 
        const boost::shared_ptr<SwapIndex>& swapIndex = coupon_->swapIndex();
        rateCurve_ = swapIndex->termStructure();
        discount_ = rateCurve_->discount(paymentDate_);
	    swapTenor_ = swapIndex->tenor();
        boost::shared_ptr<VanillaSwap> swap = swapIndex->underlyingSwap(fixingDate_);

		swapRateValue_ = swap->fairRate();

		const Spread bp = 1e-4;
        annuity_ = (swap->floatingLegBPS()/bp);

		min_ = coupon_->floor();
		max_ = coupon_->cap();
		gearing_ = coupon_->gearing();
		spread_ = coupon_->spread();
        const Size q = swapIndex->fixedLegFrequency();

        const boost::shared_ptr<Schedule> schedule(swapIndex->fixedRateSchedule(fixingDate_));

        const DayCounter dc(swapIndex->dayCounter());
		const Time startTime = dc.yearFraction(rateCurve_->referenceDate(),
                                         swap->startDate());
		const Time swapFirstPaymentTime = dc.yearFraction(rateCurve_->referenceDate(),
                                                    schedule->date(1));
		const Time paymentTime = dc.yearFraction(rateCurve_->referenceDate(),
                                           paymentDate_);
		const Real delta = (paymentTime-startTime) / (swapFirstPaymentTime-startTime);

        switch (modelOfYieldCurve_) {
            case GFunctionFactory::standard:
                gFunction_ = GFunctionFactory::newGFunctionStandard(q, delta, swapTenor_.length());
                break;
            case GFunctionFactory::exactYield:
                gFunction_ = GFunctionFactory::newGFunctionExactYield(coupon);
                break;
            case GFunctionFactory::parallelShifts:
                gFunction_ = GFunctionFactory::newGFunctionWithShifts(coupon, 0.);
                break;
            case GFunctionFactory::nonParallelShifts:
                gFunction_ = GFunctionFactory::newGFunctionWithShifts(coupon, coupon.meanReversion());
                break;
            default:
                QL_FAIL("unknown/illegal gFunction type");
        }

        vanillaOptionPricer_= boost::shared_ptr<VanillaOptionPricer>( 
            new BlackVanillaOptionPricer(swapRateValue_, swapTenor_,
                                         coupon_->swaptionVolatility().currentLink())); 
	}

	Real ConundrumPricer::price() const {

		const Real swapLetPrice_ = swapLetPrice();
		const Real spreadLegValue = spread_*coupon_->accrualPeriod()*discount_;

		const Real effectiveStrikeForMax = std::max((max_-spread_)/gearing_,QL_EPSILON);
		Real capLetPrice = 0;

		if (max_ < cutoffForCaplet_) {
			capLetPrice = optionLetPrice(true, effectiveStrikeForMax);
		}

		const Real effectiveStrikeForMin = std::max((min_-spread_)/gearing_,QL_EPSILON);;
		Real floorLetPrice = 0;


		if (min_ > cutoffForFloorlet_) {
			floorLetPrice = optionLetPrice(false, effectiveStrikeForMin);
		}
		const Real price = gearing_*(swapLetPrice_ + floorLetPrice - capLetPrice) + spreadLegValue;
		return price;
	}

	Real ConundrumPricer::rate() const {
		return price()/(coupon_->accrualPeriod()*discount_);
	}

//===========================================================================//
//                  ConundrumPricerByNumericalIntegration                    //
//===========================================================================//

    ConundrumPricerByNumericalIntegration::ConundrumPricerByNumericalIntegration(
        GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve,
		Real lowerLimit,
		Real upperLimit)
    : ConundrumPricer(modelOfYieldCurve),
       upperLimit_(upperLimit), 
	   lowerLimit_(lowerLimit) {
	}

	Real ConundrumPricerByNumericalIntegration::integrate(Real a,
        Real b, const ConundrumIntegrand& integrand) const {
        // grado polinomi di Legendre - Questa variabile serve soltanto in
        // caso di GaussLegendreQuadrature
        //const Size n = 25;
		//GaussLegendre Integral(n);

        const KronrodIntegral integral(1.e-6, 1000000);
		return integral(integrand,a , b);
	}

	Real ConundrumPricerByNumericalIntegration::optionLetPrice(
                                        bool isCap, Real strike) const {
		Real integralValue, dFdK;
		if(isCap) {
			const Real a(strike), b(std::max(strike, upperLimit_));
            const boost::shared_ptr<ConundrumIntegrand> integrandForCap = boost::shared_ptr<ConundrumIntegrand>(
            new ConundrumIntegrand(vanillaOptionPricer_, rateCurve_,
                                   gFunction_, fixingDate_, paymentDate_,
                                   annuity_, swapRateValue_, strike, true)
								   );
            integralValue = integrate(a, b, *integrandForCap);

			dFdK = integrandForCap->firstDerivativeOfF(strike);
		}
		else {
			const Real a(std::min(strike, lowerLimit_)), b(strike);
            const boost::shared_ptr<ConundrumIntegrand> integrandForFloor = boost::shared_ptr<ConundrumIntegrand>(
            new ConundrumIntegrand(vanillaOptionPricer_, rateCurve_,
                                   gFunction_, fixingDate_, paymentDate_,
                                   annuity_, swapRateValue_, strike, false)
								   );
			integralValue = -integrate(a, b, *integrandForFloor);
			dFdK = integrandForFloor->firstDerivativeOfF(strike);
		}
		const Real swaptionPrice = (*vanillaOptionPricer_)(fixingDate_,
            strike, isCap, annuity_);
		// v. HAGAN, Conundrums..., formule 2.17a, 2.18a

		return coupon_->accrualPeriod() * (discount_/annuity_) *
            ((1 + dFdK) * swaptionPrice + integralValue);
	}

	Real ConundrumPricerByNumericalIntegration::swapLetPrice() const {
		const Real atmCapLetPrice = optionLetPrice(true, swapRateValue_);
		const Real atmFloorLetPrice = optionLetPrice(false, swapRateValue_);
		return coupon_->accrualPeriod()*(discount_ * swapRateValue_)
            + atmCapLetPrice - atmFloorLetPrice;
	}

//===========================================================================//
//                              ConundrumIntegrand                           //
//===========================================================================//
	
	ConundrumPricerByNumericalIntegration::ConundrumIntegrand::ConundrumIntegrand(
        const boost::shared_ptr<VanillaOptionPricer>& o,
        const boost::shared_ptr<YieldTermStructure>& rateCurve,
        const boost::shared_ptr<GFunction>& gFunction,
        Date fixingDate,
        Date paymentDate,
        Real annuity,
        Real forwardValue,
        Real strike,
        bool isCaplet)
    : vanillaOptionPricer_(o), gFunction_(gFunction), strike_(strike),
      paymentDate_(paymentDate), fixingDate_(fixingDate), annuity_(annuity),
      isCaplet_(isCaplet), forwardValue_(forwardValue), isPayer_(isCaplet) {}

    void ConundrumPricerByNumericalIntegration::ConundrumIntegrand::setStrike(Real strike) {
		strike_ = strike;
	}

    Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::strike() const {
		return strike_;
	}

	Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::annuity() const {
		return annuity_;
	}

	Date ConundrumPricerByNumericalIntegration::ConundrumIntegrand::fixingDate() const {
		return fixingDate_;
	}

	Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::functionF (const Real x) const {
        const Real Gx = gFunction_->operator()(x);
		const Real GR = gFunction_->operator()(forwardValue_);
		return (x - strike_) * (Gx/GR - 1.0);
	}

	Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::firstDerivativeOfF (const Real x) const {
		const Real Gx = gFunction_->operator()(x);
		const Real GR = gFunction_->operator()(forwardValue_) ;
		const Real G1 = gFunction_->firstDerivative(x);
		return (Gx/GR - 1.0) + G1/GR * (x - strike_);
	}

	Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::secondDerivativeOfF (const Real x) const {
		const Real GR = gFunction_->operator()(forwardValue_) ;
		const Real G1 = gFunction_->firstDerivative(x);
		const Real G2 = gFunction_->secondDerivative(x);
		return 2.0 * G1/GR + (x - strike_) * G2/GR;
	}

	Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::operator()(Real x) const {
		const Real option = (*vanillaOptionPricer_)(fixingDate_, x, isCaplet_, annuity_);
		return option * secondDerivativeOfF(x);
	}



//===========================================================================//
//                          ConundrumPricerByBlack                           //
//===========================================================================//
    
	ConundrumPricerByBlack::ConundrumPricerByBlack(
        GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve)
    : ConundrumPricer(modelOfYieldCurve)
      { }

    //Hagan, 3.5b, 3.5c
    Real ConundrumPricerByBlack::optionLetPrice(bool isCall, Real strike) const {
        Real variance = coupon_->swaptionVolatility()->blackVariance(fixingDate_,
                                                           swapTenor_,
                                                           swapRateValue_);
        Real firstDerivativeOfGAtForwardValue = gFunction_->firstDerivative(
                                                        swapRateValue_); 
	    Real price = 0;

	    const Real CK = (*vanillaOptionPricer_)(fixingDate_, strike, isCall, annuity_);
	    price += (discount_/annuity_)*CK;
        const Real sqrtSigma2T = std::sqrt(variance);
        const Real lnRoverK =  std::log(swapRateValue_/strike);
	    const Real d32 = (lnRoverK+1.5*variance)/sqrtSigma2T;
        const Real d12 =  (lnRoverK+.5*variance)/sqrtSigma2T;
        const Real dminus12 =  (lnRoverK-.5*variance)/sqrtSigma2T;
	    const Real sgn = isCall ? 1.0 : -1.0;

        CumulativeNormalDistribution cumulativeOfNormal;
	    const Real N32 = cumulativeOfNormal(sgn*d32);
        const Real N12 = cumulativeOfNormal(sgn*d12);
        const Real Nminus12 = cumulativeOfNormal(sgn*dminus12);
    	
	    price += sgn * firstDerivativeOfGAtForwardValue * annuity_ *
            swapRateValue_ * (swapRateValue_ * std::exp(variance) * N32-
            (swapRateValue_+strike) * N12 + strike * Nminus12);
	    price *= coupon_->accrualPeriod();
	    return price;
    }

    //Hagan 3.4c
    Real ConundrumPricerByBlack::swapLetPrice() const {

        Real variance(coupon_->swaptionVolatility()->blackVariance(fixingDate_,
                                                           swapTenor_,
                                                           swapRateValue_));
        Real firstDerivativeOfGAtForwardValue(gFunction_->firstDerivative(
                                                        swapRateValue_)); 
	    Real price = 0;
	    price += discount_*swapRateValue_;
        price += firstDerivativeOfGAtForwardValue*annuity_*swapRateValue_*swapRateValue_*
            (std::exp(variance)-1.);
	    price *= coupon_->accrualPeriod();
	    return price;

    }


//===========================================================================//
//                              GFunctionStandard                            //
//===========================================================================//

    Real GFunctionFactory::GFunctionStandard::operator()(Real x) {
	    const Real n = swapLength_ * q_;
	    return x / std::pow((1.0 + x/q_), delta_) * 1.0 /
            (1.0 - 1.0 / std::pow((1.0 + x/q_), n));
    }

    Real GFunctionFactory::GFunctionStandard::firstDerivative(Real x) {
	    const Real n = swapLength_ * q_;
	    const Real a = 1.0 + x / q_;
	    const Real AA = a - delta_/q_ * x;
	    const Real B = std::pow(a,(n - delta_ - 1.0))/(std::pow(a,n) - 1.0);

	    const Real secNum = n * x * std::pow(a,(n-1.0));
	    const Real secDen = q_ * std::pow(a, delta_) * (std::pow(a, n) - 1.0) *
            (std::pow(a, n) - 1.0);
	    const Real sec = secNum / secDen;

	    return AA * B - sec;
    }

    Real GFunctionFactory::GFunctionStandard::secondDerivative(Real x) {
	    const Real n = swapLength_ * q_;
	    const Real a = 1.0 + x/q_;
	    const Real AA = a - delta_/q_ * x;
	    const Real A1 = (1.0 - delta_)/q_;
	    const Real B = std::pow(a,(n - delta_ - 1.0))/(std::pow(a,n) - 1.0);
	    const Real Num = (1.0 + delta_ - n) * std::pow(a, (n-delta_-2.0)) -
            (1.0 + delta_) * std::pow(a, (2.0*n-delta_-2.0));
	    const Real Den = (std::pow(a, n) - 1.0) * (std::pow(a, n) - 1.0);
	    const Real B1 = 1.0 / q_ * Num / Den;

	    const Real C =  x / std::pow(a, delta_);
	    const Real C1 = (std::pow(a, delta_)
		    - delta_ /q_ * x * std::pow(a, (delta_ - 1.0))) / std::pow(a, 2 * delta_);

	    const Real D =  std::pow(a, (n-1.0))/ ((std::pow(a, n) - 1.0) * (std::pow(a, n) - 1.0));
	    const Real D1 = ((n - 1.0) * std::pow(a, (n-2.0)) * (std::pow(a, n) - 1.0)
		    - 2 * n * std::pow(a, (2 * (n-1.0))))
		    / (q_ * (std::pow(a, n) - 1.0)*(std::pow(a, n) - 1.0)*(std::pow(a, n) - 1.0));

	    return A1 * B + AA * B1 - n/q_ * (C1 * D + C * D1);
    }

    boost::shared_ptr<GFunction> GFunctionFactory::newGFunctionStandard(Size q,
                                                            Real delta, Size swapLength) {
	    return boost::shared_ptr<GFunction>(new GFunctionStandard(q, delta, swapLength));
    }

//===========================================================================//
//                              GFunctionExactYield                          //
//===========================================================================//
    GFunctionFactory::GFunctionExactYield::GFunctionExactYield(const CMSCoupon& coupon){

		const boost::shared_ptr<SwapIndex>& swapIndex = coupon.swapIndex();
        const boost::shared_ptr<VanillaSwap>& swap = swapIndex->underlyingSwap(coupon.fixingDate());

		const std::vector<boost::shared_ptr<CashFlow> > fixedLeg(swap->fixedLeg());
		const boost::shared_ptr<Schedule> schedule(swapIndex->fixedRateSchedule(coupon.fixingDate()));
		const boost::shared_ptr<YieldTermStructure> rateCurve(swapIndex->termStructure());
        const DayCounter dc(swapIndex->dayCounter());

		Real swapStartTime = dc.yearFraction(rateCurve->referenceDate(), schedule->startDate());
		Real swapFirstPaymentTime = dc.yearFraction(rateCurve->referenceDate(),schedule->date(1));

		const Real paymentTime(dc.yearFraction(rateCurve->referenceDate(), coupon.date()));
       
        delta_ = (paymentTime-swapStartTime) / (swapFirstPaymentTime-swapStartTime);
		
        for(Size i=0; i<fixedLeg.size(); i++) {
			const Coupon* coupon(static_cast<const Coupon*>(fixedLeg[i].get()));
			accruals_.push_back(coupon->accrualPeriod());
		}
	}

    Real GFunctionFactory::GFunctionExactYield::operator()(Real x) {
		Real product = 1.;
		for(Size i=0; i<accruals_.size(); i++) {
			product *= 1./(1.+ accruals_[i]*x);
		}
        return x*std::pow(1.+ accruals_[0]*x,-delta_)*(1./(1.-product));
    }

    Real GFunctionFactory::GFunctionExactYield::firstDerivative(Real x) {
		Real c = -1.;
        Real derC = 0.;
        std::vector<Real> b;
        for(Size i=0; i<accruals_.size(); i++) {
            b.push_back(1./(1.+ accruals_[i]*x));
            c *= b.back();
            derC += accruals_[i]*b.back(); 
		}
        c += 1.;
        c = 1./c;
        derC *= (c-c*c);

        return -delta_*accruals_[0]*std::pow(b[0],delta_+1.)*x*c+
                std::pow(b[0],delta_)*c+ std::pow(b[0],delta_)*x*derC;
        //Real dx = 1.0e-8;
        //return (operator()(x+dx)-operator()(x-dx))/(2.0*dx);
    }

    Real GFunctionFactory::GFunctionExactYield::secondDerivative(Real x) {
        Real c = -1.;
        Real sum = 0.;
        Real sumOfSquare = 0.;
        std::vector<Real> b;
        for(Size i=0; i<accruals_.size(); i++) {
            b.push_back(1./(1.+ accruals_[i]*x));
            c *= b.back();
            sum += accruals_[i]*b.back(); 
            sumOfSquare += std::pow(accruals_[i]*b.back(),2.); 
		}
        c += 1.;
        c = 1./c;
        Real derC =sum*(c-c*c);

        return (-delta_*accruals_[0]*std::pow(b[0],delta_+1.)*c+ std::pow(b[0],delta_)*derC)*
               (-delta_*accruals_[0]*b[0]*x + 1. + x*(1.-c)*sum)+
                std::pow(b[0],delta_)*c*(delta_*std::pow(accruals_[0]*b[0],2.)*x - delta_* accruals_[0]*b[0] -
                x*derC*sum + (1.-c)*sum - x*(1.-c)*sumOfSquare);
        //Real dx = 1.0e-8;
        //return (firstDerivative(x+dx)-firstDerivative(x-dx))/(2.0*dx);
    }

    boost::shared_ptr<GFunction> GFunctionFactory::newGFunctionExactYield(const CMSCoupon& coupon) {
	    return boost::shared_ptr<GFunction>(new GFunctionExactYield(coupon));
    }



//===========================================================================//
//                            GFunctionWithShifts                            //
//===========================================================================//

    GFunctionFactory::GFunctionWithShifts::GFunctionWithShifts(const CMSCoupon& coupon, 
		Real meanReversion) : meanReversion_(meanReversion), calibratedShift_(.03), 
		tmpRs_(10000000.), accuracy_( 1.e-14) {

		const boost::shared_ptr<SwapIndex>& swapIndex = coupon.swapIndex();
        const boost::shared_ptr<VanillaSwap>& swap = swapIndex->underlyingSwap(coupon.fixingDate());

		swapRateValue_ = swap->fairRate();

		objectiveFunction_ = boost::shared_ptr<ObjectiveFunction>(new ObjectiveFunction(*this, swapRateValue_));

		const std::vector<boost::shared_ptr<CashFlow> > fixedLeg(swap->fixedLeg());
		const boost::shared_ptr<Schedule> schedule(swapIndex->fixedRateSchedule(coupon.fixingDate()));
		const boost::shared_ptr<YieldTermStructure> rateCurve(swapIndex->termStructure());
        const DayCounter dc(swapIndex->dayCounter());

		swapStartTime_ = dc.yearFraction(rateCurve->referenceDate(), schedule->startDate());
		discountAtStart_ = rateCurve->discount(schedule->startDate());

		const Real paymentTime(dc.yearFraction(rateCurve->referenceDate(), coupon.date()));

		shapedPaymentTime_ = shapeOfShift(paymentTime);

		for(Size i=0; i<fixedLeg.size(); i++) {
			const Coupon* coupon(static_cast<const Coupon*>(fixedLeg[i].get()));
			accruals_.push_back(coupon->accrualPeriod());
			const Date paymentDate(coupon->date());
			const double swapPaymentTime(dc.yearFraction(rateCurve->referenceDate(), paymentDate));
			shapedSwapPaymentTimes_.push_back(shapeOfShift(swapPaymentTime));
			swapPaymentDiscounts_.push_back(rateCurve->discount(paymentDate));
		}
		discountRatio_ = swapPaymentDiscounts_.back()/discountAtStart_;
	}

	Real GFunctionFactory::GFunctionWithShifts::operator()(Real Rs) {
        const Real calibratedShift = calibrationOfShift(Rs);    
		return Rs* functionZ(calibratedShift);                     
    }

    Real GFunctionFactory::GFunctionWithShifts::functionZ(Real x) {
		return std::exp(-shapedPaymentTime_*x)
			/ (1.-discountRatio_*std::exp(-shapedSwapPaymentTimes_.back()*x));
    }

    Real GFunctionFactory::GFunctionWithShifts::derRs_derX(Real x) {
		Real sqrtDenominator = 0;
        Real derSqrtDenominator = 0;
		for(Size i=0; i<accruals_.size(); i++) {
			sqrtDenominator += accruals_[i]*swapPaymentDiscounts_[i]
				*std::exp(-shapedSwapPaymentTimes_[i]*x);
            derSqrtDenominator -= shapedSwapPaymentTimes_[i]* accruals_[i]*swapPaymentDiscounts_[i]
				*std::exp(-shapedSwapPaymentTimes_[i]*x);
		}
        const Real denominator = sqrtDenominator* sqrtDenominator; 

        Real numerator = 0;
        numerator += shapedSwapPaymentTimes_.back()* swapPaymentDiscounts_.back()* 
                     std::exp(-shapedSwapPaymentTimes_.back()*x)*sqrtDenominator;
        numerator -= (discountAtStart_ - swapPaymentDiscounts_.back()* std::exp(-shapedSwapPaymentTimes_.back()*x))*
                     derSqrtDenominator;
        QL_REQUIRE(denominator!=0, "GFunctionWithShifts::derRs_derX: denominator == 0");
		return numerator/denominator;
    }

    Real GFunctionFactory::GFunctionWithShifts::der2Rs_derX2(Real x) {
        Real denOfRfunztion = 0.;
        Real derDenOfRfunztion = 0.;
        Real der2DenOfRfunztion = 0.;
        for(Size i=0; i<accruals_.size(); i++) {
			denOfRfunztion += accruals_[i]*swapPaymentDiscounts_[i]
				*std::exp(-shapedSwapPaymentTimes_[i]*x);
            derDenOfRfunztion -= shapedSwapPaymentTimes_[i]* accruals_[i]*swapPaymentDiscounts_[i]
				*std::exp(-shapedSwapPaymentTimes_[i]*x);
            der2DenOfRfunztion+= shapedSwapPaymentTimes_[i]*shapedSwapPaymentTimes_[i]* accruals_[i]*
                swapPaymentDiscounts_[i]*std::exp(-shapedSwapPaymentTimes_[i]*x);
		}

        const Real denominator = std::pow(denOfRfunztion, 4); 

        Real numOfDerR = 0;
        numOfDerR += shapedSwapPaymentTimes_.back()* swapPaymentDiscounts_.back()* 
                     std::exp(-shapedSwapPaymentTimes_.back()*x)*denOfRfunztion;
        numOfDerR -= (discountAtStart_ - swapPaymentDiscounts_.back()* std::exp(-shapedSwapPaymentTimes_.back()*x))*
                     derDenOfRfunztion;

        const Real denOfDerR = std::pow(denOfRfunztion,2); 
        
        Real derNumOfDerR = 0.;
        derNumOfDerR -= shapedSwapPaymentTimes_.back()*shapedSwapPaymentTimes_.back()* swapPaymentDiscounts_.back()* 
                     std::exp(-shapedSwapPaymentTimes_.back()*x)*denOfRfunztion;
        derNumOfDerR += shapedSwapPaymentTimes_.back()* swapPaymentDiscounts_.back()* 
                     std::exp(-shapedSwapPaymentTimes_.back()*x)*derDenOfRfunztion;

        derNumOfDerR -= (shapedSwapPaymentTimes_.back()*swapPaymentDiscounts_.back()* 
                        std::exp(-shapedSwapPaymentTimes_.back()*x))* derDenOfRfunztion;
        derNumOfDerR -= (discountAtStart_ - swapPaymentDiscounts_.back()* std::exp(-shapedSwapPaymentTimes_.back()*x))*
                     der2DenOfRfunztion;

        const Real derDenOfDerR = 2*denOfRfunztion*derDenOfRfunztion;

        const Real numerator = derNumOfDerR*denOfDerR -numOfDerR*derDenOfDerR; 
        QL_REQUIRE(denominator!=0, "GFunctionWithShifts::der2Rs_derX2: denominator == 0");
        return numerator/denominator;
    }

    Real GFunctionFactory::GFunctionWithShifts::derZ_derX(Real x) {
		const Real sqrtDenominator = (1.-discountRatio_*std::exp(-shapedSwapPaymentTimes_.back()*x));
        const Real denominator = sqrtDenominator* sqrtDenominator; 
		QL_REQUIRE(denominator!=0, "GFunctionWithShifts::derZ_derX: denominator == 0");

		Real numerator = 0;
        numerator -= shapedPaymentTime_* std::exp(-shapedPaymentTime_*x)* sqrtDenominator;
		numerator -= shapedSwapPaymentTimes_.back()* std::exp(-shapedPaymentTime_*x)* (1.-sqrtDenominator);
       
        return numerator/denominator;
    }

    Real GFunctionFactory::GFunctionWithShifts::der2Z_derX2(Real x) {
		const Real denOfZfunction = (1.-discountRatio_*std::exp(-shapedSwapPaymentTimes_.back()*x));
       	const Real derDenOfZfunction = shapedSwapPaymentTimes_.back()*discountRatio_*std::exp(-shapedSwapPaymentTimes_.back()*x);
        const Real denominator = std::pow(denOfZfunction, 4); 
        QL_REQUIRE(denominator!=0, "GFunctionWithShifts::der2Z_derX2: denominator == 0");

        Real numOfDerZ = 0;
        numOfDerZ -= shapedPaymentTime_* std::exp(-shapedPaymentTime_*x)* denOfZfunction;
		numOfDerZ -= shapedSwapPaymentTimes_.back()* std::exp(-shapedPaymentTime_*x)* (1.-denOfZfunction);

        const Real denOfDerZ = std::pow(denOfZfunction,2); 
        const Real derNumOfDerZ = (-shapedPaymentTime_* std::exp(-shapedPaymentTime_*x)* 
                             (-shapedPaymentTime_+(shapedPaymentTime_*discountRatio_-
                               shapedSwapPaymentTimes_.back()*discountRatio_)* std::exp(-shapedSwapPaymentTimes_.back()*x))
                              -shapedSwapPaymentTimes_.back()*std::exp(-shapedPaymentTime_*x)*
                              (shapedPaymentTime_*discountRatio_- shapedSwapPaymentTimes_.back()*discountRatio_)*
                              std::exp(-shapedSwapPaymentTimes_.back()*x));

        const Real derDenOfDerZ = 2*denOfZfunction*derDenOfZfunction;
        const Real numerator = derNumOfDerZ*denOfDerZ -numOfDerZ*derDenOfDerZ; 

        return numerator/denominator;
    }

	Real GFunctionFactory::GFunctionWithShifts::firstDerivative(Real Rs) {
        //Real dRs = 1.0e-8;
        //return (operator()(Rs+dRs)-operator()(Rs-dRs))/(2.0*dRs);
        const Real calibratedShift = calibrationOfShift(Rs); 
        return functionZ(calibratedShift) + Rs * derZ_derX(calibratedShift)/derRs_derX(calibratedShift);
    }

	Real GFunctionFactory::GFunctionWithShifts::secondDerivative(Real Rs) {
        //Real dRs = 1.0e-8;
        //return (firstDerivative(Rs+dRs)-firstDerivative(Rs-dRs))/(2.0*dRs);
        const Real calibratedShift = calibrationOfShift(Rs); 
        return 2.*derZ_derX(calibratedShift)/derRs_derX(calibratedShift) + 
            Rs * der2Z_derX2(calibratedShift)/std::pow(derRs_derX(calibratedShift),2.)-
            Rs * derZ_derX(calibratedShift)*der2Rs_derX2(calibratedShift)/
            std::pow(derRs_derX(calibratedShift),3.);
    }

	Real GFunctionFactory::GFunctionWithShifts::ObjectiveFunction::operator ()(const Real& x) const {
		Real result = 0;
		for(Size i=0; i<o_.accruals_.size(); i++) {
			result += o_.accruals_[i]*o_.swapPaymentDiscounts_[i]
				*std::exp(-o_.shapedSwapPaymentTimes_[i]*x);
		}
		result *= Rs_;

		result += o_.swapPaymentDiscounts_.back()*std::exp(-o_.shapedSwapPaymentTimes_.back()*x)
			-o_.discountAtStart_;
		return result;
	}

	void GFunctionFactory::GFunctionWithShifts::ObjectiveFunction::setSwapRateValue(Real x) {
		Rs_ = x;
	}

	Real GFunctionFactory::GFunctionWithShifts::shapeOfShift(Real s) const {
		const Real x(s-swapStartTime_);
		if(meanReversion_>0) {
			return (1.-std::exp(-meanReversion_*x))/meanReversion_;
		}
		else {
			return x;
		}
	}

	Real GFunctionFactory::GFunctionWithShifts::calibrationOfShift(Real Rs){
		
		if(Rs!=tmpRs_){
			Real initialGuess; 
			//{
			//	initialGuess=calibratedShift_;
			//}
			{
				Real N=0, D=0;
				for(Size i=0; i<accruals_.size(); i++) {
					N+=accruals_[i]*swapPaymentDiscounts_[i];
					D+=accruals_[i]*shapedSwapPaymentTimes_[i]*(swapPaymentDiscounts_[i]-swapStartTime_);
				}
				N*=Rs;
				D*=Rs;
				N+=swapPaymentDiscounts_.back();
				D+=swapPaymentDiscounts_.back()*(swapPaymentDiscounts_.back()-swapStartTime_);
				initialGuess = N/D;
			}
			objectiveFunction_->setSwapRateValue(Rs);
			Brent solver;
            solver.setMaxEvaluations(1000);
            calibratedShift_ = solver.solve(*objectiveFunction_, accuracy_, initialGuess, -10., 10.); 
			tmpRs_=Rs;
        }
		return calibratedShift_;
	}

	boost::shared_ptr<GFunction> GFunctionFactory::newGFunctionWithShifts(const CMSCoupon& coupon,
                                                                          Real meanReversion) {
	    return boost::shared_ptr<GFunction>(new GFunctionWithShifts(coupon, meanReversion));
    }


	void ConundrumPricer::testUnit() {
		

        Handle<YieldTermStructure> discountingTermStructure;

		{
		        /*********************
         ***  MARKET DATA  ***
         *********************/

        Calendar calendar = TARGET();
        // uncommenting the following line generates an error
        // calendar = Tokyo();
        Date settlementDate(22, September, 2004);
        // must be a business day
        settlementDate = calendar.adjust(settlementDate);

        Integer fixingDays = 2;
        Date todaysDate = calendar.advance(settlementDate, -fixingDays, Days);
        // nothing to do with Date::todaysDate
        Settings::instance().evaluationDate() = todaysDate;


        todaysDate = Settings::instance().evaluationDate();
        std::cout << "Today: " << todaysDate.weekday()
                  << ", " << todaysDate << std::endl;

        std::cout << "Settlement date: " << settlementDate.weekday()
                  << ", " << settlementDate << std::endl;

        // deposits
        Rate d1wQuote=0.0382;
        Rate d1mQuote=0.0372;
        Rate d3mQuote=0.0363;
        Rate d6mQuote=0.0353;
        Rate d9mQuote=0.0348;
        Rate d1yQuote=0.0345;
        // FRAs
        Rate fra3x6Quote=0.037125;
        Rate fra6x9Quote=0.037125;
        Rate fra6x12Quote=0.037125;
        // futures
        Real fut1Quote=96.2875;
        Real fut2Quote=96.7875;
        Real fut3Quote=96.9875;
        Real fut4Quote=96.6875;
        Real fut5Quote=96.4875;
        Real fut6Quote=96.3875;
        Real fut7Quote=96.2875;
        Real fut8Quote=96.0875;
        // swaps
        Rate s2yQuote=0.037125;
        Rate s3yQuote=0.0398;
        Rate s5yQuote=0.0443;
        Rate s10yQuote=0.05165;
        Rate s15yQuote=0.055175;


        /********************
         ***    QUOTES    ***
         ********************/

        // SimpleQuote stores a value which can be manually changed;
        // other Quote subclasses could read the value from a database
        // or some kind of data feed.

        // deposits
        boost::shared_ptr<Quote> d1wRate(new SimpleQuote(d1wQuote));
        boost::shared_ptr<Quote> d1mRate(new SimpleQuote(d1mQuote));
        boost::shared_ptr<Quote> d3mRate(new SimpleQuote(d3mQuote));
        boost::shared_ptr<Quote> d6mRate(new SimpleQuote(d6mQuote));
        boost::shared_ptr<Quote> d9mRate(new SimpleQuote(d9mQuote));
        boost::shared_ptr<Quote> d1yRate(new SimpleQuote(d1yQuote));
        // FRAs
        boost::shared_ptr<Quote> fra3x6Rate(new SimpleQuote(fra3x6Quote));
        boost::shared_ptr<Quote> fra6x9Rate(new SimpleQuote(fra6x9Quote));
        boost::shared_ptr<Quote> fra6x12Rate(new SimpleQuote(fra6x12Quote));
        // futures
        boost::shared_ptr<Quote> fut1Price(new SimpleQuote(fut1Quote));
        boost::shared_ptr<Quote> fut2Price(new SimpleQuote(fut2Quote));
        boost::shared_ptr<Quote> fut3Price(new SimpleQuote(fut3Quote));
        boost::shared_ptr<Quote> fut4Price(new SimpleQuote(fut4Quote));
        boost::shared_ptr<Quote> fut5Price(new SimpleQuote(fut5Quote));
        boost::shared_ptr<Quote> fut6Price(new SimpleQuote(fut6Quote));
        boost::shared_ptr<Quote> fut7Price(new SimpleQuote(fut7Quote));
        boost::shared_ptr<Quote> fut8Price(new SimpleQuote(fut8Quote));
        // swaps
        boost::shared_ptr<Quote> s2yRate(new SimpleQuote(s2yQuote));
        boost::shared_ptr<Quote> s3yRate(new SimpleQuote(s3yQuote));
        boost::shared_ptr<Quote> s5yRate(new SimpleQuote(s5yQuote));
        boost::shared_ptr<Quote> s10yRate(new SimpleQuote(s10yQuote));
        boost::shared_ptr<Quote> s15yRate(new SimpleQuote(s15yQuote));


        /*********************
         ***  RATE HELPERS ***
         *********************/

        // RateHelpers are built from the above quotes together with
        // other instrument dependant infos.  Quotes are passed in
        // relinkable handles which could be relinked to some other
        // data source later.

        // deposits
        DayCounter depositDayCounter = Actual360();

        boost::shared_ptr<RateHelper> d1w(new DepositRateHelper(
            Handle<Quote>(d1wRate),
            1*Weeks, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d1m(new DepositRateHelper(
            Handle<Quote>(d1mRate),
            1*Months, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d3m(new DepositRateHelper(
            Handle<Quote>(d3mRate),
            3*Months, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d6m(new DepositRateHelper(
            Handle<Quote>(d6mRate),
            6*Months, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d9m(new DepositRateHelper(
            Handle<Quote>(d9mRate),
            9*Months, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));
        boost::shared_ptr<RateHelper> d1y(new DepositRateHelper(
            Handle<Quote>(d1yRate),
            1*Years, fixingDays,
            calendar, ModifiedFollowing, depositDayCounter));


        // setup FRAs
        boost::shared_ptr<RateHelper> fra3x6(new FraRateHelper(
            Handle<Quote>(fra3x6Rate),
            3, 6, fixingDays, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fra6x9(new FraRateHelper(
            Handle<Quote>(fra6x9Rate),
            6, 9, fixingDays, calendar, ModifiedFollowing,
            depositDayCounter));
        boost::shared_ptr<RateHelper> fra6x12(new FraRateHelper(
            Handle<Quote>(fra6x12Rate),
            6, 12, fixingDays, calendar, ModifiedFollowing,
            depositDayCounter));


        // setup futures
        Rate convexityAdjustment = 0.0;
        Integer futMonths = 3;
        Date imm = Date::nextIMMdate(settlementDate);
        boost::shared_ptr<RateHelper> fut1(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut2(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut3(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut4(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut5(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut6(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut7(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));
        imm = Date::nextIMMdate(imm+1);
        boost::shared_ptr<RateHelper> fut8(new FuturesRateHelper(
            Handle<Quote>(fut1Price),
            imm,
            futMonths, calendar, ModifiedFollowing,
            depositDayCounter, convexityAdjustment));


        // setup swaps
        Frequency swFixedLegFrequency = Annual;
        BusinessDayConvention swFixedLegConvention = Unadjusted;
        DayCounter swFixedLegDayCounter = Thirty360(Thirty360::European);
        boost::shared_ptr<Xibor> swFloatingLegIndex(new Euribor6M);

        boost::shared_ptr<RateHelper> s2y(new SwapRateHelper(
            Handle<Quote>(s2yRate),
            2*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s3y(new SwapRateHelper(
            Handle<Quote>(s3yRate),
            3*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s5y(new SwapRateHelper(
            Handle<Quote>(s5yRate),
            5*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s10y(new SwapRateHelper(
            Handle<Quote>(s10yRate),
            10*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));
        boost::shared_ptr<RateHelper> s15y(new SwapRateHelper(
            Handle<Quote>(s15yRate),
            15*Years, fixingDays,
            calendar, swFixedLegFrequency,
            swFixedLegConvention, swFixedLegDayCounter,
            swFloatingLegIndex));


        /*********************
         **  CURVE BUILDING **
         *********************/

        // Any DayCounter would be fine.
        // ActualActual::ISDA ensures that 30 years is 30.0
        DayCounter termStructureDayCounter =
            ActualActual(ActualActual::ISDA);


        double tolerance = 1.0e-15;

        // A depo-swap curve
        std::vector<boost::shared_ptr<RateHelper> > depoSwapInstruments;
        depoSwapInstruments.push_back(d1w);
        depoSwapInstruments.push_back(d1m);
        depoSwapInstruments.push_back(d3m);
        depoSwapInstruments.push_back(d6m);
        depoSwapInstruments.push_back(d9m);
        depoSwapInstruments.push_back(d1y);
        depoSwapInstruments.push_back(s2y);
        depoSwapInstruments.push_back(s3y);
        depoSwapInstruments.push_back(s5y);
        depoSwapInstruments.push_back(s10y);
        depoSwapInstruments.push_back(s15y);
        boost::shared_ptr<YieldTermStructure> depoSwapTermStructure(new
            PiecewiseFlatForward(settlementDate, depoSwapInstruments,
                                 termStructureDayCounter, tolerance));


        // A depo-futures-swap curve
        std::vector<boost::shared_ptr<RateHelper> > depoFutSwapInstruments;
        depoFutSwapInstruments.push_back(d1w);
        depoFutSwapInstruments.push_back(d1m);
        depoFutSwapInstruments.push_back(fut1);
        depoFutSwapInstruments.push_back(fut2);
        depoFutSwapInstruments.push_back(fut3);
        depoFutSwapInstruments.push_back(fut4);
        depoFutSwapInstruments.push_back(fut5);
        depoFutSwapInstruments.push_back(fut6);
        depoFutSwapInstruments.push_back(fut7);
        depoFutSwapInstruments.push_back(fut8);
        depoFutSwapInstruments.push_back(s3y);
        depoFutSwapInstruments.push_back(s5y);
        depoFutSwapInstruments.push_back(s10y);
        depoFutSwapInstruments.push_back(s15y);
        boost::shared_ptr<YieldTermStructure> depoFutSwapTermStructure(new
            PiecewiseFlatForward(settlementDate, depoFutSwapInstruments,
                                 termStructureDayCounter, tolerance));


        // A depo-FRA-swap curve
        std::vector<boost::shared_ptr<RateHelper> > depoFRASwapInstruments;
        depoFRASwapInstruments.push_back(d1w);
        depoFRASwapInstruments.push_back(d1m);
        depoFRASwapInstruments.push_back(d3m);
        depoFRASwapInstruments.push_back(fra3x6);
        depoFRASwapInstruments.push_back(fra6x9);
        depoFRASwapInstruments.push_back(fra6x12);
        depoFRASwapInstruments.push_back(s2y);
        depoFRASwapInstruments.push_back(s3y);
        depoFRASwapInstruments.push_back(s5y);
        depoFRASwapInstruments.push_back(s10y);
        depoFRASwapInstruments.push_back(s15y);
        boost::shared_ptr<YieldTermStructure> depoFRASwapTermStructure(new
            PiecewiseFlatForward(settlementDate, depoFRASwapInstruments,
                                 termStructureDayCounter, tolerance));


        // Term structures that will be used for pricing:


        // the one used for forward rate forecasting
        Handle<YieldTermStructure> forecastingTermStructure;
		Handle<YieldTermStructure> discountingTermStructure;
		discountingTermStructure.linkTo(depoFRASwapTermStructure);
		
		}

		const Handle<YieldTermStructure> yieldTermStructure = discountingTermStructure;
		const Date referenceDate = yieldTermStructure->referenceDate();

		const std::string familyName("");
        const Integer years(10);
        const Integer settlementDays(2);
		const Currency currency = EURCurrency();
        const Calendar calendar = TARGET();
        const Frequency fixedLegFrequency(Annual);
		const Period tenor(6, Months);
        const BusinessDayConvention convention(Following);
        const DayCounter dayCounter = Actual360();

		

        const boost::shared_ptr<Xibor> iborIndex(new Euribor6M);

		const boost::shared_ptr<SwapIndex> index(
			new SwapIndex(
			familyName, 
			years, 
			settlementDays, 
			currency, 
			calendar, 
			fixedLegFrequency, 
			convention, 
			dayCounter, 
			iborIndex)
			);
		const Date paymentDate(referenceDate+365);
		const Date startDate(referenceDate+180);
		const Date endDate(startDate+365);

		const Real gearing(1);
		const Real spread(0);
		const Integer fixingDays(0);

		boost::shared_ptr<VanillaCMSCouponPricer> pricer(new ConundrumPricerByNumericalIntegration);

		CMSCoupon coupon(1, paymentDate, index, startDate, endDate, fixingDays, 
			dayCounter, pricer, gearing, spread);

		std::cout << "rate:\t" << coupon.rate();
	
	}

}
