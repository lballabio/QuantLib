/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Bernd Lewerenz

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

/*! \file continuousarithmeticasianvecerengine.hpp
    \brief Vecer engine for continuous arithmetic Asian options
*/

#ifndef quantlib_continuous_arithmetic_asian_vecer_engine_hpp
#define quantlib_continuous_arithmetic_asian_vecer_engine_hpp

#include <ql/instruments/asianoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Vecer engine for continuous-avaeraging Asian options
    /*! See <http://www.stat.columbia.edu/~vecer/asian-vecer.pdf> */
    class ContinuousArithmeticAsianVecerEngine
        : public ContinuousAveragingAsianOption::engine {
      public:
        ContinuousArithmeticAsianVecerEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            Handle<Quote> currentAverage,
            Date startDate,
            Size timeSteps = 100,
            Size assetSteps = 100,
            Real z_min = -1.0,
            Real z_max = 1.0);
        void calculate() const override;

      protected:
        // Replication of average by holding this amount in assets
        Real cont_strategy(Time t, Time T1,Time T2,Real v, Real r) const; 
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Handle<Quote> currentAverage_ ;
        Date startDate_;
        Real z_min_;
        Real z_max_;
        Size timeSteps_;
        Size assetSteps_;
    };

}


#endif
