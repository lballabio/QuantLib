/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file shortrateprocess.hpp
    \brief Short rate process

    \fullpath
    ql/InterestRateModelling/%shortrateprocess.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_short_rate_process_h
#define quantlib_interest_rate_modelling_short_rate_process_h

#include <ql/diffusionprocess.hpp>
#include <ql/types.hpp>
#include <ql/InterestRateModelling/parameter.hpp>

namespace QuantLib {

    namespace InterestRateModelling {

        //describes a process followed by the short rate or a function of it
        class ShortRateProcess : public DiffusionProcess {
          public:
            virtual ~ShortRateProcess() {};

            virtual double variable(Time t, Rate r) const = 0;
            virtual Rate shortRate(Time t, double variable) const = 0;
        };

        class MeanRevertingProcess : public ShortRateProcess {
          public:
            MeanRevertingProcess(const Parameter& mean,
                                 const Parameter& speed,
                                 const Parameter& volatility)
            : mean_(mean), speed_(speed), volatility_(volatility) {}

            virtual double drift(Time t, double x) const {
                return speed_(t)*(mean_(t) - x);
            }

            virtual double diffusion(Time t, double x) const {
                return volatility_(t);
            }

          protected:
            Parameter mean_, speed_, volatility_;
        };

        class OrnsteinUhlenbeckProcess : public MeanRevertingProcess {
          public:
            OrnsteinUhlenbeckProcess(const Parameter& speed,
                                     const Parameter& volatility)
            : MeanRevertingProcess(NullParameter(), speed, volatility)  {}

            virtual double expectation(Time t0, double x0, Time dt) const {
                return x0*QL_EXP(-speed_(t0)*dt);
            }
            virtual double variance(Time t0, double x0, Time dt) const {
                return 0.5*volatility_(t0)*volatility_(t0)/speed_(t0)*
                       (1.0 - QL_EXP(-2.0*speed_(t0)*dt));
            }
        };

    }

}


#endif
