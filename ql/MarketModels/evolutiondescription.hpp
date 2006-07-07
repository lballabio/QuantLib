/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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


#ifndef quantlib_evolutiondescription_hpp
#define quantlib_evolutiondescription_hpp

#include <ql/Math/array.hpp>
#include <vector>

namespace QuantLib {

    /*! This class stores:
		1) the times defining the rates that are to be evolved,
		2) the times at which the rates need to be known, 
		3) the numeraire to be used for each step. 
		4) which rates need to be known at each time.

        This class is really just a 3-tuple of the evolutions times, the rate
        times and the numeraires.
        
        - There will be n+1 rate times expressing payment and reset times of
        forward rates.
		- There will be any number of evolution times.
		- There will be one numeraire index for each evolution time. The
        numeraire is an index amongst the rate times so it ranges from 0 to n.
        The numeraire must not have expired before the end of the step.
        Note that in the terminal measure, one would take the index of the
        last bond as numeraire. Whereas the discretely compounding money market
        account would take the index of the first unexpired bond for each step.
        Default values for numeraires will be the final bond.
		- We also store which part of the rates are relevant for pricing via
        relevance rates. The important part for the i-th step will then range
        from relevanceRates[i].first to relevanceRates[i].second
        Default values for relevance rates will be 0 and n. 
    */
    class EvolutionDescription
    {
    public:

        EvolutionDescription(
            const Array& rateTimes,
            const Array& evolutionTimes,
            const std::vector<Size>& numeraires = std::vector<Size>(),
            const std::vector<std::pair<Size,Size> >& relevanceRates =
                                    std::vector<std::pair<Size,Size> >());
        const Array& rateTimes() const;
        const Array& taus() const;
        const Array& evolutionTimes() const;
        const std::vector<Size>& numeraires() const;
        const std::vector<std::pair<Size,Size> >& relevanceRates() const;
        Size numberOfRates() const;     // returns rateTimes().size() - 1
        Size numberOfSteps() const;     // returns evolutionTimes().size()

        void setNumeraires(const std::vector<Size>&);
        void setTerminalMeasure();
        void setMoneyMarketMeasure();

        bool isInTerminalMeasure() const;
    private:
        Array rateTimes_, evolutionTimes_;
        std::vector<Size> numeraires_;
        std::vector<std::pair<Size,Size> > relevanceRates_;
        Array taus_;
    };

}

#endif
