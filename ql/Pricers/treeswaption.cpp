
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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
                newValues_ = Array(size, 0.0);
                applyCondition();
            }

            virtual void applyCondition() {
                Size i;
                for (i=0; i<parameters_.fixedPayTimes.size(); i++) {
                    if (time_ == parameters_.fixedPayTimes[i]) {
                        if (parameters_.payFixed)
                            newValues_ -= parameters_.fixedCoupons[i];
                        else
                            newValues_ += parameters_.fixedCoupons[i];
                    }
                }
                for (i=0; i<parameters_.floatingResetTimes.size(); i++) {
                    if (time_ == parameters_.floatingResetTimes[i]) {
                        for (Size j=0; j<newValues_.size(); j++) {
                            double coupon = parameters_.nominals[i]*
                                (1.0 - bond_->values()[j]);
                            if (parameters_.payFixed)
                                newValues_[j] += coupon;
                            else
                                newValues_[j] -= coupon;
                        }
                    }
                }
                for (i=0; i<parameters_.floatingPayTimes.size(); i++) {
                    if (time_ == parameters_.floatingPayTimes[i]) {
                        bond_->reset(newValues_.size());
                    }
                }
                values_ = newValues_;
            }
          private:
            const Instruments::SwaptionParameters& parameters_;
            Handle<DiscountBondAsset> bond_;
        };

        class SwaptionAsset : public Asset {
          public:
            SwaptionAsset(
                const Instruments::SwaptionParameters& params,
                const Handle<SwapAsset>& swap)
            : parameters_(params), swap_(swap) {}

            void reset(Size size) {
                newValues_ = Array(size, 0.0);
                applyCondition();
            }

            virtual void applySpecificCondition() {
                for (Size i=0; i<newValues_.size(); i++)
                    newValues_[i] = QL_MAX(swap_->values()[i], newValues_[i]);
            }

            virtual void applyCondition() {
                Size i;
                if (parameters_.exerciseType != Exercise::American) {
                    for (i=0; i<parameters_.exerciseTimes.size(); i++) {
                        if (time_ == parameters_.exerciseTimes[i]) {
                            applySpecificCondition();
                        }
                    }
                } else {
                    applySpecificCondition();
                }
                values_ = newValues_;
            }

          private:
            const Instruments::SwaptionParameters& parameters_;
            Handle<SwapAsset> swap_;
        };

        TreeSwaption::TreeSwaption(Size timeSteps) : timeSteps_(timeSteps) {}

        void TreeSwaption::calculate() const {
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
            times.unique();
            times.sort();

            QL_REQUIRE(model_->type()==Model::OneFactor,
                "Only 1-d trees are supported at the moment");
            Handle<OneFactorModel> model(model_);

            TimeGrid timeGrid(times, timeSteps_);
            Handle<Tree> tree(model->tree(timeGrid));

            Handle<Asset> bond(new DiscountBondAsset());
            Handle<Asset> swap(new SwapAsset(parameters_, bond));
            Handle<Asset> swaption(new SwaptionAsset(parameters_, swap));

            std::vector<Handle<Asset> > assets(0);
            assets.push_back(bond);
            assets.push_back(swap);
            assets.push_back(swaption);

            //FIXME: optimize for european and bermudan
            // do not rollback until 0 but until the first exercise date
            // sum with state prices...
            
            tree->rollback(assets, times.back(), 0.0);
            results_.value = swaption->values()[0];

            std::cout << "Discount bond price: " << bond->values()[0]*100.0 << std::endl;
            std::cout << "Theoretical value: " << model->termStructure()->discount(parameters_.floatingPayTimes[0])*100.0 << std::endl;
            std::cout << "Swap price: " << swap->values()[0] << std::endl;
            std::cout << "Swaption price: " << swaption->values()[0] << std::endl;
        }

    }

}
