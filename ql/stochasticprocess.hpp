/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file stochasticprocess.hpp
    \brief stochastic processes
*/

#ifndef quantlib_stochastic_process_hpp
#define quantlib_stochastic_process_hpp

#include <ql/date.hpp>
#include <ql/Patterns/observable.hpp>
#include <ql/Math/matrix.hpp>

namespace QuantLib {

    //! multi dimensional Stochastic process class.
    /*! This class describes a stochastic process governed by
        \f[
        d\mathrm{x}_t = \mu(t, x_t)\mathrm{d}t
                      + \sigma(t, \mathrm{x}_t) \cdot d\mathrm{W}_t.
        \f]
    */
    class GenericStochasticProcess : public Observer, public Observable {
      public:
        //! discretization of a stochastic process over a given time interval
        class discretization {
          public:
            virtual ~discretization() {}
            virtual Disposable<Array> expectation(
                                              const GenericStochasticProcess&,
                                              Time t0, const Array& x0,
                                              Time dt) const = 0;
            virtual Disposable<Matrix> covariance(
                                              const GenericStochasticProcess&,
                                              Time t0, const Array& x0,
                                              Time dt) const = 0;
        };
        virtual ~GenericStochasticProcess() {}
        //! \name Stochastic process interface
        //@{
        //! returns the number of dimensions of the stochastic process
        virtual Size size() const = 0;
        //! returns the initial values of the state variables
        virtual Disposable<Array> initialValues() const = 0;
        /*! \brief returns the drift part of the equation, i.e.,
                   \f$ \mu(t, \mathrm{x}_t) \f$
        */
        virtual Disposable<Array> drift(Time t, const Array& x) const = 0;
        /*! \brief returns the diffusion part of the equation, i.e.
                   \f$ \sigma(t, \mathrm{x}_t) \f$
        */
        virtual Disposable<Matrix> diffusion(Time t, const Array& x) const = 0;
        /*! returns the expectation
            \f$ E(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Disposable<Array> expectation(Time t0, const Array& x0,
                                              Time dt) const;
        /*! returns the covariance
            \f$ V(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Disposable<Matrix> covariance(Time t0, const Array& x0,
                                              Time dt) const;
        /*! applies a change to the asset value. By default; it
            returns \f$ x + \Delta x \f$.
        */
        virtual Disposable<Array> evolve(const Array& change,
                                         const Array& currentValue) const;

        //! \name utilities
        //@{
        /*! returns the time value corresponding to the given date
            in the reference system of the stochastic process.

            \note As a number of processes might not need this
                  functionality, a default implementation is given
                  which raises an exception.
        */
        virtual Time time(const Date&) const;
        //@}

        //! \name Observer interface
        //@{
        void update();
        //@}
      protected:
        GenericStochasticProcess();
        GenericStochasticProcess(const boost::shared_ptr<discretization>&);
        boost::shared_ptr<discretization> discretization_;
    };


    //! 1-dimensional stochastic process
    /*! This class describes a stochastic process governed by
        \f[
            dx_t = \mu(t, x_t)dt + \sigma(t, x_t)dW_t.
        \f]
    */
    class StochasticProcess1D : public GenericStochasticProcess {
      public:
        //! discretization of a 1-D stochastic process
        class discretization {
          public:
            virtual ~discretization() {}
            virtual Real expectation(const StochasticProcess1D&,
                                     Time t0, Real x0, Time dt) const = 0;
            virtual Real variance(const StochasticProcess1D&,
                                  Time t0, Real x0, Time dt) const = 0;
        };
        //! \name 1-D stochastic process interface
        //@{
        //! returns the initial value of the state variable
        virtual Real x0() const = 0;
        //! returns the drift part of the equation, i.e. \f$ \mu(t, x_t) \f$
        virtual Real drift(Time t, Real x) const = 0;
        /*! \brief returns the diffusion part of the equation, i.e.
            \f$ \sigma(t, x_t) \f$
        */
        virtual Real diffusion(Time t, Real x) const = 0;
        /*! returns the expectation
            \f$ E(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Real expectation(Time t0, Real x0, Time dt) const;
        /*! returns the variance
            \f$ V(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Real variance(Time t0, Real x0, Time dt) const;
        /*! applies a change to the asset value. By default; it
            returns \f$ x + \Delta x \f$.
        */
        virtual Real evolve(Real change, Real currentValue) const;
        //@}
      protected:
        StochasticProcess1D();
        StochasticProcess1D(const boost::shared_ptr<discretization>&);
        boost::shared_ptr<discretization> discretization_;
      private:
        // GenericStochasticProcess interface implementation
        Size size() const;
        Disposable<Array> initialValues() const;
        Disposable<Array> drift(Time t, const Array& x) const;
        Disposable<Matrix> diffusion(Time t, const Array& x) const;
        Disposable<Array> expectation(Time t0, const Array& x0,
                                      Time dt) const;
        Disposable<Matrix> covariance(Time t0, const Array& x0,
                                      Time dt) const;
        Disposable<Array> evolve(const Array& change,
                                 const Array& currentValue) const;
    };

    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to StochasticProcess1D */
    typedef StochasticProcess1D StochasticProcess;
    #endif


    // inline definitions

    inline Size StochasticProcess1D::size() const {
        return 1;
    }

    inline Disposable<Array> StochasticProcess1D::initialValues() const {
        Array a(1, x0());
        return a;
    }

    inline Disposable<Array> StochasticProcess1D::drift(
                                               Time t, const Array& x) const {
        QL_REQUIRE(x.size() == 1, "1-D array required");
        Array a(1, drift(t, x[0]));
        return a;
    }

    inline Disposable<Matrix> StochasticProcess1D::diffusion(
                                               Time t, const Array& x) const {
        QL_REQUIRE(x.size() == 1, "1-D array required");
        Matrix m(1, 1, diffusion(t, x[0]));
        return m;
    }

    inline Disposable<Array> StochasticProcess1D::expectation(
                                    Time t0, const Array& x0, Time dt) const {
        QL_REQUIRE(x0.size() == 1, "1-D array required");
        Array a(1, expectation(t0, x0[0], dt));
        return a;
    }

    inline Disposable<Matrix> StochasticProcess1D::covariance(
                                    Time t0, const Array& x0, Time dt) const {
        QL_REQUIRE(x0.size() == 1, "1-D array required");
        Matrix m(1, 1, variance(t0, x0[0], dt));
        return m;
    }

    inline Disposable<Array> StochasticProcess1D::evolve(
                                            const Array& change,
                                            const Array& currentValue) const {
        QL_REQUIRE(change.size() == 1, "1-D array required");
        QL_REQUIRE(currentValue.size() == 1, "1-D array required");
        Array a(1, evolve(change[0],currentValue[0]));
        return a;
    }

}


#endif
