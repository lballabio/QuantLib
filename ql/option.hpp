
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

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
#include <ql/payoff.hpp>
#include <ql/exercise.hpp>

namespace QuantLib {

    //! base option class
    class Option : public Instrument {
      public:
        class arguments;
        enum Type { Call, Put };
        Option(const boost::shared_ptr<Payoff>& payoff,
               const boost::shared_ptr<Exercise>& exercise,
               const boost::shared_ptr<PricingEngine>& engine =
                                          boost::shared_ptr<PricingEngine>())
        : payoff_(payoff), exercise_(exercise) {
            if (engine)
                setPricingEngine(engine);
        }
      protected:
        // arguments
        boost::shared_ptr<Payoff> payoff_;
        boost::shared_ptr<Exercise> exercise_;
    };


    /*! basic %option arguments

        \todo
        - remove std::vector<Time> stoppingTimes
        - how to handle strike-less option (asian average strike,
          forward, etc.)?
    */
    class Option::arguments : public virtual Arguments {
      public:
        arguments() {}
        void validate() const {
            QL_REQUIRE(payoff, "no payoff given");
        }
        boost::shared_ptr<Payoff> payoff;
        boost::shared_ptr<Exercise> exercise;
        // shouldn't be here
        // it should be moved elsewhere
        std::vector<Time> stoppingTimes;
    };

    //! additional %option results
    class Greeks : public virtual Results {
      public:
        Greeks() { reset(); }
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
    // add here vomma, ect.
    class MoreGreeks : public virtual Results {
      public:
        MoreGreeks() { reset(); }
        void reset() {
            itmCashProbability = deltaForward = elasticity = thetaPerDay =
                strikeSensitivity = Null<Real>();
        }
        Real itmCashProbability, deltaForward, elasticity, thetaPerDay,
             strikeSensitivity;
    };


    //! format option type for output
    class OptionTypeFormatter {
      public:
        static std::string toString(Option::Type type) {
            switch (type) {
              case Option::Call:     return "call";
              case Option::Put:      return "put";
              default:
                QL_FAIL("unknown option type");
            }
        }
    };

}


#endif
