/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2018 Sebastian Schlenkrich

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


#ifndef quantlib_vanillalocalvolsmilesection_hpp
#define quantlib_vanillalocalvolsmilesection_hpp

#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/math/optimization/method.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>

#include <ql/experimental/vanillalocalvolmodel/vanillalocalvolmodel.hpp>

namespace QuantLib {

	class VanillaLocalVolModelSmileSection : public SmileSection {
	private:
		ext::shared_ptr<VanillaLocalVolModel>  model_;
	protected:
		virtual Volatility volatilityImpl(Rate strike) const;
	public:
		VanillaLocalVolModelSmileSection(
			const ext::shared_ptr<VanillaLocalVolModel>&    model,
			const DayCounter&                               dc = DayCounter(),
			const VolatilityType                            type = Normal,
			const Rate                                      shift = 0.0)
			: SmileSection(model->timeToExpiry(), dc, type, shift), model_(model) {}

		VanillaLocalVolModelSmileSection(
			const Date&                                   expiryDate,
			const Rate&                                   forward,
			const std::vector<Rate>&                      relativeStrikes,
			const std::vector<Volatility>&                smileVolatilities,
			const Real                                    extrapolationRelativeStrike,
			const Real                                    extrapolationSlope,
			bool                                          vegaWeighted = false,
			const ext::shared_ptr<EndCriteria>&           endCriteria = ext::shared_ptr<EndCriteria>(new EndCriteria(100, 10, 1.0e-6, 1.0e-6, 1.0e-6)),
			const ext::shared_ptr<OptimizationMethod>&    method = ext::shared_ptr<OptimizationMethod>(new LevenbergMarquardt(1.0e-6, 1.0e-6, 1.0e-6)),
			const DayCounter&                             dc = Actual365Fixed(),
			const Date&                                   referenceDate = Date(),
			const VolatilityType                          type = Normal,
			const Rate                                    shift = 0.0,
			const ext::shared_ptr<VanillaLocalVolModel>&  model = ext::shared_ptr<VanillaLocalVolModel>(),
			const Real                                    minSlope = -3.0,   //  lower boundary for m in calibration
			const Real                                    maxSlope =  3.0,   //  upper boundary for m in calibration
			const Real                                    alpha = 1.0e-4);   //  Tikhonov alpha

		VanillaLocalVolModelSmileSection(
			const Date&                                       expiryDate,
			const Rate&                                       forward,
			const Volatility&                                 atmVolatility,
			const ext::shared_ptr<VanillaLocalVolModelSmileSection>& smile1,
			const ext::shared_ptr<VanillaLocalVolModelSmileSection>& smile2,
			const Real&                                       rho,
			const bool                                        calcSimple = true,  // use only ATM vol for x-grid calculation
			const DayCounter&                                 dc = Actual365Fixed(),
			const Date&                                       referenceDate = Date(),
			const VolatilityType                              type = Normal,
			const Rate                                        shift = 0.0);

		// SmileSection interface 
		virtual Real minStrike() const { return model_->underlyingS().front(); }
		virtual Real maxStrike() const { return model_->underlyingS().back();  }
		virtual Real atmLevel()  const { return model_->forward();             }

		// overload optionPrice() as a basis for implied volatility calculation
		virtual Real optionPrice(Rate strike, Option::Type type = Option::Call, Real discount = 1.0) const;

		// inspector
		inline const ext::shared_ptr<VanillaLocalVolModel>&  model() const { return model_; }
	};
}

#endif  /* ifndef quantlib_vanillalocalvolsmilesection_hpp */
