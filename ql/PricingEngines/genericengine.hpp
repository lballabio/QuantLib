
/*
 Copyright (C) 2002 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file genericengine.hpp
    \brief Generic option engine

    \fullpath
    ql/PricingEngines/%genericengine.hpp
*/

// $Id$

#ifndef quantlib_generic_engine_h
#define quantlib_generic_engine_h

#include <ql/option.hpp>
#include <ql/types.hpp>
#include <ql/argsandresults.hpp>
#include <ql/pricingengine.hpp>


namespace QuantLib {

    namespace PricingEngines {

        //! template base class for option pricing engines
        /*! Derived engines only need to implement the <tt>calculate()</tt>
            method the inherit from PricingEngine
        */
        template<class ArgumentsType, class ResultsType>
        class GenericEngine : public PricingEngine {
          public:
            Arguments* arguments() { return &arguments_; }
            const Results* results() const { return &results_; }
          protected:
            ArgumentsType arguments_;
            mutable ResultsType results_;
        };


    }

}

#endif

