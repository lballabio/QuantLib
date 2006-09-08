/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mark Joshi

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

#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/Math/matrix.hpp>

namespace QuantLib {

    EvolutionDescription::EvolutionDescription(
                           const std::vector<Time>& rateTimes,
                           const std::vector<Time>& evolutionTimes,
                           const std::vector<Size>& numeraires,
                           const std::vector<std::pair<Size,Size> >& relevanceRates)
    : rateTimes_(rateTimes), evolutionTimes_(evolutionTimes),
      steps_(evolutionTimes.size()),
      numeraires_(numeraires), relevanceRates_(relevanceRates),
      rateTaus_(rateTimes.size()-1),
      effStopTime_(evolutionTimes.size(), rateTimes.size()-1),
      firstAliveRate_(evolutionTimes.size())
    {

        // check coherence of input data
        QL_REQUIRE(rateTimes_.size()>1,
                   "Rate times must have 2 elements at least");
        QL_REQUIRE(rateTimes_[0]>=0.0,
                   "first rate time must be non negative");
        for (Size i = 1; i<rateTimes.size(); ++i)
            QL_REQUIRE(rateTimes[i]>rateTimes[i-1],
                       "rate times must be strictly increasing");

        QL_REQUIRE(steps_>0,
                   "Evolution times must have 1 elements at least");
        for (Size i = 1; i<steps_; ++i)
            QL_REQUIRE(evolutionTimes[i]>evolutionTimes[i-1],
                       "evolution times must be strictly increasing");

        QL_REQUIRE(rateTimes.back() >= evolutionTimes.back(),
                   "last evolution time is past last rate time");

        if (numeraires.empty()) {
            numeraires_ = std::vector<Size>(steps_, rateTimes_.size()-1);
        } else {
            QL_REQUIRE(numeraires.size() == steps_,
                       "Numeraires / evolutionTimes mismatch");
            for (Size i=0; i<numeraires.size()-1; i++)
                QL_REQUIRE(rateTimes[numeraires[i]] >= evolutionTimes[i],
                           "Numeraire " << i << " expired");
        }

        if (relevanceRates.empty()) {
            relevanceRates_ = std::vector<std::pair<Size,Size> >(
                                steps_, std::make_pair(0,rateTimes.size()-1));
        } else {
            QL_REQUIRE(relevanceRates.size() == steps_,
                       "relevanceRates / evolutionTimes mismatch");
        }

        for (Size i=0; i<rateTaus_.size(); i++)
            rateTaus_[i] = rateTimes_[i+1] - rateTimes_[i];

        for (Size j=0; j<steps_; ++j) {
            for (Size i=0; i<rateTimes_.size()-1; ++i)
                effStopTime_[j][i] =
                    std::min(evolutionTimes_[j], rateTimes_[i]);
        }

        Time currentEvolutionTime = 0.0;
        Size firstAliveRate = 0;
        for (Size j=0; j<steps_; ++j) {
            while (rateTimes_[firstAliveRate] <= currentEvolutionTime)
                ++firstAliveRate;
            firstAliveRate_[j] = firstAliveRate;
            currentEvolutionTime = evolutionTimes_[j];
        }
    }

    void EvolutionDescription::setNumeraires(const std::vector<Size>& numeraires) {
        QL_REQUIRE(numeraires.size() == evolutionTimes_.size(),
                   "Numeraires / evolutionTimes mismatch");
        for (Size i=0; i<numeraires.size()-1; i++) {
            QL_REQUIRE(rateTimes_[numeraires[i]] >= evolutionTimes_[i],
                       "Numeraire " << i << " expired");
        }
        std::copy(numeraires.begin(), numeraires.end(), numeraires_.begin());
    }

    void EvolutionDescription::setTerminalMeasure() {
        std::fill(numeraires_.begin(), numeraires_.end(), rateTimes_.size()-1);
    }

    bool EvolutionDescription::isInTerminalMeasure() const {
        return *std::min_element(numeraires_.begin(), numeraires_.end()) ==
            rateTimes_.size()-1;
    }

    void EvolutionDescription::setMoneyMarketMeasurePlus(Size offset) {
        Size j=0, maxNumeraire=rateTimes_.size()-1;
        for (Size i=0; i<evolutionTimes_.size(); ++i) {
            while (rateTimes_[j] < evolutionTimes_[i])
                j++;
            numeraires_[i] = std::min(j+offset, maxNumeraire);
        }
    }

    bool EvolutionDescription::isInMoneyMarketMeasure() const {
        bool result = true;
        Size j = 0;
        for (Size i=0; i<evolutionTimes_.size(); ++i) {
            while (rateTimes_[j] < evolutionTimes_[i])
                j++;
            result = result && (numeraires_[i] == j);
        }
        return result;
    }

}
