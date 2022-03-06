/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file analytic_discr_geom_av_strike.hpp
    \brief Analytic engine for discrete geometric average-strike Asian option
*/

#ifndef quantlib_analytic_discrete_geometric_average_strike_asian_engine_hpp
#define quantlib_analytic_discrete_geometric_average_strike_asian_engine_hpp

#include <ql/instruments/asianoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for European discrete geometric average-strike Asian option
    /*! This class implements a discrete geometric average-strike Asian
        option, with European exercise.  The formula is from "Asian
        Option", E. Levy (1997) in "Exotic Options: The State of the
        Art", edited by L. Clewlow, C. Strickland, pag 65-97

        \test
        - the correctness of the returned value is tested by
          reproducing known good results.

        \ingroup asianengines
    */

    class AnalyticDiscreteGeometricAverageStrikeAsianEngine
        : public DiscreteAveragingAsianOption::engine{
      public:
        AnalyticDiscreteGeometricAverageStrikeAsianEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}

#endif


#ifndef id_957ad3cc7e2227eb25e306e33ecaa427
#define id_957ad3cc7e2227eb25e306e33ecaa427
inline bool test_957ad3cc7e2227eb25e306e33ecaa427(int* i) { return i != 0; }
#endif
