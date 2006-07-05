/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2006 Marco Bianchetti
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2006 Giorgio Facchinetti

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

	EvolutionDescription::EvolutionDescription(const Array& rateTimes,
                           const Array& evolutionTimes,
                           const std::vector<Size>& numeraires,
                           const std::vector<std::pair<Size,Size> >& relevanceRates)
    : rateTimes_(rateTimes), evolutionTimes_(evolutionTimes),
      numeraires_(numeraires), relevanceRates_(relevanceRates),
      taus_(rateTimes.size()-1) {

 /* There will n+1 rate times expressing payment and reset times of forward rates.

               |-----|-----|-----|-----|-----|      (size = 6)
               t0    t1    t2    t3    t4    t5     rateTimes
               f0    f1    f2    f3    f4           forwardRates
               d0    d1    d2    d3    d4    d5     discountBonds 
               d0/d0 d1/d0 d2/d0 d3/d0 d4/d0 d5/d0  discountRatios
               sr0   sr1   sr2   sr3   sr4           coterminalSwaps
*/
          // check coherence of input data
          QL_REQUIRE(rateTimes_.size()>1, 
              "Array rate times must have 2 elements at least");
          // todo: check increasing rateTimes

          QL_REQUIRE(evolutionTimes.size()>0, 
              "Array evolution times must have 1 elements at least");         
          // todo: check increasing evolutionTimes

          if (numeraires.empty()) {
              // default numeraire is the terminal one
              std::fill(numeraires_.begin(), numeraires_.end(),
                        rateTimes_.size()-1);
          } else {
              QL_REQUIRE(numeraires.size() == evolutionTimes.size(), 
                         "Numeraires / evolutionTimes mismatch");     
              for (Size i=0; i<numeraires.size()-1; i++) {
                  QL_REQUIRE(rateTimes[numeraires[i]] >= evolutionTimes[i], 
                             "Numeraire " << i << " expired");
              }
          }
        /* 
        Default values for numeraires will be the final bond.
		- We also store which part of the rates are relevant for pricing via
        relevance rates. The important part for the i-th step will then range
        from relevanceRates[i].first to relevanceRates[i].second
        Default values for relevance rates will be 0 and n. 
        */

        for (Size i=0; i<rateTimes.size()-2; i++) {
            taus_[i] = rateTimes_[i+1] - rateTimes_[i];
        }
    }

    const Array& EvolutionDescription::rateTimes() const {
        return rateTimes_;
    }

    const Array& EvolutionDescription::taus() const {
        return taus_;
    }

    const Array& EvolutionDescription::evolutionTimes() const {
        return evolutionTimes_;
    }

    const std::vector<Size>& EvolutionDescription::numeraires() const {
        return numeraires_;
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
}
