/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 StatPro Italia srl

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

/*! \file perturbativebarrieroptionengine.hpp
    \brief perturbative barrier-option engine
*/

#ifndef quantlib_perturbative_barrier_option_engine_hpp
#define quantlib_perturbative_barrier_option_engine_hpp

#include <ql/instruments/barrieroption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! perturbative barrier-option engine
    /*! This engine implements the approach described in
        <http://www.econ.univpm.it/recchioni/finance/w3/>.

        \warning This was reported to fail tests on Mac OS X 10.8.4.

        \ingroup barrierengines
    */
    class PerturbativeBarrierOptionEngine : public BarrierOption::engine  {
      public:
        explicit PerturbativeBarrierOptionEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>,
                                                 Natural order = 1,
                                                 bool zeroGamma = false);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Natural order_;
        bool zeroGamma_;
    };

}


#endif



#ifndef id_bc4eb59fb038920d747ff79b04f75de3
#define id_bc4eb59fb038920d747ff79b04f75de3
inline bool test_bc4eb59fb038920d747ff79b04f75de3(const int* i) {
    return i != nullptr;
}
#endif
