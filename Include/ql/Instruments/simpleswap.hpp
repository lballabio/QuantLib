
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

/*! \file simpleswap.hpp

    \fullpath
    Include/ql/Instruments/%simpleswap.hpp
    \brief Simple fixed-rate vs Libor swap

*/

#ifndef quantlib_simple_swap_h
#define quantlib_simple_swap_h

#include "ql/Instruments/swap.hpp"
#include "ql/index.hpp"

namespace QuantLib {

    namespace Instruments {

        //! Simple fixed-rate vs Libor swap
        class SimpleSwap : public Swap {
          public:
            SimpleSwap(bool payFixedRate,
                // dates
                const Date& startDate, int n, TimeUnit units,
                const Handle<Calendar>& calendar, 
                RollingConvention rollingConvention, 
                // nominals
                const std::vector<double>& nominals, 
                // fixed leg
                int fixedFrequency, 
                const std::vector<Rate>& couponRates, 
                bool fixedIsAdjusted, 
                const Handle<DayCounter>& fixedDayCount, 
                // floating leg
                int floatingFrequency, 
                const Handle<Index>& index, 
                const std::vector<Spread>& spreads, 
                // hook to term structure
                const RelinkableHandle<TermStructure>& termStructure, 
                // description
                const std::string& isinCode = "",
                const std::string& description = "");
            double BPS() const;
            const Date& maturity() const { return maturity_; }
          private:
            void performCalculations() const;
            bool payFixedRate_;
            Date maturity_;
            mutable double BPS_;
        };


        // inline definitions
        
        inline double SimpleSwap::BPS() const {
            calculate();
            return BPS_;
        }

    }

}


#endif
