/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file juquadraticengine.hpp
    \brief Ju quadratic (1999) approximation engine
*/

#ifndef quantlib_ju_quadratic_engine_hpp
#define quantlib_ju_quadratic_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for American options with Ju quadratic approximation
    /*! Reference:
        An Approximate Formula for Pricing American Options,
        Journal of Derivatives Winter 1999,
        Ju, N.

        \warning Barone-Adesi-Whaley critical commodity price
                 calculation is used, it has not been modified to see
                 whether the method of Ju is faster. Ju does not say
                 how he solves the equation for the critical stock
                 price, e.g. Newton method. He just gives the
                 solution.  The method of BAW gives answers to the
                 same accuracy as in Ju (1999).

        \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class JuQuadraticApproximationEngine
        : public VanillaOption::engine {
      public:
        JuQuadraticApproximationEngine(
                    const boost::shared_ptr<GeneralizedBlackScholesProcess>&);
        void calculate() const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif
