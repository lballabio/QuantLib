
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

/*! \file swap.hpp
    \brief Interest rate swap

    $Id$
*/

//  $Source$
//  $Log$
//  Revision 1.4  2001/07/02 12:36:18  sigmud
//  pruned redundant header inclusions
//
//  Revision 1.3  2001/06/21 14:30:43  lballabio
//  Observability is back
//
//  Revision 1.2  2001/05/31 13:17:36  lballabio
//  Added SimpleSwap
//
//  Revision 1.1  2001/05/31 08:56:40  lballabio
//  Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
//

#ifndef quantlib_swap_h
#define quantlib_swap_h

#include "ql/instrument.hpp"
#include "ql/cashflow.hpp"
#include "ql/termstructure.hpp"

namespace QuantLib {

    namespace Instruments {

        //! Interest rate swap
        /*! The cash flows belonging to the first leg are payed;
            the ones belonging to the first leg are received. */
        class Swap : public Instrument {
          public:
            Swap(const std::vector<Handle<CashFlow> >& firstLeg,
                 const std::vector<Handle<CashFlow> >& secondLeg,
                 const RelinkableHandle<TermStructure>& termStructure, 
                 const std::string& isinCode = "",
                 const std::string& description = "");
            ~Swap();
          protected:
            std::vector<Handle<CashFlow> > firstLeg_, secondLeg_;
            RelinkableHandle<TermStructure> termStructure_;
            void performCalculations() const;
        };

    }

}


#endif
