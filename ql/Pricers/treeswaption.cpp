/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file treeswaption.cpp
    \brief European swaption calculated using finite differences

    \fullpath
    ql/Pricers/%treeswaption.cpp
*/

// $Id$

#include "ql/Pricers/treeswaption.hpp"
#include "ql/InterestRateModelling/onefactormodel.hpp"
#include "ql/asset.hpp"

namespace QuantLib {

    namespace Pricers {

        using namespace InterestRateModelling;
        using namespace Lattices;

        class DiscountBondAsset : public Asset {
          public:
            DiscountBondAsset() {}
            void reset(Size size) {
                values_ = Array(size, 1.0);
            }
        };

        class SwapAsset : public Asset {
          public:
            SwapAsset(const Instruments::SwaptionParameters& params,
                      const Handle<DiscountBondAsset>& bond)
            : parameters_(params), bond_(bond) {}

            void reset(Size size) {
                values_ = Array(size, 0.0);
                applyCondition();
            }

            virtual void applyCondition() {
                Size i;

                QL_REQUIRE(time()==bond_->time(),
                    "Underlying bond has not been rolled back!");

                for (i=0; i<parameters_.fixedPayTimes.size(); i++) {
                    if (time_ == parameters_.fixedPayTimes[i]) {
                        if (parameters_.payFixed)
                            values_ -= parameters_.fixedCoupons[i];
                        else
                            values_ += parameters_.fixedCoupons[i];
                    }
                }

                for (i=0; i<parameters_.floatingResetTimes.size(); i++) {
                    if (time_ == parameters_.floatingResetTimes[i]) {
                        for (Size j=0; j<values_.size(); j++) {
                            double coupon = parameters_.nominals[i]*
                                (1.0 - bond_->values()[j]);
                            if (parameters_.payFixed)
                                values_[j] += coupon;
                            else
                                values_[j] -= coupon;
                        }
                    }
                }
                for (i=0; i<parameters_.floatingPayTimes.size(); i++) {
                    if (time_ == parameters_.floatingPayTimes[i]) {
                        bond_->reset(values_.size());
                    }
                }
            }
          private:
            Instruments::SwaptionParameters parameters_;
            Handle<DiscountBondAsset> bond_;
        };

        class SwaptionAsset : public Asset {
          public:
            SwaptionAsset(
                const Instruments::SwaptionParameters& params,
                const Handle<SwapAsset>& swap)
            : parameters_(params), swap_(swap) {}

            void reset(Size size) {
                values_ = Array(size, 0.0);
                applyCondition();
            }

            virtual void applySpecificCondition() {
                for (Size i=0; i<values_.size(); i++)
                    values_[i] = QL_MAX(swap_->values()[i], values_[i]);
            }

            virtual void applyCondition() {
                QL_REQUIRE(time()==swap_->time(),
                    "Underlying swap has not been rolled back!");

                Size i;
                if (parameters_.exerciseType != Exercise::American) {
                    for (i=0; i<parameters_.exerciseTimes.size(); i++) {
                        if (time_ == parameters_.exerciseTimes[i]) {
                            applySpecificCondition();
                        }
                    }
                } else {
                    if (
                      (time_ >= parameters_.exerciseTimes[0]) &&
                      (time_ <= parameters_.exerciseTimes[1]))
                        applySpecificCondition();
                }
            }

          private:
            Instruments::SwaptionParameters parameters_;
            Handle<SwapAsset> swap_;
        };

        void TreeSwaption::calculate() const {

            Handle<Tree> tree;

            if (tree_.isNull()) {
                QL_REQUIRE(!model_.isNull(), "You must first define a model");

                std::list<Time> times(0);
                Size i;
                for (i=0; i<parameters_.exerciseTimes.size(); i++)
                    times.push_back(parameters_.exerciseTimes[i]);
                for (i=0; i<parameters_.fixedPayTimes.size(); i++)
                    times.push_back(parameters_.fixedPayTimes[i]);
                for (i=0; i<parameters_.floatingResetTimes.size(); i++)
                    times.push_back(parameters_.floatingResetTimes[i]);
                for (i=0; i<parameters_.floatingPayTimes.size(); i++)
                    times.push_back(parameters_.floatingPayTimes[i]);
                times.sort();
                times.unique();

                Handle<OneFactorModel> model(model_);
                QL_REQUIRE(!model.isNull(), "Only 1-d trees are supported");

                TimeGrid timeGrid(times, timeSteps_);
                tree = model->tree(timeGrid);
            } else {
                tree = tree_;
            }

            Handle<Asset> bond(new DiscountBondAsset());
            Handle<Asset> swap(new SwapAsset(parameters_, bond));
            Handle<Asset> swaption(new SwaptionAsset(parameters_, swap));

            std::vector<Handle<Asset> > assets(0);
            assets.push_back(bond);
            assets.push_back(swap);

            Time lastFixedPay = parameters_.fixedPayTimes.back();
            Time lastFloatPay = parameters_.floatingPayTimes.back();
            Time start = QL_MAX(lastFixedPay, lastFloatPay);

            tree->initialize(bond, start);
            tree->initialize(swap, start);

            Time stop = parameters_.exerciseTimes.back();
            tree->rollback(assets, stop);

            assets.push_back(swaption);
            tree->initialize(swaption, stop);

            stop = parameters_.exerciseTimes[0];
            tree->rollback(assets, stop);

            results_.value = tree->presentValue(swaption);
        }

    }

}
