/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file bjerksundstenslandengine.hpp
    \brief Bjerksund and Stensland approximation engine
*/

#ifndef quantlib_bjerkland_stensland_engine_hpp
#define quantlib_bjerkland_stensland_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Bjerksund and Stensland pricing engine for American options (1993)
    /*! \ingroup vanillaengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class BjerksundStenslandApproximationEngine
        : public VanillaOption::engine {
      public:
        BjerksundStenslandApproximationEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>);
        void calculate() const override;

      private:
        OneAssetOption::results americanCallApproximation(
            Real S, Real X, Real rfD, Real dD, Real variance) const;
        OneAssetOption::results europeanCallResults(
            Real S, Real X, Real rfD, Real dD, Real variance) const;
        OneAssetOption::results immediateExercise(Real S, Real X) const;


        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif
