/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Jose Aparicio

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

#ifndef convolved_student_t_hpp
#define convolved_student_t_hpp

#include <ql/types.hpp>
#include <ql/utilities/disposable.hpp>
#include <vector>
#include <numeric>
#include <functional>

namespace QuantLib {

    /*! \brief Cumulative (generalized) BehrensFisher distribution.

    Exact analitical computation of the cumulative probability distribution of
    the linear combination of an arbitrary number (not just two) of T random
    variables of odd integer order. Adapted from the algorithm in:\par
        V. Witkovsky, Journal of Statistical Planning and Inference 94
        (2001) 1-13\par
    see also:\par
        On the distribution of a linear combination of t-distributed
        variables; Glenn Alan Walker, Ph.D.thesis University of Florida 1977\par
        'Convolutions of the T Distribution'; S. Nadarajah, D. K. Dey in
        Computers and Mathematics with Applications 49 (2005) 715-721\par
    The last reference provides direct expressions for some of the densities
    when the linear combination of only two Ts is just an addition. It can be
    used for testing the results here.\par
    Another available test on this algorithm stems from the realization that a
    linear convex (\f$ \sum a_i=1\f$) combination of Ts of order one is stable
    in the distribution sense (but this result is often of no practical use
    because of its non-finite variance).\par
    This implementation is for two or more T variables in the linear
    combination albeit these must be of odd order. The case of exactly two T of
    odd order is known to be a finite mixture of Ts but that result is not used
    here. On this line see 'Linearization coefficients of Bessel polynomials'
    C.Berg, C.Vignat; February 2008; arXiv:math/0506458

        \todo Implement the series expansion solution for the addition of
        two Ts of even order described in: 'On the density of the sum of two
        independent Student t-random vectors' C.Berg, C.Vignat; June 2009;
        eprint arXiv:0906.3037
    */
    class CumulativeBehrensFisher { // ODD orders only by now, rename?
    public:
        typedef Probability result_type;
        typedef Real argument_type;
        /*!
            @param degreesFreedom Degrees of freedom of the Ts convolved. The
                algorithm is limited to odd orders only.
            @param factors Factors in the linear combination of the Ts.
        */
        CumulativeBehrensFisher(
            const std::vector<Integer>& degreesFreedom = std::vector<Integer>(),
            const std::vector<Real>& factors = std::vector<Real>());

        //! Degrees of freedom of the Ts involved in the convolution.
        const std::vector<Integer>& degreeFreedom() const {
            return degreesFreedom_;
        }
        //! Factors in the linear combination.
        const std::vector<Real>& factors() const {
            return factors_;
        }
    private:
        /*! \brief Student t characteristic polynomials.

        Generates the polynomial coefficients defining the characteristic
        function of a T distribution \f$T_\nu\f$ of odd order; \f$\nu=2n+1\f$.
        In general the characteristic function is given by:
        \f[
        \phi_{\nu}(t) = \varphi_{n}(t) \exp{-\nu^{1/2}|t|} ;\,where\,\nu = 2n+1
        \f]
        where \f$ \varphi \f$ are polynomials that are computed recursively.

        The convolved characteristic function is the product of the two previous
        characteristic functions and the problem is then the convolution (a
        product) of two polynomials.

            @param n Natural number defining the order of the T for which
            the characteristic function is to be computed. The order of the
             T is then \f$ \nu=2n+1 \f$
        */
        // move outside of the class, as a separate problem?
        Disposable<std::vector<Real> > polynCharactT(Natural n) const;

        Disposable<std::vector<Real> > convolveVectorPolynomials(
            const std::vector<Real>& v1,
            const std::vector<Real>& v2) const ;
    public:
        /*! \brief Returns the cumulative probability of the resulting
        distribution.\par
            To obtain the cumulative probability the Gil-Pelaez theorem
              is applied:\par
            First compute the characteristic function of the linear combination
            variable by multiplying the individual characteristic functions.
            Then transform back integrating the characteristic function
            according to the GP theorem; this is done here analytically feeding
            in the expression of the total characteristic
            function this:
            \f[ \int_0^{\infty}x^n e^{-ax}sin(bx)dx =
                (-1)^n \Gamma(n+1) \frac{sin((n+1)arctg2(-b/a))}
                    {(\sqrt{a^2+b^2})^{n+1}}; for\,a>0,\,b>0
            \f]
            and for the first term I use:
            \f[
            \int_0^{\infty} \frac{e^{-ax}sin(bx)}{x} dx = arctg2(b/a)
            \f]
            The GP complex integration is simplified thanks to the symetry of
            the distribution.
        */
      Probability operator()(Real x) const;

      /*! \brief Returns the probability density of the resulting
      distribution.\par
          Similarly to the cumulative probability, Gil-Pelaez theorem is
          applied, the integration is similar.

          \todo Implement in a separate class? given the name of this class..
      */
      Probability density(Real x) const;

    private:
        mutable std::vector<Integer> degreesFreedom_;
        mutable std::vector<Real> factors_;

        mutable std::vector<std::vector<Real> > polynCharFnc_;
        mutable std::vector<Real> polyConvolved_;

        // cached factor in the exponential of the characteristic function
        mutable Real a_ = 0., a2_;
    };



    /*! \brief Inverse of the cumulative of the convolution of odd-T
    distributions

    Finds the inverse through a root solver. To find limits for the solver
    domain use is made of the property that the convolved distribution is
    bounded above by the normalized gaussian. If the coeffiecient in the linear
    combination add up to a number below one the T of order one can be used as
    a limit below but in general this is not necessarily the case and a constant
    is used.
    Also the fact that the combination is symmetric is used.
     */
    class InverseCumulativeBehrensFisher {
    public:
        typedef Real result_type;
        typedef Probability argument_type;
        /*!
            @param degreesFreedom Degrees of freedom of the Ts convolved. The
                algorithm is limited to odd orders only.
            @param factors Factors in the linear combination of the Ts.
            @param accuracy The accuracy of the root-solving process.
        */
        InverseCumulativeBehrensFisher(
            const std::vector<Integer>& degreesFreedom = std::vector<Integer>(),
            const std::vector<Real>& factors = std::vector<Real>(),
            Real accuracy = 1.e-6);
        //! Returns the cumulative inverse value.
        Real operator()(Probability q) const;

      private:
        mutable Real normSqr_, accuracy_;
        mutable CumulativeBehrensFisher distrib_;
    };

}

#endif
