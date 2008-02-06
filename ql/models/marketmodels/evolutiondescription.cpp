/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006, 2007 Mark Joshi

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

#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/utilities.hpp>
#include <ql/math/matrix.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    EvolutionDescription::EvolutionDescription() {}

    EvolutionDescription::EvolutionDescription(
                     const std::vector<Time>& rateTimes,
                     const std::vector<Time>& evolutionTimes,
                     const std::vector<std::pair<Size,Size> >& relevanceRates)
    : numberOfRates_(rateTimes.empty() ? 0 : rateTimes.size()-1),
      rateTimes_(rateTimes),
      evolutionTimes_(evolutionTimes.empty() && !rateTimes.empty() ?
                      std::vector<Time>(rateTimes.begin(), rateTimes.end()-1) :
                      evolutionTimes),
      relevanceRates_(relevanceRates),
      rateTaus_(numberOfRates_),
      //effStopTime_(evolutionTimes_.size(), rateTimes_.size()-1),
      firstAliveRate_(evolutionTimes_.size())
    {

        checkIncreasingTimesAndCalculateTaus(rateTimes_, rateTaus_);

        checkIncreasingTimes(evolutionTimes_);
        Size numberOfSteps = evolutionTimes_.size();

        QL_REQUIRE(evolutionTimes_.back()<=rateTimes[rateTimes.size()-2],
                   "The last evolution time (" << evolutionTimes_.back() <<
                   ") is past the last fixing time (" <<
                   rateTimes[numberOfRates_-2] << ")");

        if (relevanceRates.empty())
            relevanceRates_ = std::vector<std::pair<Size,Size> >(
                                numberOfSteps, std::make_pair(0,numberOfRates_));
        else
            QL_REQUIRE(relevanceRates.size() == numberOfSteps,
                       "relevanceRates / evolutionTimes mismatch");

        //for (Size j=0; j<numberOfSteps; ++j) {
        //    for (Size i=0; i<numberOfRates_; ++i)
        //        effStopTime_[j][i] =
        //            std::min(evolutionTimes_[j], rateTimes_[i]);
        //}

        Time currentEvolutionTime = 0.0;
        Size firstAliveRate = 0;
        for (Size j=0; j<numberOfSteps; ++j) {
            while (rateTimes_[firstAliveRate] <= currentEvolutionTime)
                ++firstAliveRate;
            firstAliveRate_[j] = firstAliveRate;
            currentEvolutionTime = evolutionTimes_[j];
        }
    }

    const std::vector<Time>& EvolutionDescription::rateTimes() const {
        return rateTimes_;
    }

    const std::vector<Time>& EvolutionDescription::rateTaus() const {
        return rateTaus_;
    }

    const std::vector<Time>& EvolutionDescription::evolutionTimes() const {
        return evolutionTimes_;
    }

    //const Matrix& EvolutionDescription::effectiveStopTimes() const {
    //    return effStopTime_;
    //}

    const std::vector<Size>& EvolutionDescription::firstAliveRate() const {
        return firstAliveRate_;
    }

    const std::vector<std::pair<Size,Size> >& EvolutionDescription::relevanceRates() const {
        return relevanceRates_;
    }

    Size EvolutionDescription::numberOfRates() const {
        return numberOfRates_;
    }

    Size EvolutionDescription::numberOfSteps() const {
        return evolutionTimes_.size();
    }

    void checkCompatibility(const EvolutionDescription& evolution,
                            const std::vector<Size>& numeraires)
    {
        const std::vector<Time>& evolutionTimes = evolution.evolutionTimes();
        Size n = evolutionTimes.size();
        QL_REQUIRE(numeraires.size() == n,
                   "Size mismatch between numeraires (" << numeraires.size()
                   << ") and evolution times (" << n << ")");

        const std::vector<Time>& rateTimes = evolution.rateTimes();
        for (Size i=0; i<n-1; i++)
            QL_REQUIRE(rateTimes[numeraires[i]] >= evolutionTimes[i],
                       io::ordinal(i+1) << " step, evolution time " <<
                       evolutionTimes[i] << ": the numeraire (" << numeraires[i] <<
                       "), corresponding to rate time " <<
                       rateTimes[numeraires[i]] << ", is expired");
    }

    bool isInTerminalMeasure(const EvolutionDescription& evolution,
                             const std::vector<Size>& numeraires) {
        const std::vector<Time>& rateTimes = evolution.rateTimes();
        return *std::min_element(numeraires.begin(), numeraires.end()) ==
                                                          rateTimes.size()-1;
    }

    bool isInMoneyMarketPlusMeasure(const EvolutionDescription& evolution,
                                    const std::vector<Size>& numeraires,
                                    Size offset) {
        bool res = true;
        const std::vector<Time>& rateTimes = evolution.rateTimes();
        Size maxNumeraire=rateTimes.size()-1;
        QL_REQUIRE(offset<=maxNumeraire,
                   "offset (" << offset <<
                   ") is greater than the max allowed value for numeraire ("
                   << maxNumeraire << ")");
        const std::vector<Time>& evolutionTimes = evolution.evolutionTimes();
        for (Size i=0, j=0; i<evolutionTimes.size(); ++i) {
            while (rateTimes[j] < evolutionTimes[i])
                j++;
            res = (numeraires[i] == std::min(j+offset, maxNumeraire)) && res;
        }
        return res;
    }

    bool isInMoneyMarketMeasure(const EvolutionDescription& evolution,
                                const std::vector<Size>& numeraires) {
        return isInMoneyMarketPlusMeasure(evolution, numeraires, 0);
    }

    std::vector<Size> terminalMeasure(const EvolutionDescription& evolution)
    {
        return std::vector<Size>(evolution.evolutionTimes().size(),
                                 evolution.rateTimes().size()-1);
    }

    std::vector<Size> moneyMarketPlusMeasure(const EvolutionDescription& ev,
                                             Size offset) {
        const std::vector<Time>& rateTimes = ev.rateTimes();
        Size maxNumeraire = rateTimes.size()-1;
        QL_REQUIRE(offset<=maxNumeraire,
                   "offset (" << offset <<
                   ") is greater than the max allowed value for numeraire ("
                   << maxNumeraire << ")");

        const std::vector<Time>& evolutionTimes = ev.evolutionTimes();
        Size n = evolutionTimes.size();
        std::vector<Size> numeraires(n);
        for (Size i=0, j=0; i<n; ++i) {
            while (rateTimes[j] < evolutionTimes[i])
                j++;
            numeraires[i] = std::min(j+offset, maxNumeraire);
        }
        return numeraires;
    }

    std::vector<Size> moneyMarketMeasure(const EvolutionDescription& evol) {
        return moneyMarketPlusMeasure(evol, 0);
    }

}
