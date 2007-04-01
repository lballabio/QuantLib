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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/marketmodels/evolutiondescription.hpp>
#include <ql/math/matrix.hpp>
#include <ql/utilities/dataformatters.hpp>

namespace QuantLib {

    EvolutionDescription::EvolutionDescription() {}

    EvolutionDescription::EvolutionDescription(
                     const std::vector<Time>& rateTimes,
                     const std::vector<Time>& evolutionTimes,
                     const std::vector<std::pair<Size,Size> >& relevanceRates)
    : rateTimes_(rateTimes),
      evolutionTimes_(evolutionTimes.empty() && !rateTimes.empty() ?
                      std::vector<Time>(rateTimes.begin(), rateTimes.end()-1) :
                      evolutionTimes),
      relevanceRates_(relevanceRates),
      rateTaus_(rateTimes_.size()-1),
      effStopTime_(evolutionTimes_.size(), rateTimes_.size()-1),
      firstAliveRate_(evolutionTimes_.size())
    {

        Size nRates_ = rateTimes_.size() - 1; 

        QL_REQUIRE(nRates_>0,
                   "Rate times must contain at least two values");
        QL_REQUIRE(rateTimes_[0]>=0.0,
                   "first rate time must be non negative");
        for (Size i=0; i<nRates_; ++i) {
            rateTaus_[i] = rateTimes_[i+1] - rateTimes_[i];
            QL_REQUIRE(rateTaus_[i]>0, "non increasing rate times");
        }

        Size steps_ = evolutionTimes_.size(); 
        QL_REQUIRE(steps_>0,
                   "Evolution times must have 1 elements at least");
        for (Size i = 1; i<steps_; ++i)
            QL_REQUIRE(evolutionTimes_[i]>evolutionTimes_[i-1],
                       "Evolution times must be strictly increasing");
        QL_REQUIRE(rateTimes.back() >= evolutionTimes_.back(),
                   "The last evolution time is past the last rate time");

        if (relevanceRates.empty())
            relevanceRates_ = std::vector<std::pair<Size,Size> >(
                                steps_, std::make_pair(0,nRates_));
        else
            QL_REQUIRE(relevanceRates.size() == steps_,
                       "relevanceRates / evolutionTimes mismatch");

        for (Size j=0; j<steps_; ++j) {
            for (Size i=0; i<nRates_; ++i)
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

    const std::vector<Time>& EvolutionDescription::rateTimes() const {
        return rateTimes_;
    }

    const std::vector<Time>& EvolutionDescription::rateTaus() const {
        return rateTaus_;
    }

    const std::vector<Time>& EvolutionDescription::evolutionTimes() const {
        return evolutionTimes_;
    }

    const Matrix& EvolutionDescription::effectiveStopTime() const {
        return effStopTime_;
    }

    const std::vector<Size>& EvolutionDescription::firstAliveRate() const {
        return firstAliveRate_;
    }

    const std::vector<std::pair<Size,Size> >& EvolutionDescription::relevanceRates() const {
        return relevanceRates_;
    }

    Size EvolutionDescription::numberOfRates() const {
        return rateTimes_.size() - 1; 
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
                       io::ordinal(i) << " step, evolution time " <<
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
