/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
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

#ifndef quantlib_evolution_description_hpp
#define quantlib_evolution_description_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Market-model evolution description
    /*! This class stores:
        -# evolutionTimes = the times at which the rates need to be known,
        -# rateTimes = the times defining the rates that are to be evolved,
        -# relevanceRates = which rates need to be known at each time.

        This class is really just a tuple of evolution and rate times;
        - there will be n+1 rate times expressing payment and reset
          times of forward rates.
        - there will be any number of evolution times.
        - we also store which part of the rates are relevant for
          pricing via relevance rates. The important part for the i-th
          step will then range from relevanceRates[i].first to
          relevanceRates[i].second. Default values for relevance rates
          will be 0 and n.
        - example for n = 5:
        <pre>
           |-----|-----|-----|-----|-----|      (size = 6)
           t0    t1    t2    t3    t4    t5     rateTimes
           f0    f1    f2    f3    f4           forwardRates
           d0    d1    d2    d3    d4    d5     discountBonds
           d0/d0 d1/d0 d2/d0 d3/d0 d4/d0 d5/d0  discountRatios
           sr0   sr1   sr2   sr3   sr4          coterminalSwaps
        </pre>
    */
    class EvolutionDescription {
        // This typedef is used so that gcc 3.3 can parse the
        // constructor declaration (not even parenthesizing the
        // default argument worked)
        typedef std::pair<Size,Size> range;
      public:
        EvolutionDescription() = default;
        explicit EvolutionDescription(
            const std::vector<Time>& rateTimes,
            const std::vector<Time>& evolutionTimes = std::vector<Time>(),
            const std::vector<std::pair<Size,Size> >& relevanceRates =
                                                        std::vector<range>());
        const std::vector<Time>& rateTimes() const;
        const std::vector<Time>& rateTaus() const;
        const std::vector<Time>& evolutionTimes() const;
        //const Matrix& effectiveStopTimes() const;
        const std::vector<Size>& firstAliveRate() const;
        const std::vector<std::pair<Size,Size> >& relevanceRates() const;
        Size numberOfRates() const;
        Size numberOfSteps() const;
      private:
        Size numberOfRates_;
        std::vector<Time> rateTimes_, evolutionTimes_;
        std::vector<std::pair<Size,Size> > relevanceRates_;
        // convenience variables
        std::vector<Time> rateTaus_;
        std::vector<Size> firstAliveRate_;
    };

    // Numeraire functions

    /*! Check that there is one numeraire for each evolution time.
        Each numeraire must be an index amongst the rate times so it ranges
        from 0 to n. Each numeraire must not have expired before the end of
        the step.
    */
    void checkCompatibility(const EvolutionDescription& evolution,
                            const std::vector<Size>& numeraires);

    bool isInTerminalMeasure(const EvolutionDescription& evolution,
                             const std::vector<Size>& numeraires);
    bool isInMoneyMarketPlusMeasure(const EvolutionDescription& evolution,
                                    const std::vector<Size>& numeraires,
                                    Size offset = 1);
    bool isInMoneyMarketMeasure(const EvolutionDescription& evolution,
                                const std::vector<Size>& numeraires);

    //! Terminal measure: the last bond is used as numeraire.
    std::vector<Size> terminalMeasure(const EvolutionDescription& evolution);

    /*! Offsetted discretely compounded money market account measure:
        for each step the offset-th unexpired bond is used as numeraire.
        When offset=0 the result is the usual discretely compounded money
        market account measure
    */
    std::vector<Size> moneyMarketPlusMeasure(const EvolutionDescription&,
                                             Size offset = 1);

    /*! Discretely compounded money market account measure:
        for each step the first unexpired bond is used as numeraire.
    */
    std::vector<Size> moneyMarketMeasure(const EvolutionDescription&);

}

#endif
