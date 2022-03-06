/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Andres Hernandez

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

/*! \file levyflightdistribution.hpp
    \brief Levy Flight, aka Pareto Type I, distribution as needed by Boost Random
*/

#ifndef quantlib_levy_flight_distribution_hpp
#define quantlib_levy_flight_distribution_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <boost/random/detail/config.hpp>
#include <boost/random/detail/operators.hpp>
#include <boost/random/uniform_01.hpp>
#include <iosfwd>

namespace QuantLib {

    //! Levy Flight distribution as needed by Boost Random
    /*! The levy flight distribution is a random distribution with 
        the following form:
        \f[
        p(x) = \frac{\alpha x_m^{\alpha}}{x^{\alpha+1}}
        \f]
        with support over \f$ x \in [x_m, \infty) \f$
        and the parameter \f$ \alpha > 0 \f$.

        Levy Flight is normally defined as \f$ x_m = 1 \f$ and \f$ 0 <
        \alpha < 2 \f$, which is where \f$ p(x) \f$ has an infinite
        variance. However, the more general version, known as Pareto
        Type I, is well defined for \f$ \alpha > 2 \f$, so the current
        implementation does not restrict \f$ \alpha \f$ to be smaller
        than 2.
    */
    class LevyFlightDistribution
    {
      public:
        typedef Real input_type;
        typedef Real result_type;

        class param_type
        {
          public:

            typedef LevyFlightDistribution distribution_type;

            /*!    Constructs parameters with a given xm and alpha
                Requires: alpha > 0
            */
            param_type(Real xm = 1.0, Real alpha = 1.0)
              : xm_(xm), alpha_(alpha) { QL_REQUIRE(alpha_ > 0.0, "alpha must be larger than 0"); }

            //! Returns the xm parameter of the distribution
            Real xm() const { return xm_; }
            
            //! Returns the alpha parameter of the distribution
            Real alpha() const { return alpha_; }

            //! Writes the parameters to a @c std::ostream
            BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(os, param_type, parm)
            {
                os << parm.xm_ << " " << parm.alpha_;
                return os;
            }
            
            //! Reads the parameters from a @c std::istream
            BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(is, param_type, parm)
            {
                is >> parm.xm_ >> std::ws >> parm.alpha_;
                return is;
            }

            //! Returns true if the two sets of parameters are equal
            BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(param_type, lhs, rhs)
            { return lhs.xm_ == rhs.xm_ && lhs.alpha_ == rhs.alpha_; }

            //! Returns true if the two sets of parameters are different
            BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(param_type)

        private:
            Real xm_;
            Real alpha_;
        };

        //! \name Constructors
        //@{
        /*! Constructs a LevyFlightDistribution with a given xm and alpha
            Requires: alpha > 0
        */
        explicit LevyFlightDistribution(Real xm = 1.0, Real alpha = 1.0)
          : xm_(xm), alpha_(alpha) { QL_REQUIRE(alpha_ > 0.0, "alpha must be larger than 0"); }

        //!Constructs a LevyFlightDistribution from its parameters
        explicit LevyFlightDistribution(const param_type& parm)
          : xm_(parm.xm()), alpha_(parm.alpha()) {}

        // compiler-generated copy ctor and assignment operator are fine
        //@}

        //! \name Inspectors
        //@{
        //! Returns the xm parameter of the distribution
        Real xm() const { return xm_; }
            
        //! Returns the alpha parameter of the distribution
        Real alpha() const { return alpha_; }

        //! Returns the smallest value that the distribution can produce
        Real min BOOST_PREVENT_MACRO_SUBSTITUTION () const
        { return xm_; }
        //! Returns the largest value that the distribution can produce
        Real max BOOST_PREVENT_MACRO_SUBSTITUTION () const
        { return QL_MAX_REAL; }

        //! Returns the parameters of the distribution
        param_type param() const { return {xm_, alpha_}; }
        //@}
        
        //! Sets the parameters of the distribution
        void param(const param_type& parm) { 
            xm_ = parm.xm();
            alpha_ = parm.alpha();
        }

        /*! Effects: Subsequent uses of the distribution do not depend
            on values produced by any engine prior to invoking reset.
        */
        void reset() { }
        
        //! Returns the value of the pdf for x
        Real operator()(Real x) const{
            using std::pow;
            if(x < xm_) return 0.0;
            return alpha_*pow(xm_/x, alpha_)/x;
        }
        
        /*!    Returns a random variate distributed according to the
            levy flight distribution.
        */
        template<class Engine>
        result_type operator()(Engine& eng) const{
            using std::pow;
            return xm_*pow(boost::random::uniform_01<Real>()(eng), -1.0/alpha_);
        }

        /*!    Returns a random variate distributed according to the
            levy flight with parameters specified by parm
        */
        template<class Engine>
        result_type operator()(Engine& eng, const param_type& parm) const{
            return LevyFlightDistribution (parm)(eng);
        }

        //! Writes the distribution to a std::ostream
        BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(os, LevyFlightDistribution, ed)
        {
            os << ed.xm_ << " " << ed.alpha_;
            return os;
        }

        //! Reads the distribution from a std::istream
        BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(is, LevyFlightDistribution, ed)
        {
            is >> ed.xm_ >> std::ws >> ed.alpha_;
            return is;
        }

        /*! Returns true iff the two distributions will produce identical
            sequences of values given equal generators.
        */
        BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(LevyFlightDistribution, lhs, rhs)
        { return lhs.xm_ == rhs.xm_ && lhs.alpha_ == rhs.alpha_; }
        
        /*!    Returns true iff the two distributions will produce different
            sequences of values given equal generators.
        */
        BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(LevyFlightDistribution)

    private:
        result_type xm_;
        result_type alpha_;
    };

}

#endif


#ifndef id_b727629ec5098bbe8e0a76e8ffca7164
#define id_b727629ec5098bbe8e0a76e8ffca7164
inline bool test_b727629ec5098bbe8e0a76e8ffca7164(const int* i) {
    return i != nullptr;
}
#endif
