/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2006 Ferdinando Ametrano
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file payoff.hpp
    \brief Option payoff classes
*/

#ifndef quantlib_payoff_hpp
#define quantlib_payoff_hpp

#include <ql/types.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/errors.hpp>
#include <functional>

namespace QuantLib {

    //! Abstract base class for option payoffs
    class Payoff {
      public:
        virtual ~Payoff() = default;
        //! \name Payoff interface
        //@{
        /*! \warning This method is used for output and comparison between
                payoffs. It is <b>not</b> meant to be used for writing
                switch-on-type code.
        */
        virtual std::string name() const = 0;
        virtual std::string description() const = 0;
        virtual Real operator()(Real price) const = 0;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    };


    // inline definitions

    inline void Payoff::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<Payoff>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            QL_FAIL("not a payoff visitor");
    }

}


#endif
