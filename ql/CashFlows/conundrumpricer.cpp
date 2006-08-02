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
                                              Real deflator) const
    {
        Real optionType = isCall ? 1.0 : -1.0;
        Real variance = volatilityStructure_->blackVariance(expiryDate,
            swapTenor_, forwardValue_); 
        return deflator * detail::blackFormula(forwardValue_, strike,
            std::sqrt(variance), optionType);
    };


    ConundrumPricer::ConundrumPricer(
                const GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve,
                const boost::shared_ptr<YieldTermStructure>& rateCurve,
                const CMSCoupon& coupon)
    : modelOfYieldCurve_(modelOfYieldCurve), rateCurve_(rateCurve),
      coupon_(coupon), cutoffForCaplet_(2), cutoffForFloorlet_(0),
      fixingDate_(coupon.fixingDate()),
      paymentDate_(coupon.date()), discount_(rateCurve_->discount(paymentDate_))
    {
		const boost::shared_ptr<SwapIndex>& swapRate = coupon_.index();
        swapTenor_ = swapRate->tenor();
        boost::shared_ptr<VanillaSwap> swap = swapRate->underlyingSwap(fixingDate_);
		swapRateValue_ = swap->fairRate();

		static const Spread basisPoSize = 1.0e-4;
        annuity_ = (swap->floatingLegBPS()/basisPoSize)/coupon_.nominal();

		min_ = coupon_.floor();
		max_ = coupon_.cap();
		gearing_ = coupon_.gearing();
		spread_ = coupon_.spread();
        
        Size q = swapRate->fixedLegFrequency();

        boost::shared_ptr<Schedule> schedule(swapRate->fixedRateSchedule(fixingDate_));

        DayCounter dc = swapRate->dayCounter();
		Time startTime = dc.yearFraction(rateCurve_->referenceDate(),
                                         swap->startDate());
		Time swapFirstPaymentTime = dc.yearFraction(rateCurve_->referenceDate(),
                                                    schedule->date(1));
		Time paymentTime = dc.yearFraction(rateCurve_->referenceDate(),
                                           paymentDate_);
		Real delta = (paymentTime-startTime) / (swapFirstPaymentTime-startTime);

        gFunction_ = GFunctionFactory::newGFunctionStandard(q, delta, swapTenor_.length());
	}

	Real ConundrumPricer::price() const
    {
		const Real swapLetPrice_ = swapLetPrice(); 
		const Real spreadLegValue = spread_*coupon_.accrualPeriod()*discount_;

		const Real effectiveStrikeForMax = (max_-spread_)/gearing_;
		Real capLetPrice = 0;

		if (max_ < cutoffForCaplet_) {
			if (effectiveStrikeForMax<=swapRateValue_) {
				capLetPrice = optionLetPrice(false, effectiveStrikeForMax) 
					+ (swapLetPrice_ - effectiveStrikeForMax*coupon_.accrualPeriod()*discount_);
			} else {
				capLetPrice = optionLetPrice(true, effectiveStrikeForMax);
			}
		}

		const Real effectiveStrikeForMin = (min_-spread_)/gearing_;
		Real floorLetPrice = 0;

		if (min_ > cutoffForFloorlet_) { 
			if (effectiveStrikeForMin<=swapRateValue_) {
				floorLetPrice = optionLetPrice(false, effectiveStrikeForMin);
			} else {
				floorLetPrice = optionLetPrice(true, effectiveStrikeForMin) - 
					(swapLetPrice_ - effectiveStrikeForMin*coupon_.accrualPeriod()*discount_);
			}
		}
		const Real price = gearing_*(swapLetPrice_ + floorLetPrice - capLetPrice) + spreadLegValue;
		return price;
	}

	Real ConundrumPricer::rate() const
    {
       return price()/(coupon_.accrualPeriod()*discount_);
	}


    	
    ////////////////		ConundrumPricerByNumericalIntegration
    ConundrumPricerByNumericalIntegration::ConundrumPricerByNumericalIntegration(
        GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve,
        const boost::shared_ptr<YieldTermStructure>& rateCurve,
        const CMSCoupon& coupon,
		const boost::shared_ptr<VanillaOptionPricer>& o)
    : ConundrumPricer(modelOfYieldCurve, rateCurve,coupon),
      vanillaOptionPricer_(o), mInfinity_(1.0)
    {
		integrandForCap_ = boost::shared_ptr<ConundrumIntegrand>(
            new ConundrumIntegrand(vanillaOptionPricer_, rateCurve_,
                                   gFunction_, fixingDate_, paymentDate_,
                                   annuity_, swapRateValue_, swapRateValue_, true));
		integrandForFloor_ = boost::shared_ptr<ConundrumIntegrand>(
            new ConundrumIntegrand(vanillaOptionPricer_, rateCurve_,
                                   gFunction_, fixingDate_, paymentDate_,
                                   annuity_, swapRateValue_, swapRateValue_, false));
	}

	Real ConundrumPricerByNumericalIntegration::integrate(Real a,
        Real b, const ConundrumIntegrand& integrand) const
    {
        // grado polinomi di Legendre - Questa variabile serve soltanto in
        // caso di GaussLegendreQuadrature
        //const Size n = 25;
		//GaussLegendre Integral(n);

        KronrodIntegral integral(1.e-12, 1000000); 
		return integral(integrand,a , b);
	}

	Real ConundrumPricerByNumericalIntegration::optionLetPrice(
                                        bool isCap, Real strike) const {
		Real integralValue, dFdK;
		if(isCap) {
			const Real a = strike; 
			const Real b = strike + mInfinity_;
            integrandForCap_->setStrike(strike);
			integralValue = integrate(a, b, *integrandForCap_);
			dFdK = integrandForCap_->firstDerivativeOfF(strike);
		}
		else {
			const Real a = 0.0; 
			const Real b = strike;
            integrandForFloor_->setStrike(strike);
			integralValue = -integrate(a, b, *integrandForFloor_);
			dFdK = integrandForFloor_->firstDerivativeOfF(strike);
		}
		const Real swaptionPrice = (*vanillaOptionPricer_)(fixingDate_,
            strike, isCap, annuity_);
		// v. HAGAN, Conundrums..., formule 2.17a, 2.18a
		return coupon_.accrualPeriod() * (discount_/annuity_) * 
            ((1 + dFdK) * swaptionPrice + integralValue);
	}

	Real ConundrumPricerByNumericalIntegration::swapLetPrice() const {
		const Real atmCapLetPrice = optionLetPrice(true, swapRateValue_);
		const Real atmFloorLetPrice = optionLetPrice(false, swapRateValue_);
		return coupon_.accrualPeriod()*(discount_ * swapRateValue_)
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

	Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::functionF (const Real x) const
    {
        const Real Gx = gFunction_->operator()(x); 
		const Real GR = gFunction_->operator()(forwardValue_); 
		return (x - strike_) * (Gx/GR - 1.0);
	}

	Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::firstDerivativeOfF (const Real x) const
    {
		const Real Gx = gFunction_->operator()(x); 
		const Real GR = gFunction_->operator()(forwardValue_) ; 
		const Real G1 = gFunction_->firstDerivative(x);
		return (Gx/GR - 1.0) + G1/GR * (x - strike_);
	}

	Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::secondDerivativeOfF (const Real x) const
    {
		const Real GR = gFunction_->operator()(forwardValue_) ; 
		const Real G1 = gFunction_->firstDerivative(x);
		const Real G2 = gFunction_->secondDerivative(x);
		return 2.0 * G1/GR + (x - strike_) * G2/GR; 
	}

	Real ConundrumPricerByNumericalIntegration::ConundrumIntegrand::operator()(Real x) const
    { 
		const Real option = (*vanillaOptionPricer_)(fixingDate_, x, isCaplet_, annuity_); 
		return option * secondDerivativeOfF(x); 
	}


    ////////////////////// ConundrumPricerByBlack
    ConundrumPricerByBlack::ConundrumPricerByBlack(
        GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve,
        const boost::shared_ptr<YieldTermStructure>& rateCurve,
        const CMSCoupon& coupon,
		const boost::shared_ptr<VanillaOptionPricer>& o)
    : ConundrumPricer(modelOfYieldCurve, rateCurve, coupon), 
      vanillaOptionPricer_(o),
      variance_(coupon.swaptionVolatility()->blackVariance(fixingDate_,
                                                           swapTenor_,
                                                           swapRateValue_)),
      firstDerivativeOfGAtForwardValue_(gFunction_->firstDerivative(
                                                        swapRateValue_)) {}

    //Hagan, 3.5b, 3.5c
    Real ConundrumPricerByBlack::optionLetPrice(bool isCall, Real strike) const
     {
	    Real price = 0;
        
	    const Real CK = (*vanillaOptionPricer_)(fixingDate_, strike, isCall, annuity_); 
	    price += (discount_/annuity_)*CK;
        const Real sqrtSigma2T = std::sqrt(variance_);
        const Real lnRoverK =  std::log(swapRateValue_/strike);
	    const Real d32 = (lnRoverK+1.5*variance_)/sqrtSigma2T;
        const Real d12 =  (lnRoverK+.5*variance_)/sqrtSigma2T;
        const Real dminus12 =  (lnRoverK-.5*variance_)/sqrtSigma2T;
	    const Real sgn = isCall ? 1.0 : -1.0;
    
        CumulativeNormalDistribution cumulativeOfNormal;
	    const Real N32 = cumulativeOfNormal(sgn*d32);
        const Real N12 = cumulativeOfNormal(sgn*d12);
        const Real Nminus12 = cumulativeOfNormal(sgn*dminus12);
    	
	    price += sgn * firstDerivativeOfGAtForwardValue_ * annuity_ *
            swapRateValue_ * (swapRateValue_ * std::exp(variance_) * N32-
            (swapRateValue_+strike) * N12 + strike * Nminus12);
	    price *= coupon_.accrualPeriod();
	    return price;
    }

    //Hagan 3.4c
    Real ConundrumPricerByBlack::swapLetPrice() const
    {
	    Real price = 0;
	    price += discount_*swapRateValue_;
        price += firstDerivativeOfGAtForwardValue_*annuity_*swapRateValue_*swapRateValue_*
            (std::exp(variance_)-1);
	    price *= coupon_.accrualPeriod();
	    return price;
    }


 
    ////////////////////// 
    Rate CMSCoupon::rate() const
    {
        Date d = fixingDate();
        const Rate forwardValue = index_->fixing(d);
        boost::shared_ptr<VanillaOptionPricer> vanillaOptionPricer( 
            new BlackVanillaOptionPricer(forwardValue, index()->tenor(),
                                         swaptionVol_));
        
        switch (typeOfConvexityAdjustment_) {
          case ConvexityAdjustmentPricer::ConundrumByBlack:
            return ConundrumPricerByBlack(
                    GFunctionFactory::standard,
                    index()->iborIndex()->termStructure(),
                    *this,
                    vanillaOptionPricer).rate();
              //return rate1();
          case ConvexityAdjustmentPricer::ConundrumByNumericalIntegration:
            return ConundrumPricerByNumericalIntegration(
                    GFunctionFactory::standard,
                    index()->iborIndex()->termStructure(),
                    *this,
                    vanillaOptionPricer).rate();
          default:
            QL_FAIL("invalid ConvexityAdjustemPricer type");
        }
    }

    //Rate CMSCoupon::convexityAdjustment(Rate fixing) const {
    //    return pricer_->rate(fixing, swaptionVol_, *this)-(gearing*fixing+spread);
    //    return pricer_->adjustment(fixing, swaptionVol_, *this);
    //}
    Real GFunctionFactory::GFunctionStandard::operator()(Real x)
    {
	    const Real n = swapLength_ * q_; 
	    return x / std::pow((1.0 + x/q_), delta_) * 1.0 /
            (1.0 - 1.0 / std::pow((1.0 + x/q_), n));
    }

    Real GFunctionFactory::GFunctionStandard::firstDerivative(Real x)
    {
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

    Real GFunctionFactory::GFunctionStandard::secondDerivative(Real x)
    {
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
                                                            Real delta, Size swapLength)
    {
	    return boost::shared_ptr<GFunction>(new GFunctionStandard(q, delta, swapLength));
    }

	Real GFunctionFactory::GFunctionWithShifts::operator()(Real R)
    {
		return R*firstDerivative(R);
    }

	Real GFunctionFactory::GFunctionWithShifts::firstDerivative(Real R)
    {
		return std::exp(-paymentTime_*shift_) 
			/ (1-discountRatio_*std::exp(-swapEndTime_*shift_));
    }

	Real GFunctionFactory::GFunctionWithShifts::secondDerivative(Real R)
    {
		return 0;
    }

	GFunctionFactory::GFunctionWithShifts::GFunctionWithShifts(boost::shared_ptr<CMSCoupon> coupon, 
		Real meanReversion) : meanReversion_(meanReversion) {

		const boost::shared_ptr<SwapIndex> swapRate = coupon->index();
        const boost::shared_ptr<VanillaSwap> swap = swapRate->underlyingSwap(coupon->fixingDate());
		swapRateValue_ = swap->fairRate();
		const std::vector<boost::shared_ptr<CashFlow> > fixedLeg = swap->fixedLeg();

		const boost::shared_ptr<Schedule> schedule(swapRate->fixedRateSchedule(coupon->fixingDate()));
		

		const boost::shared_ptr<YieldTermStructure> rateCurve = swapRate->iborIndex()->termStructure();
        const DayCounter dc = swapRate->dayCounter();
		swapStartTime_ = dc.yearFraction(rateCurve->referenceDate(), schedule->startDate());
		paymentTime_ = dc.yearFraction(rateCurve->referenceDate(),
                                           coupon->date());
		
		for(Size i=0; i<fixedLeg.size(); i++) {
			accruals_.push_back(static_cast<const Coupon*>(fixedLeg[i].get())->accrualPeriod());
			const Date paymentDate = static_cast<const Coupon*>(fixedLeg[i].get())->date();
			swapPaymentTimes_.push_back(dc.yearFraction(rateCurve->referenceDate(), paymentDate));
			swapPaymentDiscounts_.push_back(rateCurve->discount(paymentDate));
		}
		swapEndTime_ = swapPaymentTimes_.back();
		discountRatio_ = swapPaymentDiscounts_.back()/swapPaymentDiscounts_.front();
		
		std::auto_ptr<ObjectiveFunction> objectiveFunction(new ObjectiveFunction(*this));

		Bisection solver;
		shift_ = solver.solve(*objectiveFunction, .00001, .03, .1); // ????
	}

	Real GFunctionFactory::GFunctionWithShifts::ObjectiveFunction::operator ()(const Real& x) const {
		Real result = 0;
		for(Size i=0; i<o_.accruals_.size(); i++) {
			result += o_.accruals_[i]*o_.swapPaymentDiscounts_[i]*std::exp((o_.swapPaymentTimes_[i]*o_.shift_));
		}
		result *=o_.swapRateValue_;

		result += o_.swapPaymentDiscounts_.back()*std::exp(o_.swapPaymentTimes_.back()*o_.shift_)
			- o_.swapPaymentDiscounts_.front();
		return result;
	}

	Real GFunctionFactory::GFunctionWithShifts::shape(Real s) const {
		if(meanReversion_>0) {
			return (1.-std::exp(-meanReversion_*(s-swapStartTime_)))/meanReversion_;
		}
		else
			return s-swapStartTime_;
	}
}
