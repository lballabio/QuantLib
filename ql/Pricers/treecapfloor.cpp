

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
            void reset(Size size) {
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
                      case Option::Straddle:
                          throw IllegalArgumentError(
                              "DiscountBondOptionAsset: straddle not handled");
                          break;
                      default:
                          throw IllegalArgumentError(
                              "DiscountBondOptionAsset: invalid option type");
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
            Size nPeriods = parameters_.startTimes.size();
            Size i;
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
