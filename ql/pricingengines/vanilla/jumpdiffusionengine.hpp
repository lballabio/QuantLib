/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Ferdinando Ametrano
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

/*! \file jumpdiffusionengine.hpp
    \brief Jump diffusion (Merton 1976) engine
*/

#ifndef quantlib_jumpdiffusionengine_h
#define quantlib_jumpdiffusionengine_h

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/merton76process.hpp>

namespace QuantLib {

    //! Jump-diffusion engine for vanilla options
    /*! \ingroup vanillaengines

        \test
        - the correctness of the returned value is tested by
          reproducing results available in literature.
        - the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
    */
    class JumpDiffusionEngine : public VanillaOption::engine {
      public:
        JumpDiffusionEngine(std::shared_ptr<Merton76Process>,
                            Real relativeAccuracy_ = 1e-4,
                            Size maxIterations = 100);
        void calculate() const override;

      private:
        std::shared_ptr<Merton76Process> process_;
        Real relativeAccuracy_;
        Size maxIterations_;
    };

}





#endif
