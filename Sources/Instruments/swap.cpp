
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file swap.cpp
    \brief Interest rate swap

    $Id$
*/

//  $Source$
//  $Log$
//  Revision 1.1  2001/05/31 08:56:40  lballabio
//  Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
//

#include "ql/Instruments/swap.hpp"

namespace QuantLib {

    namespace Instruments {

        Swap::Swap(const std::vector<Handle<CashFlow> >& firstLeg,
            const std::vector<Handle<CashFlow> >& secondLeg,
            const RelinkableHandle<TermStructure>& termStructure, 
            const std::string& isinCode, const std::string& description)
        : Instrument(isinCode,description), firstLeg_(firstLeg), 
            secondLeg_(secondLeg), termStructure_(termStructure) {}
    
        void Swap::performCalculations() const {
            QL_REQUIRE(!termStructure_.isNull(),
                "trying to price swap on null term structure");
            Date settlement = termStructure_->settlementDate();
            NPV_ = 0.0;
            isExpired_ = true;
            // subtract first leg cash flows
            for (int i=0; i<firstLeg_.size(); i++) {
                Date cashFlowDate = firstLeg_[i]->date();
                if (cashFlowDate >= settlement) {
                    isExpired_ = false;  // keeping track of whether it was set 
                                         // already isn't worth the effort
                    NPV_ -= firstLeg_[i]->amount() *
                        termStructure_->discount(cashFlowDate);
                }
            }
            // add second leg cash flows
            for (int j=0; j<secondLeg_.size(); j++) {
                Date cashFlowDate = secondLeg_[j]->date();
                if (cashFlowDate >= settlement) {
                    isExpired_ = false;
                    NPV_ += secondLeg_[j]->amount() *
                        termStructure_->discount(cashFlowDate);
                }
            }
        }
    
    }

}
