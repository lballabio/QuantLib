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
#include <ql/Solvers1D/all.hpp>

namespace QuantLib
{
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

        gFunction_ = GFunctionFactory::newGFunctionStandard(q, delta, swapTenor_.length());

        vanillaOptionPricer_= boost::shared_ptr<VanillaOptionPricer>( 
            new BlackVanillaOptionPricer(swapRateValue_, swapTenor_,
                                         coupon_->swaptionVolatility().currentLink())); 
        
	}

	Real ConundrumPricer::price() const {

		const Real swapLetPrice_ = swapLetPrice();
		const Real spreadLegValue = spread_*coupon_->accrualPeriod()*discount_;

		const Real effectiveStrikeForMax = (max_-spread_)/gearing_;
		Real capLetPrice = 0;

		if (max_ < cutoffForCaplet_) {
			capLetPrice = optionLetPrice(true, effectiveStrikeForMax);
		}

		const Real effectiveStrikeForMin = (min_-spread_)/gearing_;
		Real floorLetPrice = 0;


		if (min_ > cutoffForFloorlet_) {
			floorLetPrice = optionLetPrice(false, effectiveStrikeForMin);
		}
		const Real price = gearing_*(swapLetPrice_ + floorLetPrice - capLetPrice) + spreadLegValue;
		return price;
	}

	Real ConundrumPricer::rate() const
    {
       return price()/(coupon_->accrualPeriod()*discount_);

	}

    ////////////////		ConundrumPricerByNumericalIntegration

    ConundrumPricerByNumericalIntegration::ConundrumPricerByNumericalIntegration(
        GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve)
    : ConundrumPricer(modelOfYieldCurve),
       mInfinity_(1.0)
    {	}

	Real ConundrumPricerByNumericalIntegration::integrate(Real a,
        Real b, const ConundrumIntegrand& integrand) const {
        // grado polinomi di Legendre - Questa variabile serve soltanto in
        // caso di GaussLegendreQuadrature
        //const Size n = 25;
		//GaussLegendre Integral(n);

        const KronrodIntegral integral(1.e-12, 1000000);
		return integral(integrand,a , b);
	}

	Real ConundrumPricerByNumericalIntegration::optionLetPrice(
                                        bool isCap, Real strike) const {
		Real integralValue, dFdK;
		if(isCap) {
			const Real a = strike;
			const Real b = strike + mInfinity_;
            boost::shared_ptr<ConundrumIntegrand> integrandForCap = boost::shared_ptr<ConundrumIntegrand>(
            new ConundrumIntegrand(vanillaOptionPricer_, rateCurve_,
                                   gFunction_, fixingDate_, paymentDate_,
                                   annuity_, swapRateValue_, strike, true));

            integralValue = integrate(a, b, *integrandForCap);

			dFdK = integrandForCap->firstDerivativeOfF(strike);
		}
		else {
			const Real a = 0.0;
			const Real b = strike;
            boost::shared_ptr<ConundrumIntegrand> integrandForFloor = boost::shared_ptr<ConundrumIntegrand>(
            new ConundrumIntegrand(vanillaOptionPricer_, rateCurve_,
                                   gFunction_, fixingDate_, paymentDate_,
                                   annuity_, swapRateValue_, strike, false));
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

	//////////////////		ConundrumIntegrand
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


    ////////////////////// ConundrumPricerByBlack
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

    //////////////////////

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

	Real GFunctionFactory::GFunctionWithShifts::operator()(Real R) {
		return R*firstDerivative(R);
    }

	Real GFunctionFactory::GFunctionWithShifts::firstDerivative(Real) {
		return std::exp(-shapedPaymentTime_*shift_)
			/ (1.-discountRatio_*std::exp(-shapedSwapPaymentTimes_.back()*shift_));
    }

	Real GFunctionFactory::GFunctionWithShifts::secondDerivative(Real) {
		return 0;
    }

	GFunctionFactory::GFunctionWithShifts::GFunctionWithShifts(const boost::shared_ptr<CMSCoupon>& coupon, 
		Real meanReversion) : meanReversion_(meanReversion) {

		const boost::shared_ptr<SwapIndex>& swapIndex = coupon->swapIndex();
        const boost::shared_ptr<VanillaSwap>& swap = swapIndex->underlyingSwap(coupon->fixingDate());

		swapRateValue_ = swap->fairRate();

		const std::vector<boost::shared_ptr<CashFlow> > fixedLeg(swap->fixedLeg());
		const boost::shared_ptr<Schedule> schedule(swapIndex->fixedRateSchedule(coupon->fixingDate()));
		const boost::shared_ptr<YieldTermStructure> rateCurve(swapIndex->termStructure());
        const DayCounter dc(swapIndex->dayCounter());

		swapStartTime_ = dc.yearFraction(rateCurve->referenceDate(), schedule->startDate());
		discountAtStart_ = rateCurve->discount(schedule->startDate());

		const Real paymentTime(dc.yearFraction(rateCurve->referenceDate(), coupon->date()));

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
		/** calibration of shift */
		{
			const ObjectiveFunction objectiveFunction(*this);
			const Bisection solver;
			accuracy_ = .0000001;
			shift_ = solver.solve(objectiveFunction, accuracy_, .03, .1); // ????
		}
	}

	Real GFunctionFactory::GFunctionWithShifts::ObjectiveFunction::operator ()(const Real& x) const {
		Real result = 0;
		for(Size i=0; i<o_.accruals_.size(); i++) {
			result += o_.accruals_[i]*o_.swapPaymentDiscounts_[i]
				*std::exp(-o_.shapedSwapPaymentTimes_[i]*o_.shift_);
		}
		result *= o_.swapRateValue_;

		result += o_.swapPaymentDiscounts_.back()*std::exp(-o_.shapedSwapPaymentTimes_.back()*o_.shift_)
			-o_.discountAtStart_;
		return result;
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
}
