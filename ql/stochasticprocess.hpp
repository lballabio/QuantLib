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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file stochasticprocess.hpp
    \brief stochastic processes
*/

#ifndef quantlib_stochastic_process_hpp
#define quantlib_stochastic_process_hpp

#include <ql/time/date.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/math/matrix.hpp>

namespace QuantLib {

    //! multi-dimensional stochastic process class.
    /*! This class describes a stochastic process governed by
        \f[
        d\mathrm{x}_t = \mu(t, x_t)\mathrm{d}t
                      + \sigma(t, \mathrm{x}_t) \cdot d\mathrm{W}_t.
        \f]
    */
    class StochasticProcess : public Observer, public Observable {
      public:
        //! discretization of a stochastic process over a given time interval
        class discretization {
          public:
            virtual ~discretization() = default;
            virtual Disposable<Array> drift(const StochasticProcess&,
                                            Time t0, const Array& x0,
                                            Time dt) const = 0;
            virtual Disposable<Matrix> diffusion(
                                              const StochasticProcess&,
                                              Time t0, const Array& x0,
                                              Time dt) const = 0;
            virtual Disposable<Matrix> covariance(
                                              const StochasticProcess&,
                                              Time t0, const Array& x0,
                                              Time dt) const = 0;
        };
        ~StochasticProcess() override = default;
        //! \name Stochastic process interface
        //@{
        //! returns the number of dimensions of the stochastic process
        virtual Size size() const = 0;
        //! returns the number of independent factors of the process
        virtual Size factors() const;
        //! returns the initial values of the state variables
        virtual Disposable<Array> initialValues() const = 0;
        /*! \brief returns the drift part of the equation, i.e.,
                   \f$ \mu(t, \mathrm{x}_t) \f$
        */
        virtual Disposable<Array> drift(Time t,
                                        const Array& x) const = 0;
        /*! \brief returns the diffusion part of the equation, i.e.
                   \f$ \sigma(t, \mathrm{x}_t) \f$
        */
        virtual Disposable<Matrix> diffusion(Time t,
                                             const Array& x) const = 0;
        /*! returns the expectation
            \f$ E(\mathrm{x}_{t_0 + \Delta t}
                | \mathrm{x}_{t_0} = \mathrm{x}_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Disposable<Array> expectation(Time t0,
                                              const Array& x0,
                                              Time dt) const;
        /*! returns the standard deviation
            \f$ S(\mathrm{x}_{t_0 + \Delta t}
                | \mathrm{x}_{t_0} = \mathrm{x}_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Disposable<Matrix> stdDeviation(Time t0,
                                                const Array& x0,
                                                Time dt) const;
        /*! returns the covariance
            \f$ V(\mathrm{x}_{t_0 + \Delta t}
                | \mathrm{x}_{t_0} = \mathrm{x}_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Disposable<Matrix> covariance(Time t0,
                                              const Array& x0,
                                              Time dt) const;
        /*! returns the asset value after a time interval \f$ \Delta t
            \f$ according to the given discretization. By default, it
            returns
            \f[
            E(\mathrm{x}_0,t_0,\Delta t) +
            S(\mathrm{x}_0,t_0,\Delta t) \cdot \Delta \mathrm{w}
            \f]
            where \f$ E \f$ is the expectation and \f$ S \f$ the
            standard deviation.
        */
        virtual Disposable<Array> evolve(Time t0,
                                         const Array& x0,
                                         Time dt,
                                         const Array& dw) const;
        /*! applies a change to the asset value. By default, it
            returns \f$ \mathrm{x} + \Delta \mathrm{x} \f$.
        */
        virtual Disposable<Array> apply(const Array& x0,
                                        const Array& dx) const;
        //@}

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
        void update() override;
        //@}
      protected:
        StochasticProcess() = default;
        explicit StochasticProcess(ext::shared_ptr<discretization>);
        ext::shared_ptr<discretization> discretization_;
    };


