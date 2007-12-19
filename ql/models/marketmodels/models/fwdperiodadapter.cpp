/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2007 StatPro Italia srl

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

#include <ql/models/marketmodels/models/fwdperiodadapter.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/forwardforwardmappings.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <set>

namespace QuantLib {

    FwdPeriodAdapter::FwdPeriodAdapter(
                               const boost::shared_ptr<MarketModel>& largeModel,
                               Size period,
                               Size offset,
                               const std::vector<Spread>& newDisplacements)
    :
      numberOfFactors_(largeModel->numberOfFactors()),
          numberOfRates_((largeModel->numberOfRates()-offset) / (period > 0 ? period : 1) ),
      numberOfSteps_(largeModel->numberOfSteps()),
      pseudoRoots_(numberOfSteps_, Matrix(numberOfRates_,
                                          numberOfFactors_)),
                                          displacements_(newDisplacements)
    {
        QL_REQUIRE( period >0, "period must  be greater than zero in fwdperiodadapter");
        QL_REQUIRE(period > offset, "period must be greater than offset in fwdperiodadapter");

        const std::vector<Spread>& largeDisplacements_ =
            largeModel->displacements();

        if (displacements_.size() == 1)
        {
            Real dis = displacements_[0];
            displacements_.resize(numberOfRates_);
            std::fill(displacements_.begin(), displacements_.end(), dis);
        }

        if (displacements_.size() ==0) // if not specified use average across rate
        {
            displacements_.reserve(numberOfRates_);
            Size m=0;
            Real sum=0.0;
            for (Size k=0; k < numberOfRates_; ++k)
            {
                for (Size l=0; l < period; ++l, ++m)
                    sum+= largeDisplacements_[m];

                displacements_.push_back(sum/period);
            }
        }
        QL_REQUIRE( displacements_.size() == numberOfRates_,"newDisplacements should be empty,1, or number of new rates in fwdperiodadapter");

        LMMCurveState largeCS(largeModel->evolution().rateTimes());
        largeCS.setOnForwardRates(largeModel->initialRates());

        LMMCurveState smallCS(
                ForwardForwardMappings::RestrictCurveState(largeCS,
                                    period, offset
                                        ));

        initialRates_ =smallCS.forwardRates();

        Real finalReset = smallCS.rateTimes()[smallCS.numberOfRates()-1];
        std::vector<Time> oldEvolutionTimes(largeModel->evolution().evolutionTimes());
        std::vector<Time> newEvolutionTimes;
        for (Size i =0; i < oldEvolutionTimes.size() && oldEvolutionTimes[i]<= finalReset; ++i)
            newEvolutionTimes.push_back(oldEvolutionTimes[i]);

        evolution_=EvolutionDescription(smallCS.rateTimes(),
                                        newEvolutionTimes);

        numberOfSteps_ = newEvolutionTimes.size();


        const std::vector<Time>& rateTimes =
            smallCS.rateTimes();
        // we must ensure we step through all rateTimes
        const std::vector<Time>& evolutionTimes =
            evolution_.evolutionTimes();

        std::set<Time> setTimes(evolutionTimes.begin(),evolutionTimes.end());

        for (Size i=0; i < rateTimes.size()-1; ++i)
            QL_REQUIRE(setTimes.find(rateTimes[i]) != setTimes.end(),
                        "every new rate time except last must be an evolution time in fwdperiod adapter");


        Matrix YMatrix =
            ForwardForwardMappings::YMatrix( largeCS,
                                                 largeDisplacements_,
                                                  displacements_,
                                                 period,
                                                 offset
                                                 );

        const std::vector<Size>& alive =
            evolution_.firstAliveRate();

        for (Size k = 0; k<numberOfSteps_; ++k) {
            pseudoRoots_[k]=YMatrix*largeModel->pseudoRoot(k);
            for (Size i=0; i<alive[k]; ++i)
                std::fill(pseudoRoots_[k].row_begin(i),
                          pseudoRoots_[k].row_end(i),
                          0.0);
        }
    }



}


