/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Theo Boafo
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file tflattice.hpp
    \brief Binomial Tsiveriotis-Fernandes tree model
*/

#ifndef quantlib_lattices_tf_lattice_hpp
#define quantlib_lattices_tf_lattice_hpp

#include <ql/methods/lattices/bsmlattice.hpp>
#include <ql/pricingengines/bond/discretizedconvertible.hpp>

namespace QuantLib {

    //! Binomial lattice approximating the Tsiveriotis-Fernandes model
    /*! At this time, this lattice only works with the DiscretizedConvertible class.

        \ingroup lattices
    */
    template <class T>
    class TsiveriotisFernandesLattice : public BlackScholesLattice<T> {
      public:
        TsiveriotisFernandesLattice(const ext::shared_ptr<T>& tree,
                                    Rate riskFreeRate,
                                    Time end,
                                    Size steps,
                                    Spread creditSpread,
                                    Volatility volatility,
                                    Spread divYield);

        Spread creditSpread() const { return creditSpread_; };

      protected:
        void stepback(Size i,
                      const Array& values,
                      const Array& conversionProbability,
                      const Array& spreadAdjustedRate,
                      Array& newValues,
                      Array& newConversionProbability,
                      Array& newSpreadAdjustedRate) const;
        void rollback(DiscretizedAsset&, Time to) const override;
        void partialRollback(DiscretizedAsset&, Time to) const override;

      private:
        Spread creditSpread_;
    };


    // template definitions

    template <class T>
    TsiveriotisFernandesLattice<T>::TsiveriotisFernandesLattice(
                                             const ext::shared_ptr<T>& tree,
                                             Rate riskFreeRate,
                                             Time end,
                                             Size steps,
                                             Spread creditSpread,
                                             Volatility sigma,
                                             Spread divYield)
    : BlackScholesLattice<T>(tree, riskFreeRate, end, steps),
      creditSpread_(creditSpread) {
        QL_REQUIRE(this->pu_<=1.0,
                   "probability (" << this->pu_ << ") higher than one");
        QL_REQUIRE(this->pu_>=0.0,
                   "negative (" << this->pu_ << ") probability");
    }

    template <class T>
    void TsiveriotisFernandesLattice<T>::stepback(
                                          Size i,
                                          const Array& values,
                                          const Array& conversionProbability,
                                          const Array& spreadAdjustedRate,
                                          Array& newValues,
                                          Array& newConversionProbability,
                                          Array& newSpreadAdjustedRate) const {

        for (Size j=0; j<this->size(i); j++) {

            // new conversion probability is calculated via backward
            // induction using up and down probabilities on tree on
            // previous conversion probabilities, ie weighted average
            // of previous probabilities.
            newConversionProbability[j] =
                this->pd_*conversionProbability[j] +
                this->pu_*conversionProbability[j+1];

            // Use blended discounting rate
            newSpreadAdjustedRate[j] =
                newConversionProbability[j] * this->riskFreeRate_ +
                (1-newConversionProbability[j])*(this->riskFreeRate_+creditSpread_);

            newValues[j] =
                (this->pd_*values[j]/(1+(spreadAdjustedRate[j]*this->dt_)))
              + (this->pu_*values[j+1]/(1+(spreadAdjustedRate[j+1]*this->dt_)));

        }
    }

    template <class T>
    void TsiveriotisFernandesLattice<T>::rollback(DiscretizedAsset& asset,
                                                  Time to) const {
        partialRollback(asset,to);
        asset.adjustValues();
    }


    template <class T>
    void TsiveriotisFernandesLattice<T>::partialRollback(DiscretizedAsset& asset,
                                                         Time to) const {

        Time from = asset.time();

        if (close(from,to))
            return;

        QL_REQUIRE(from > to,
                   "cannot roll the asset back to" << to
                   << " (it is already at t = " << from << ")");

        auto& convertible = dynamic_cast<DiscretizedConvertible&>(asset);

        auto iFrom = Integer(this->t_.index(from));
        auto iTo = Integer(this->t_.index(to));

        for (Integer i=iFrom-1; i>=iTo; --i) {

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