    //! 1-dimensional stochastic process
    /*! This class describes a stochastic process governed by
        \f[
            dx_t = \mu(t, x_t)dt + \sigma(t, x_t)dW_t.
        \f]
    */
    class StochasticProcess1D : public StochasticProcess {
      public:
        //! discretization of a 1-D stochastic process
        class discretization {
          public:
            virtual ~discretization() = default;
            virtual Real drift(const StochasticProcess1D&,
                               Time t0, Real x0, Time dt) const = 0;
            virtual Real diffusion(const StochasticProcess1D&,
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
        /*! returns the standard deviation
            \f$ S(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Real stdDeviation(Time t0, Real x0, Time dt) const;
        /*! returns the variance
            \f$ V(x_{t_0 + \Delta t} | x_{t_0} = x_0) \f$
            of the process after a time interval \f$ \Delta t \f$
            according to the given discretization. This method can be
            overridden in derived classes which want to hard-code a
            particular discretization.
        */
        virtual Real variance(Time t0, Real x0, Time dt) const;
        /*! returns the asset value after a time interval \f$ \Delta t
            \f$ according to the given discretization. By default, it
            returns
            \f[
            E(x_0,t_0,\Delta t) + S(x_0,t_0,\Delta t) \cdot \Delta w
            \f]
            where \f$ E \f$ is the expectation and \f$ S \f$ the
            standard deviation.
        */
        virtual Real evolve(Time t0, Real x0, Time dt, Real dw) const;
        /*! applies a change to the asset value. By default, it
            returns \f$ x + \Delta x \f$.
        */
        virtual Real apply(Real x0, Real dx) const;
        //@}
      protected:
        StochasticProcess1D() = default;
        explicit StochasticProcess1D(ext::shared_ptr<discretization>);
        ext::shared_ptr<discretization> discretization_;
      private:
        // StochasticProcess interface implementation
        Size size() const override;
        Disposable<Array> initialValues() const override;
        Disposable<Array> drift(Time t, const Array& x) const override;
        Disposable<Matrix> diffusion(Time t, const Array& x) const override;
        Disposable<Array> expectation(Time t0, const Array& x0, Time dt) const override;
        Disposable<Matrix> stdDeviation(Time t0, const Array& x0, Time dt) const override;
        Disposable<Matrix> covariance(Time t0, const Array& x0, Time dt) const override;
        Disposable<Array> evolve(Time t0, const Array& x0, Time dt, const Array& dw) const override;
        Disposable<Array> apply(const Array& x0, const Array& dx) const override;
    };


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
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(x.size() == 1, "1-D array required");
        #endif
        Array a(1, drift(t, x[0]));
        return a;
    }

    inline Disposable<Matrix> StochasticProcess1D::diffusion(
                                               Time t, const Array& x) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(x.size() == 1, "1-D array required");
        #endif
        Matrix m(1, 1, diffusion(t, x[0]));
        return m;
    }

    inline Disposable<Array> StochasticProcess1D::expectation(
                                    Time t0, const Array& x0, Time dt) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(x0.size() == 1, "1-D array required");
        #endif
        Array a(1, expectation(t0, x0[0], dt));
        return a;
    }

    inline Disposable<Matrix> StochasticProcess1D::stdDeviation(
                                    Time t0, const Array& x0, Time dt) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(x0.size() == 1, "1-D array required");
        #endif
        Matrix m(1, 1, stdDeviation(t0, x0[0], dt));
        return m;
    }

    inline Disposable<Matrix> StochasticProcess1D::covariance(
                                    Time t0, const Array& x0, Time dt) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(x0.size() == 1, "1-D array required");
        #endif
        Matrix m(1, 1, variance(t0, x0[0], dt));
        return m;
    }

    inline Disposable<Array> StochasticProcess1D::evolve(
                                             Time t0, const Array& x0,
                                             Time dt, const Array& dw) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(x0.size() == 1, "1-D array required");
        QL_REQUIRE(dw.size() == 1, "1-D array required");
        #endif
        Array a(1, evolve(t0,x0[0],dt,dw[0]));
        return a;
    }

    inline Disposable<Array> StochasticProcess1D::apply(
                                                      const Array& x0,
                                                      const Array& dx) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(x0.size() == 1, "1-D array required");
        QL_REQUIRE(dx.size() == 1, "1-D array required");
        #endif
        Array a(1, apply(x0[0],dx[0]));
        return a;
    }

}


#endif


#ifndef id_b40e588a88ee2e56a5f2bcf849e0882b
#define id_b40e588a88ee2e56a5f2bcf849e0882b
inline bool test_b40e588a88ee2e56a5f2bcf849e0882b(int* i) { return i != 0; }
#endif
