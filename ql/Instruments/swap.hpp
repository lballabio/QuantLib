
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file swap.hpp
    \brief Interest rate swap

    \fullpath
    ql/Instruments/%swap.hpp
*/

// $Id$

#ifndef quantlib_swap_h
#define quantlib_swap_h

#include <ql/instrument.hpp>
#include <ql/cashflow.hpp>
#include <ql/termstructure.hpp>

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
            // added interface
            double firstLegBPS() const;
            double secondLegBPS() const;
          protected:
            // methods
            void performCalculations() const;
            // data members
            std::vector<Handle<CashFlow> > firstLeg_, secondLeg_;
            RelinkableHandle<TermStructure> termStructure_;
            mutable double firstLegBPS_, secondLegBPS_;
        };


        // inline definitions

        inline double Swap::firstLegBPS() const {
            calculate();
            return firstLegBPS_;
        }

        inline double Swap::secondLegBPS() const {
            calculate();
            return secondLegBPS_;
        }

    }

}


#endif
