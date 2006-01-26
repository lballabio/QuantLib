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
    \brief Binomial Tsiveriotis-Fernandes tree model
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
                      const Array& spreadAdjustedRate,
                      Array& newValues, Array& newConversionProbability,
                      Array& newSpreadAdjustedRate) const;
        void rollback(DiscretizedAsset&, Time to) const;
        void partialRollback(DiscretizedAsset&, Time to) const;

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
              const Array& spreadAdjustedRate, Array& newValues,
              Array& newConversionProbability,
              Array& newSpreadAdjustedRate) const {

        for (Size j=0; j<this->size(i); j++) {

            // new conversion probability is calculated via backward
            // induction using up and down probabilities on tree on
            // previous conversion probabilities, ie weighted average
            // of previous probabilities.
            newConversionProbability[j] =
                pd_*conversionProbability[j]+ pu_*conversionProbability[j+1];

            // Use blended discounting rate
            newSpreadAdjustedRate[j] =
                newConversionProbability[j] * riskFreeRate_ +
                (1-newConversionProbability[j])*(riskFreeRate_+creditSpread_);

			// Integer timeLength = Integer(this->t_[i]*365);

			//Date periodDate =
            // T::calendar.advance(T::arguments_.settlementDays,
            //                     timeLength, Days);

			// Real accruedInterest = T::accruedAmount(periodDate);

            //Holding Value ie if not callable or puttable, add
            //accrued Interest if any.
            //newValues[j] =
            //    (pd_*(values[j]+accruedInterest)/(1+(spreadAdjustedRate[j]*dt_)))
            //  + (pu_*(values[j+1] + accruedInterest)/(1+(spreadAdjustedRate[j+1]*dt_)));
            newValues[j] =
                (pd_*values[j]/(1+(spreadAdjustedRate[j]*dt_)))
              + (pu_*values[j+1]/(1+(spreadAdjustedRate[j+1]*dt_)));

        }
    }

    template <class T>
	void TsiveriotisFernandesLattice<T>::rollback(
                                     DiscretizedAsset& asset, Time to) const {
        partialRollback(asset,to);
        asset.adjustValues();
    }


	template <class T>
	void TsiveriotisFernandesLattice<T>::partialRollback(
                                     DiscretizedAsset& asset, Time to) const {

        Time from = asset.time();

        if (close(from,to))
            return;

        QL_REQUIRE(from > to,
                   "cannot roll the asset back to" << to
                   << " (it is already at t = " << from << ")");

        DiscretizedConvertible& convertible =
            dynamic_cast<DiscretizedConvertible&>(asset);

        Integer iFrom = Integer(this->t_.index(from));
        Integer iTo = Integer(this->t_.index(to));

        for (Integer i=iFrom-1; i>=iTo; i--) {

            Array newValues(this->size(i));
            Array newSpreadAdjustedRate(this->size(i));
            Array newConversionProbability(this->size(i));

            stepback(i, convertible.values(),
                     convertible.conversionProbability(),
                     convertible.spreadAdjustedRate(), newValues,
                     newConversionProbability,newSpreadAdjustedRate);

            convertible.time() = this->t_[i];
            convertible.values() = newValues;
            convertible.spreadAdjustedRate() = newSpreadAdjustedRate;
            convertible.conversionProbability() = newConversionProbability;

            // skip the very last adjustment
            if (i != iTo)
                convertible.adjustValues();
        }
    }

}


#endif
