/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file continuousarithmeticasianlevyengine.hpp
    \brief Levy engine for continuous arithmetic Asian options
*/

#ifndef quantlib_continuous_arithmetic_asian_levy_engine_hpp
#define quantlib_continuous_arithmetic_asian_levy_engine_hpp

#include <ql/instruments/asianoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    class ContinuousArithmeticAsianLevyEngine
        : public ContinuousAveragingAsianOption::engine {
      public:
        ContinuousArithmeticAsianLevyEngine(
            const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
            const Handle<Quote>& currentAverage,
            Date startDate);
        void calculate() const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Handle<Quote> currentAverage_ ;
        Date startDate_;
    };

}


#endif
