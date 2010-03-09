/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andrea Odetti

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

/*! \file pathpayoff.hpp
    \brief Option payoff classes
*/

#ifndef quantlib_path_payoff_hpp
#define quantlib_path_payoff_hpp

#include <ql/math/matrix.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/handle.hpp>
#include <functional>

namespace QuantLib {

    //! Abstract base class for path-dependent option payoffs
    class PathPayoff : std::unary_function<Matrix, Disposable<Array> > {
      public:
        virtual ~PathPayoff() {}
        //! \name Payoff interface
        //@{
        /*! \warning This method is used for output and comparison between
                payoffs. It is <b>not</b> meant to be used for writing
                switch-on-type code.
        */
        virtual std::string name() const = 0;
        virtual std::string description() const = 0;


        /*
          This function returns all the payoff and early termination payments 
          for a single path. If the option is cancelled at time i, all payments
          on and before i are taken into account + the value of exercises[i].
          i.e.: cancellation at i does not cancel payments[i]!

          forwardTermStructures contains the yield term structure at each fixing date

          leave states empty to signal exercise is not possible
          in that case, exercises[] will not be accessed.
         */

        virtual void value(const Matrix       & path, 
                           const std::vector<Handle<YieldTermStructure> > & forwardTermStructures,
                           Array              & payments, 
                           Array              & exercises, 
                           std::vector<Array> & states) const = 0;

        /*
          Dimension of the basis functions.
          It must be the same as the size of every element of states in value().
         */

        virtual Size basisSystemDimension() const = 0;

        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    };


    // inline definitions

    inline void PathPayoff::accept(AcyclicVisitor& v) {
        Visitor<PathPayoff>* v1 = dynamic_cast<Visitor<PathPayoff>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            QL_FAIL("not a path-payoff visitor");
    }
}


#endif
