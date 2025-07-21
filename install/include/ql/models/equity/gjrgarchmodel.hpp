/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Yee Man Chan

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

/*! \file gjrgarchmodel.hpp
    \brief GJR-GARCH model for the stochastic volatility of an asset
*/

#ifndef quantlib_gjrgarch_model_hpp
#define quantlib_gjrgarch_model_hpp

#include <ql/models/model.hpp>
#include <ql/processes/gjrgarchprocess.hpp>

namespace QuantLib {

    //! GJR-GARCH model for the stochastic volatility of an asset
    /*! References:

        Glosten, L., Jagannathan, R., Runkle, D., 1993. 
    Relationship between the expected value and the volatility
    of the nominal excess return on stocks. Journal of Finance
    48, 1779-1801

        \test calibration is not implemented for GJR-GARCH
    */
    class GJRGARCHModel : public CalibratedModel {
      public:
        GJRGARCHModel(const ext::shared_ptr<GJRGARCHProcess>& process);

        // variance mean reversion level multiplied by
        // the proportion not accounted by alpha, beta and gamma
        Real omega() const { return arguments_[0](0.0); }
        // proportion attributed to the impact of all innovations
        Real alpha() const { return arguments_[1](0.0); }
        // proportion attributed to the impact of previous variance
        Real beta() const { return arguments_[2](0.0); }
        // proportion attributed to the impact of negative innovations
        Real gamma()   const { return arguments_[3](0.0); }
        // market price of risk
        Real lambda()   const { return arguments_[4](0.0); }
        // spot variance
        Real v0()    const { return arguments_[5](0.0); }

        // underlying process
        ext::shared_ptr<GJRGARCHProcess> process() const { return process_; }

        class VolatilityConstraint;
      protected:
        void generateArguments() override;
        ext::shared_ptr<GJRGARCHProcess> process_;
    };
}


#endif

