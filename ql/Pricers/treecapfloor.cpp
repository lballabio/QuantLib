
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

/*! \file treecapfloor.cpp
    \brief Cap/Floor calculated using a tree

    \fullpath
    ql/Pricers/%treecapfloor.cpp
*/

// $Id$

#include "ql/Pricers/treecapfloor.hpp"

#include "ql/asset.hpp"
#include "ql/InterestRateModelling/onefactormodel.hpp"

namespace QuantLib {

    namespace Pricers {

        using namespace Instruments;
        using namespace InterestRateModelling;
        using namespace Lattices;

        class DiscountBondOptionAsset : public Asset {
          public:
            DiscountBondOptionAsset(Option::Type type, 
                                    Time maturity, 
                                    double strike) 
            : type_(type), maturity_(maturity), strike_(strike) {}
            void reset(size_t size) {
                values_ = Array(size, 1.0);
            }
            virtual void applyCondition() {
                if (time_ == maturity_) {
                    Size i;
                    switch(type_) {
                      case Option::Call:
                        for (i=0; i<newValues_.size(); i++)
                          newValues_[i] = QL_MAX(newValues_[i] - strike_, 0.0);
                        break;
                      case Option::Put:
                        for (i=0; i<newValues_.size(); i++)
                          newValues_[i] = QL_MAX(strike_ - newValues_[i], 0.0);
                        break;
                    }
                }
                values_ = newValues_;
            }
          private:
            Option::Type type_;
            Time maturity_;
            double strike_;
        };

        void TreeCapFloor::calculate() const {
            QL_REQUIRE(!model_.isNull(), "Cannot price without model!");

            QL_REQUIRE(
                model_->type() == InterestRateModelling::Model::OneFactor,
                "Analytical formulas required for n-factor models n>1");
            Handle<OneFactorModel> model(model_);

            std::list<Time> times(0);
            size_t nPeriods = parameters_.startTimes.size();
            size_t i;
            for (i=0; i<nPeriods; i++)
                times.push_back(parameters_.startTimes[i]);
            for (i=0; i<nPeriods; i++)
                times.push_back(parameters_.endTimes[i]);
            times.unique();
            times.sort();

            TimeGrid timeGrid(times, timeSteps_);
            Handle<Tree> tree = model->tree(timeGrid);

            Option::Type optionType;
            if (parameters_.type==VanillaCapFloor::Cap)
                optionType = Option::Put;
            else
                optionType = Option::Call;

            double value = 0.0;

            for (i=0; i<nPeriods; i++) {
                Rate exerciseRate;
                if (i<parameters_.exerciseRates.size())
                    exerciseRate = parameters_.exerciseRates[i];
                else
                    exerciseRate = parameters_.exerciseRates.back();

                Time maturity = parameters_.startTimes[i];
                Time bond = parameters_.endTimes[i];

                Time tenor = bond - maturity;
                double strike = 1.0/(1.0+exerciseRate*tenor);

                std::vector<Handle<Asset> > assets(0);
                Handle<Asset> dbo(
                    new DiscountBondOptionAsset(optionType, maturity, strike));
                assets.push_back(dbo);
                tree->rollback(assets, bond, maturity);
                double optionValue = tree->presentValue(dbo);
                double capletValue = parameters_.nominals[i]*
                    (1.0+exerciseRate*tenor)*optionValue;
                value += capletValue;
            }
            results_.value = value;

        }

    }

}
