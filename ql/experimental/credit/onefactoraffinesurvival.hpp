/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Jose Aparicio

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef onefactor_affine_survival_hpp
#define onefactor_affine_survival_hpp

#include <ql/models/shortrate/onefactormodel.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/termstructures/credit/hazardratestructure.hpp>
#include <utility>

namespace QuantLib {
    
    /*! Survival probability term structure based on a one factor stochastic
    model of the default intensity.
    */
    /*
    While deriving from the hazard rate class the HRTS refers only to the
    deterministic part of the model. The probabilities depend on this 
    component and the stochastic part and are rewritten here.
    Derived classes need to specify the deterministic part 
    of the hazard rate if any (the one returned by 'hazardRateImpl'). It
    is needed for the conditional/forward probabilities.
     */
    class OneFactorAffineSurvivalStructure 
        : public HazardRateStructure {
    public:
        // implement remaining constructors.....
      explicit OneFactorAffineSurvivalStructure(
          ext::shared_ptr<OneFactorAffineModel> model,
          const DayCounter& dayCounter = DayCounter(),
          const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
          const std::vector<Date>& jumpDates = std::vector<Date>())
      : HazardRateStructure(dayCounter, jumps, jumpDates), model_(std::move(model)) {}

      OneFactorAffineSurvivalStructure(
          ext::shared_ptr<OneFactorAffineModel> model,
          const Date& referenceDate,
          const Calendar& cal = Calendar(),
          const DayCounter& dayCounter = DayCounter(),
          const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
          const std::vector<Date>& jumpDates = std::vector<Date>())
      : HazardRateStructure(referenceDate, Calendar(), dayCounter, jumps, jumpDates),
        model_(std::move(model)) {}

      OneFactorAffineSurvivalStructure(
          ext::shared_ptr<OneFactorAffineModel> model,
          Natural settlementDays,
          const Calendar& calendar,
          const DayCounter& dayCounter = DayCounter(),
          const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
          const std::vector<Date>& jumpDates = std::vector<Date>())
      : HazardRateStructure(settlementDays, calendar, dayCounter, jumps, jumpDates),
        model_(std::move(model)) {}

      //! \name TermStructure interface
      //@{
      // overwrite on mkt models (e.g. bootstraps)
      Date maxDate() const override { return Date::maxDate(); }

      /* Notice this is not calling hazard rate methods, these are
         stochastic now.
      */
      /*!
        Returns the probability at a future time dTgt, conditional to
        survival at a prior time dFwd and to the realization of a particular
        hazard rate value at dFwd.
        \param dFwd Time of the forward survival calculation and HR
                    realization.
        \param dTgt Target time of survival probability.
        \param yVal Realized value of the HR at time dFwd.
        \param extrapolate Allow curve extrapolation.
        \return Survival probability.

        \todo turn into a protected method to be called by
              defaults and survivals
      */
      /*
        Note: curve extrapolation has a different meaning on different curve
          types; for matched to market structures the credit market curves
          would be requested for extrapolation; for affine models on top of
          a static term structure it is this one that will be required for
          extrapolation.
       */
      Probability conditionalSurvivalProbability(const Date& dFwd,
                                                 const Date& dTgt,
                                                 Real yVal,
                                                 bool extrapolate = false) const {
          return conditionalSurvivalProbability(timeFromReference(dFwd), timeFromReference(dTgt),
                                                yVal, extrapolate); 
        }
        Probability conditionalSurvivalProbability(
                Time tFwd, Time tgt, Real yVal,
                bool extrapolate = false) const
        {
            #if defined(QL_EXTRA_SAFETY_CHECKS)
                QL_REQUIRE(tgt >= tFwd, "Incorrect dates ordering.");
            #endif
            checkRange(tFwd, extrapolate);
            checkRange(tgt, extrapolate);
            
            // \todo ADD JUMPS TREATMENT

            return conditionalSurvivalProbabilityImpl(tFwd, tgt, yVal);
        }
        //@}
        // DefaultTermStructure interface
        using DefaultProbabilityTermStructure::hazardRate;
        Rate hazardRate(Time t, bool extrapolate = false) const {
            checkRange(t, extrapolate);
            return hazardRateImpl(t);
        }
    protected:
        //! \name DefaultProbabilityTermStructure implementation
        //@{
      Probability survivalProbabilityImpl(Time) const override;
      Real defaultDensityImpl(Time) const override;
      //@}
      // avoid call super
      // \todo addd date overload
      virtual Probability conditionalSurvivalProbabilityImpl(Time tFwd, Time tgt, Real yVal) const;

      // HazardRateStructure interface
      Real hazardRateImpl(Time) const override {
          // no deterministic component
          return 0.;
      }

        ext::shared_ptr<OneFactorAffineModel> model_;        
    };
    
    inline Probability
        OneFactorAffineSurvivalStructure::survivalProbabilityImpl(
        Time t) const
    {
        Real initValHR =
            model_->dynamics()->shortRate(0., 
                model_->dynamics()->process()->x0());

        return model_->discountBond(0., t, initValHR);
    }

    inline Probability
        OneFactorAffineSurvivalStructure::conditionalSurvivalProbabilityImpl(
            Time tFwd, Time tgt, Real yVal) const {
        return model_->discountBond(tFwd, tgt, yVal);
    }

    inline Real 
        OneFactorAffineSurvivalStructure::defaultDensityImpl(Time t) const {
        Real initValHR = 
            model_->dynamics()->shortRate(0., 
                model_->dynamics()->process()->x0());;

        return hazardRateImpl(t)*survivalProbabilityImpl(t) /
            model_->discountBond(0., t, initValHR);
    }
}

#endif
