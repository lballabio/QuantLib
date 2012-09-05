/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file option.hpp
    \brief Base option class
*/

#ifndef quantlib_option_hpp
#define quantlib_option_hpp

#include <ql/instrument.hpp>

namespace QuantLib {

    class Payoff;
    class Exercise;

    //! base option class
    class Option : public Instrument {
      public:
        class arguments;
        enum Type { Put = -1,
                    Call = 1
        };
        Option(const boost::shared_ptr<Payoff>& payoff,
               const boost::shared_ptr<Exercise>& exercise)
        : payoff_(payoff), exercise_(exercise) {}
        void setupArguments(PricingEngine::arguments*) const;
        boost::shared_ptr<Payoff> payoff() { return payoff_; }
        boost::shared_ptr<Exercise> exercise() { return exercise_; };
      protected:
        // arguments
        boost::shared_ptr<Payoff> payoff_;
        boost::shared_ptr<Exercise> exercise_;
    };

    /*! \relates Option */
    std::ostream& operator<<(std::ostream&, Option::Type);

    //! basic %option %arguments
    class Option::arguments : public virtual PricingEngine::arguments {
      public:
        arguments() {}
        void validate() const {
            QL_REQUIRE(payoff, "no payoff given");
            QL_REQUIRE(exercise, "no exercise given");
        }
        boost::shared_ptr<Payoff> payoff;
        boost::shared_ptr<Exercise> exercise;
    };

    //! additional %option results
    class Greeks : public virtual PricingEngine::results {
      public:
        void reset() {
            delta =  gamma = theta = vega =
                rho = dividendRho = Null<Real>();
        }
        Real delta, gamma;
        Real theta;
        Real vega;
        Real rho, dividendRho;
    };

    //! more additional %option results
    class MoreGreeks : public virtual PricingEngine::results {
      public:
        void reset() {
            itmCashProbability = deltaForward = elasticity = thetaPerDay =
                strikeSensitivity = Null<Real>();
        }
        Real itmCashProbability, deltaForward, elasticity, thetaPerDay,
             strikeSensitivity;
    };


    // inline definitions

    inline void Option::setupArguments(PricingEngine::arguments* args) const {
        Option::arguments* arguments =
            dynamic_cast<Option::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->payoff = payoff_;
        arguments->exercise = exercise_;
    }

    inline std::ostream& operator<<(std::ostream& out, Option::Type type) {
        switch (type) {
          case Option::Call:
            return out << "Call";
          case Option::Put:
            return out << "Put";
          default:
            QL_FAIL("unknown option type");
        }
    }

}


#endif
