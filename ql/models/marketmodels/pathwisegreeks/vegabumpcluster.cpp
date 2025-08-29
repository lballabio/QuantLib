/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Mark Joshi

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

#include <ql/errors.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/pathwisegreeks/vegabumpcluster.hpp>
#include <utility>
#include <valarray>

namespace QuantLib {



    VegaBumpCluster::VegaBumpCluster(Size factorBegin,
        Size factorEnd,
        Size rateBegin,
        Size rateEnd,
        Size stepBegin,
        Size stepEnd)
        :
    factorBegin_(factorBegin),
        factorEnd_(factorEnd),
        rateBegin_(rateBegin),
        rateEnd_(rateEnd),
        stepBegin_(stepBegin),
        stepEnd_(stepEnd)
    {
        QL_REQUIRE(factorBegin_<factorEnd_, "must have factorBegin_ < factorEnd_ in VegaBumpCluster ");
        QL_REQUIRE(rateBegin_<rateEnd_, "must have rateBegin_ < rateEnd_ in VegaBumpCluster ");
        QL_REQUIRE(stepBegin_<stepEnd_, "must have stepBegin_ < stepEnd_ in VegaBumpCluster ");
    }

    bool VegaBumpCluster::doesIntersect(const VegaBumpCluster& comparee) const
    {
        if (factorEnd_ <= comparee.factorBegin_)
            return false;

        if (rateEnd_ <= comparee.rateBegin_)
            return false;

        if (stepEnd_ <= comparee.stepBegin_)
            return false;


        if (comparee.factorEnd_ <= factorBegin_)
            return false;

        if (comparee.rateEnd_ <= rateBegin_)
            return false;

        if (comparee.stepEnd_ <= stepBegin_)
            return false;

        return true;


    }


    bool VegaBumpCluster::isCompatible(const ext::shared_ptr<MarketModel>& volStructure) const
    {
        if (rateEnd_ > volStructure->numberOfRates())
            return false;

        if (stepEnd_ > volStructure->numberOfSteps())
            return false;

        if (factorEnd_ > volStructure->numberOfFactors())
            return false;

        Size firstAliveRate = volStructure->evolution().firstAliveRate()[stepEnd_-1];

        return rateBegin_ >= firstAliveRate; // if the rate has reset after the beginning of the last step of the bump
    }



    VegaBumpCollection::VegaBumpCollection(const ext::shared_ptr<MarketModel>& volStructure,
                           bool factorwiseBumping)
                            : associatedVolStructure_(volStructure)
    {
        Size steps = volStructure->numberOfSteps();
        Size rates = volStructure->numberOfRates();
        Size factors = volStructure->numberOfFactors();

        for (Size s=0; s < steps; ++s)
            for (Size r=volStructure->evolution().firstAliveRate()[s]; r < rates; ++r)
            {
                if (factorwiseBumping)
                {
                    for (Size f=0; f < factors; ++f)
                    {
                        VegaBumpCluster thisCluster(f,f+1,r,r+1,s,s+1);
                        allBumps_.push_back(thisCluster);

                    }
                }
                else
                {
                     VegaBumpCluster thisCluster(0,factors,r,r+1,s,s+1);
                     allBumps_.push_back(thisCluster);

                }
            }

        checked_=true;
        full_=true;
        nonOverlapped_=true;


    }


    VegaBumpCollection::VegaBumpCollection(std::vector<VegaBumpCluster> allBumps,
                                           ext::shared_ptr<MarketModel> volStructure)
    : allBumps_(std::move(allBumps)), associatedVolStructure_(std::move(volStructure)),
      checked_(false) {
        for (auto& allBump : allBumps_)
            QL_REQUIRE(allBump.isCompatible(associatedVolStructure_),
                       "incompatible bumps passed to VegaBumpCollection");
    }


    const std::vector<VegaBumpCluster>& VegaBumpCollection::allBumps() const
    {
        return allBumps_;
    }

    bool VegaBumpCollection::isFull() const // i.e. is every alive pseudo-root element bumped at least once
    {
        if (checked_)
            return full_;
        std::vector<std::vector<std::valarray<bool> > > v;

        Size factors = associatedVolStructure_->numberOfFactors();

        std::valarray<bool> model(false,factors);
    //    std::fill(model.begin(), model.end(), false);

        std::vector<std::valarray<bool> > modelTwo;
        modelTwo.reserve(associatedVolStructure_->numberOfRates());
        for (Size i=0; i < associatedVolStructure_->numberOfRates(); ++i)
            modelTwo.push_back(model);

        v.reserve(associatedVolStructure_->numberOfSteps());
        for (Size j=0; j < associatedVolStructure_->numberOfSteps(); ++j)
            v.push_back(modelTwo);

        for (const auto& allBump : allBumps_) {
            for (Size f = allBump.factorBegin(); f < allBump.factorEnd(); ++f)
                for (Size r = allBump.rateBegin(); r < allBump.rateEnd(); ++r)
                    for (Size s = allBump.stepBegin(); s < allBump.stepEnd(); ++s)
                        v[s][r][f] = true;
        }

        Size numberFailures =0;
        for (Size s =0; s < associatedVolStructure_->numberOfSteps(); ++s)
            for (Size f=0; f < associatedVolStructure_->numberOfFactors(); ++f)
                for (Size r=associatedVolStructure_->evolution().firstAliveRate()[s]; r <  associatedVolStructure_->numberOfRates(); ++r)
                    if (!v[s][r][f])
                        ++numberFailures;

        return numberFailures>0;

    }

    bool VegaBumpCollection::isNonOverlapping() const // i.e. is every alive pseudo-root element bumped at most once
    {

        if (checked_)
            return nonOverlapped_;

        std::vector<std::vector<std::valarray<bool> > > v;

        Size factors = associatedVolStructure_->numberOfFactors();


        std::valarray<bool> model(false,factors);
        //std::fill(model.begin(), model.end(), false);

        std::vector<std::valarray<bool> > modelTwo;
        modelTwo.reserve(associatedVolStructure_->numberOfRates());
        for (Size i=0; i < associatedVolStructure_->numberOfRates(); ++i)
            modelTwo.push_back(model);

        v.reserve(associatedVolStructure_->numberOfSteps());
        for (Size j=0; j < associatedVolStructure_->numberOfSteps(); ++j)
            v.push_back(modelTwo);

        Size numberFailures=0;

        for (const auto& allBump : allBumps_) {
            for (Size f = allBump.factorBegin(); f < allBump.factorEnd(); ++f)
                for (Size r = allBump.rateBegin(); r < allBump.rateEnd(); ++r)
                    for (Size s = allBump.stepBegin(); s < allBump.stepEnd(); ++s) {
                        if (v[s][r][f])
                            ++numberFailures;
                        v[s][r][f] = true;
                    }
        }

        return numberFailures>0;

    }

    bool VegaBumpCollection::isSensible() const // i.e. is every alive pseudo-root element bumped precisely once
    {
        if (checked_)
            return true;

        return isNonOverlapping() && isFull();
    }


    Size VegaBumpCollection::numberBumps() const
    {
        return allBumps_.size();
    }

}
