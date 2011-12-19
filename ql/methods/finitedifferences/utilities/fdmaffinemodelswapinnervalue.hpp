/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdmaffinemodelswapinnervalue.hpp
*/

#ifndef quantlib_fdm_affine_model_swap_inner_value_hpp
#define quantlib_fdm_affine_model_swap_inner_value_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/utilities/fdmaffinemodeltermstructure.hpp>

#include <map>

namespace QuantLib {

    template <class ModelType>
    class FdmAffineModelSwapInnerValue : public FdmInnerValueCalculator {
      public:
        FdmAffineModelSwapInnerValue(
            const boost::shared_ptr<ModelType>& disModel,
            const boost::shared_ptr<ModelType>& fwdModel,
            const boost::shared_ptr<VanillaSwap>& swap,
            const std::map<Time, Date>& exerciseDates,
            const boost::shared_ptr<FdmMesher>& mesher,
            Size direction);

        Real innerValue(const FdmLinearOpIterator& iter, Time t);
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t);

      private:
        Disposable<Array> getState(
            const boost::shared_ptr<ModelType>& model,
            Time t,
            const FdmLinearOpIterator& iter) const;

        RelinkableHandle<YieldTermStructure> disTs_, fwdTs_;
        const boost::shared_ptr<ModelType> disModel_, fwdModel_;

        const boost::shared_ptr<IborIndex> index_;
        const boost::shared_ptr<VanillaSwap> swap_;
        const std::map<Time, Date> exerciseDates_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const Size direction_;
    };

    template <class ModelType> inline
    FdmAffineModelSwapInnerValue<ModelType>::FdmAffineModelSwapInnerValue(
        const boost::shared_ptr<ModelType>& disModel,
        const boost::shared_ptr<ModelType>& fwdModel,
        const boost::shared_ptr<VanillaSwap>& swap,
        const std::map<Time, Date>& exerciseDates,
        const boost::shared_ptr<FdmMesher>& mesher,
        Size direction)
    : disModel_(disModel),
      fwdModel_(fwdModel),
      index_(swap->iborIndex()),
      swap_(boost::shared_ptr<VanillaSwap>(
          new VanillaSwap(swap->type(),
                          swap->nominal(),
                          swap->fixedSchedule(),
                          swap->fixedRate(),
                          swap->fixedDayCount(),
                          swap->floatingSchedule(),
                          swap->iborIndex()->clone(fwdTs_),
                          swap->spread(),
                          swap->floatingDayCount(),
                          swap->paymentConvention()))),
      exerciseDates_(exerciseDates),
      mesher_(mesher),
      direction_(direction) {
    }

    template <class ModelType> inline
    Real FdmAffineModelSwapInnerValue<ModelType>::innerValue(
        const FdmLinearOpIterator& iter, Time t) {

        const Date& iterExerciseDate = exerciseDates_.find(t)->second;

        const Array disRate(getState(disModel_, t, iter));
        const Array fwdRate(getState(fwdModel_, t, iter));

        if (disTs_.empty() || iterExerciseDate != disTs_->referenceDate()) {

            const Handle<YieldTermStructure> discount
                = disModel_->termStructure();

            disTs_.linkTo(boost::shared_ptr<YieldTermStructure>(
                new FdmAffineModelTermStructure(disRate,
                    discount->calendar(), discount->dayCounter(),
                    iterExerciseDate, discount->referenceDate(),
                    disModel_)));

            const Handle<YieldTermStructure> fwd = fwdModel_->termStructure();

            fwdTs_.linkTo(boost::shared_ptr<YieldTermStructure>(
                new FdmAffineModelTermStructure(fwdRate,
                    fwd->calendar(), fwd->dayCounter(),
                    iterExerciseDate, fwd->referenceDate(),
                    fwdModel_)));

            swap_->setPricingEngine(boost::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(
                    disTs_, boost::none,
                    iterExerciseDate+index_->fixingDays(),
                    iterExerciseDate)));
        }
        else {
            boost::dynamic_pointer_cast<FdmAffineModelTermStructure>(
                disTs_.currentLink())->setVariable(disRate);
            boost::dynamic_pointer_cast<FdmAffineModelTermStructure>(
                fwdTs_.currentLink())->setVariable(fwdRate);
        }

        return std::max(0.0, swap_->NPV());
    }

    template <class ModelType> inline
    Real FdmAffineModelSwapInnerValue<ModelType>::avgInnerValue(
        const FdmLinearOpIterator& iter, Time t) {
        return innerValue(iter, t);
    }

}
#endif
