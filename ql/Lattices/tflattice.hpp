/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Theo Boafo

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file tflattice.hpp
    \brief Binomial trees under the TsiveriotisFernandes Binomial tree model
*/

#ifndef quantlib_lattices_tf_lattice_hpp
#define quantlib_lattices_tf_lattice_hpp

#include <ql/Lattices/bsmlattice.hpp>
#include <ql/PricingEngines/Hybrid/discretizedconvertible.hpp>
#include <ql/daycounter.hpp>
#include <ql/calendar.hpp>

namespace QuantLib {

    //! Binomial lattice approximating the Tsiveriotis-Fernandes model
    /*! \ingroup lattices */
	template <class T>
    class TsiveriotisFernandesLattice : public BlackScholesLattice<T> {
      public:
        TsiveriotisFernandesLattice(const boost::shared_ptr<T>& tree,
                                    Rate riskFreeRate,
                                    Time end,
                                    Size steps,
                                    Real creditSpread,
                                    Volatility volatility,
                                    Spread divYield);

        Rate riskFreeRate() const { return riskFreeRate_; };
        Real creditSpread() const { return creditSpread_; };
        Real dt() const {return dt_;};

      protected:
        void stepback(Size i, const Array& values,
                      const Array& conversionProbability,
                      const Array& spreadAdjustRate,
                      Array& newValues, Array& newConversionProbability,
                      Array& newSpreadAdjustRate) const;
        void rollback(DiscretizedConvertible&, Time to) const;
        void partialRollback(DiscretizedConvertible&, Time to) const;

      private:
        Real pd_, pu_,creditSpread_,dt_;
        Rate riskFreeRate_;
    };


	// template definitions

    template <class T>
    TsiveriotisFernandesLattice<T>::TsiveriotisFernandesLattice(
                                             const boost::shared_ptr<T>& tree,
                                             Rate riskFreeRate,
                                             Time end, Size steps,
                                             Real creditSpread,
                                             Volatility sigma, Spread divYield)
    : BlackScholesLattice<T>(tree,riskFreeRate,end,steps) {

        dt_ = end/steps;

        Real a = std::exp((riskFreeRate-divYield)*dt_);

        Real u = std::exp(sigma*(std::sqrt(dt_)));

        Real d = 1/u;

        pu_ = (a-d)/(u-d);
        pd_ = 1 - pu_;


        riskFreeRate_ = riskFreeRate;
        creditSpread_ = creditSpread;

        QL_REQUIRE(pu_<=1.0, "negative probability");
        QL_REQUIRE(pu_>=0.0, "negative probability");
    }

    template <class T>
    void TsiveriotisFernandesLattice<T>::stepback(
              Size i, const Array& values, const Array& conversionProbability,
              const Array& spreadAdjustRate, Array& newValues,
              Array& newConversionProbability,
              Array& newSpreadAdjustRate) const {

        for (Size j=0; j<T::size(i); j++) {

            // new conversion probability is calculated via backward
            // induction using up and down probabilities on tree on
            // previous conversion probabilities, ie weighted average
            // of previous probabilities.
            newConversionProbability[j] =
                pd_*conversionProbability[j]+ pu_*conversionProbability[j+1];

            // Use blended discounting rate
            newSpreadAdjustRate[j] =
                newConversionProbability[j] * riskFreeRate_ +
                (1-newConversionProbability[j])*(riskFreeRate_+creditSpread_);

			Integer timeLength = Integer(T::t_[i]*T::arguments_.dayCounter);

			Date periodDate = T::calendar.advance(T::arguments_.settlementDays,
                                                  timeLength, Days);

			Real accruedInterest = cvbond_->accruedAmount(periodDate);

            //Holding Value ie if not callable or puttable, add
            //accrued Interest if any.
            newValues[j] =
                (pd_*(values[j]+accruedInterest)/(1+(spreadAdjustRate[j]*dt_)))
              + (pu_*(values[j+1] + accruedInterest)/(1+(spreadAdjustRate[j+1]*dt_)));

        }
    }

    template <class T>
	void TsiveriotisFernandesLattice<T>::rollback(
                               DiscretizedConvertible& asset, Time to) const {
        partialRollback(asset,to);
        asset.adjustValues();
    }


	template <class T>
	void TsiveriotisFernandesLattice<T>::partialRollback(
                               DiscretizedConvertible& asset, Time to) const {

        Time from = asset.time();

        if (close(from,to))
            return;

        QL_REQUIRE(from > to,
                   "cannot roll the asset back to" << to
                   << " (it is already at t = " << from << ")");

        Integer iFrom = Integer(T::t_.findIndex(from));
        Integer iTo = Integer(T::t_.findIndex(to));

        for (Integer i=iFrom-1; i>=iTo; i--) {

            Array newValues(T::size(i));
            Array newSpreadAdjustRate(T::size(i));
            Array newConversionProbability(T::size(i));

            stepback(i, asset.values(), asset.conversionProbability(),
                     asset.spreadAdjustRate(), newValues,
                     newConversionProbability,newSpreadAdjustRate);

            asset.time() = T::t_[i];
            asset.values() = newValues;
            asset.spreadAdjustRate()=newSpreadAdjustRate;
            asset.conversionProbability()=newConversionProbability;


            // skip the very last adjustment
            if (i != iTo)
                asset.adjustValues();
        }
    }



}


#endif
