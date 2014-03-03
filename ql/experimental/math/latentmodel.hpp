/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
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
#ifndef quantlib_latent_model_hpp
#define quantlib_latent_model_hpp

#include <vector>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>

#include <ql/experimental/math/multidimquadrature.hpp>
#include <ql/math/randomnumbers/randomsequencegenerator.hpp>

/*! \file latentmodel.hpp
    \brief Generic multifactor latent variable model.
*/

namespace QuantLib {

    namespace detail {
        // havent figured out how to do this in-place
        struct multiplyV {
            typedef std::vector<QuantLib::Real>& result_type;
            std::vector<Real>& operator()(Real d,  std::vector<Real>& v) {
                std::transform(v.begin(), v.end(), v.begin(), 
                    boost::lambda::_1 * d);
            return v;
            }
        };
    }

    // Intended to replace OneFactorCopula

    /*!
    \brief Generic multifactor latent variable model.\par
        In this model set up one considers latent (random) variables 
        \f$ Y_i \f$ described by:
        \f[
        \begin{array}{ccccc}
        Y_1 & = & \sum_k M_k a_{1,k} & + \sqrt{1-\sum_k a_{1,k}^2} Z_1 & 
            \sim \Phi_{Y_1}\nonumber \\
        ... & = &      ... & ...   & \nonumber \\
        Y_i & = & \sum_k M_k a_{i,k} & + \sqrt{1-\sum_k a_{i,k}^2} Z_i & 
            \sim \Phi_{Y_i}\nonumber \\
        ... & = &      ... & ...   & \nonumber \\
        Y_N & = & \sum_k M_k a_{N,k} & + \sqrt{1-\sum_k a_{N,k}^2} Z_N & 
            \sim \Phi_{Y_N}
        \end{array}
        \f]
        where the systemic \f$ M_k \f$ and idiosyncratic \f$ Z_i \f$ (this last 
        one known as error term in some contexts) random variables have 
        independent zero-mean unit-variance distributions. A restriction of the 
        model implemented here is that the N idiosyncratic variables all follow 
        the same probability law \f$ \Phi_Z(z)\f$ (but they are still 
        independent random variables) Also the model is normalized 
        so that: \f$-1\leq a_{i,k} \leq 1\f$ (technically the \f$Y_i\f$ are 
        convex linear combinations). The correlation between \f$Y_i\f$ and 
        \f$Y_j\f$ is then \f$\sum_k a_{i,k} a_{j,k}\f$. 
        \f$\Phi_{Y_i}\f$ denotes the cumulative distribution function of 
        \f$Y_i\f$ which in general differs for each latent variable.\par
        In its single factor set up this model is usually employed in pricing
        and in its multifactorial version in portfolio risk metrics.\par
        The latent variables are typically treated as unobservable magnitudes 
        and they serve to model one or several magnitudes related to them 
        through some function
        \f[
        \begin{array}{ccc}
        F_i(Y_i) & = & 
            F_i(\sum_k M_k a_{i,k} + \sqrt{1-\sum_k a_{i,k}^2} Z_i )\nonumber \\
        & = & F_i(M_1,..., M_k, ..., M_K, Z_i)
        \end{array}
        \f]
        The transfer function can have a more generic form: 
        \f$F_i(Y_1,....,Y_N)\f$ but here the model is restricted to a one to 
        one relation between the latent variables and the modelled ones. Also 
        it is assumed that \f$F_i(y_i; \tau)\f$ is monotonic in \f$y_i\f$; it 
        can then be inverted and the relation of the cumulative probability of 
        \f$F_i\f$ and \f$Y_i\f$ is simple:
        \f[
        \int_{\infty}^b \phi_{F_i} df = 
            \int_{\infty}^{F_i^{-1}(b)} \phi_{Y_i} dy
        \f]
        If  \f$t\f$ is some value of the functional or modelled variable, 
        \f$y\f$ is mapped to \f$t\f$ such that percentiles match, i.e. 
        \f$F_Y(y)=Q_i(t)\f$ or \f$y=F_Y^{-1}(Q_i(t))\f$.
        The class provides an integration facility of arbitrary functions 
        dependent on the model states. It also provides random number generation
        interfaces for usage of the model in monte carlo simulations.\par
        Now let \f$\Phi_Z(z)\f$ be the cumulated distribution function of (all 
        equal as mentioned) \f$Z_i\f$. For a given realization of \f$M_k\f$, 
        this determines the distribution of \f$y\f$:
        \f[
        Prob \,(Y_i < y|M_k) = \Phi_Z \left( \frac{y-\sum_k a_{i,k}\,M_k}
            {\sqrt{1-\sum_k a_{i,k}^2}}\right)
        \qquad
        \mbox{or}
        \qquad
        Prob \,(t_i < t|M) = \Phi_Z \left( \frac
            {F_{Y_{i}}^{-1}(Q_i(t))-\sum_k a_{i,k}\,M_k}
            {\sqrt{1-\sum_k a_{i,k}^2}}
        \right)
        \f]
        The distribution functions of \f$ M_k, Z_i \f$ are specified in
        specific copula template classes. The distribution function 
        of \f$ Y_i \f$ is then given by the convolution
        \f[
        F_{Y_{i}}(y) = Prob\,(Y_i<y) = 
        \int_{-\infty}^\infty\,\cdots\,\int_{-\infty}^{\infty}\:
        D_Z(z)\,\prod_k D_{M_{k}}(m_k) \quad
        \Theta \left(y - \sum_k a_{i,k}m_k - 
            \sqrt{1-\sum_k a_{i,k}^2}\,z\right)\,d\bar{m}\,dz,
        \qquad
        \Theta (x) = \left\{
        \begin{array}{ll}
        1 & x \geq 0 \\
        0 & x < 0
        \end{array}\right.
        \f]
        where \f$ D_Z(z) \f$ and \f$ D_M(m) \f$ are the probability
        densities of \f$ Z\f$ and \f$ M, \f$ respectively.\par
        This convolution can also be written
        \f[
        F_{Y_{i}}(y) = Prob \,(Y_i < y) =
        \int_{-\infty}^\infty\,\cdots\,\int_{-\infty}^{\infty} 
            D_{M_{k}}(m_k)\,dm_k\:
        \int_{-\infty}^{g(y,\vec{a},\vec{m})} D_Z(z)\,dz, \qquad
        g(y,\vec{a},\vec{m}) = \frac{y - \sum_k a_{i,k}m_k}
            {\sqrt{1-\sum_k a_{i,k}^2}}, \qquad \sum_k a_{i,k}^2 < 1
        \f]
        In general, \f$ F_{Y_{i}}(y) \f$ needs to be computed numerically.\par
        The policy class template separates the copula function (the 
        distributions involved) and the functionality (i.e. what the latent  
        model represents: a default probability, a recovery...). Since the  
        copula methods for the 
        probabilities are to be called repeatedly from an integration or a MC 
        simulation, virtual tables are avoided and template parameter mechnics 
        is preferred.\par
        There is nothing at this level enforncing the requirement 
        on the factor distributions to be of zero mean and unit variance. Thats 
        the user responsibility and the model fails to behave correctly if it 
        is not the case.\par
        Derived classes should implement a modelled magnitude (default time, 
        etc) and will provide probability distributions and conditional values.
        They could also provide functionality for the parameter inversion 
        problem, the (e.g.) time at which the modeled variable first takes a 
        given value. This problem has solution/sense depending on the transfer 
        function \f$F_i(Y_i)\f$ characteristics.
    */
    template <class copulaPolicyImpl>
    class LatentModel {
    public:
        //! \name Copula interface.
        //@{
        typedef typename copulaPolicyImpl copulaType;
        /*! Cumulative probability of the \f$ Y_i \f$ modelled latent random 
            variable to take a given value.
        */
        Probability cumulativeY(Real val, Size iVariable) const {
            return copula_.cumulativeY(val, iVariable);
        }
        //! Cumulative distribution of Z, the idiosyncratic/error factors.
        Probability cumulativeZ(Real z) const {
            return copula_.cumulativeZ(z);
        }
        //! Density function of M, the market/systemic factors.
        Probability density(const std::vector<Real>& m) const {
            #if defined(QL_EXTRA_SAFETY_CHECKS)
                QL_REQUIRE(m.size() == nFactors_, 
                    "Factor size must match that of model.");
            #endif
            return copula_.density(m);
        }
        //! Inverse cumulative distribution of the systemic factor iFactor.
        Real inverseCumulativeDensity(Probability p, Size iFactor) const {
            return copula_.inverseCumulativeDensity(p, iFactor);
        }
        /*! Inverse cumulative value of the i-th random latent variable with a 
         given probability. */
        Real inverseCumulativeY(Probability p, Size iVariable) const {
            return copula_.inverseCumulativeY(p, iVariable);
        }
        /*! Inverse cumulative value of the idiosyncratic variable with a given 
        probability. */
        Real inverseCumulativeZ(Probability p) const {
            return copula_.inverseCumulativeZ(p);
        }
        //! All factor cumulative inversion. Used in integrations and sampling.
        Disposable<std::vector<Real> > 
            allFactorCumulInverter(const std::vector<Real>& probs) const {
            return copula_->allFactorCumulInverter(probs);
        }
        //@}

        /*!The value of the latent variable Y_i conditional to (given) a set of 
        values of the factors. */
        Real latentVarValue(const std::vector<Real> m, Size iVar) const {
            return std::inner_product(factorWeights_[iVar].begin(), 
                factorWeights_[iVar].end(), m.begin()) 
                + m.back() * idiosyncFctrs_[iVar];
        }

        friend class InverseLatentVariableRsg;
        //! \name Random number generator facility.
        //@{
        /*!  Allows generation or random samples of the latent variable. 

            RSG will be a uniform sequence generator in the default 
            implementation. Some derived classes might specialize (on the copula
            type) to another type of generator if a more efficient algorithm 
            that the distribution inversion is available rewritig then the 
            nextSequence method for a particular copula implementation.
        */
        template <class RSG>         
        class LatentVariableModelRsg {
            // derive RandomSequenceGenerator? <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        public:
            typedef Sample<std::vector<Real> > sample_type;
            LatentVariableModelRsg(const RSG& rsg)
            : sequenceGenerator_(rsg),
              // dimensions should be coherent (check?)
              x_(std::vector<Real>(rsg.dimension()), 1.0) { }

            /*! Returns a sample of the modelled variables \f$ Y_i \f$. 
            This method has the vocation of being specialized at particular 
            types of the copula with a more efficient inversion to generate the 
            random variables modelled (e.g. Box-Muller for a gaussian).
            Here a default implementation is provided based directly on the 
            inversion of the cumulative distribution from the copula.
             */
            const sample_type& nextSequence() const {
                typename RSG::sample_type sample =
                    sequenceGenerator_.nextSequence();
                // sample.value.size() is equal to the num of factors in the
                // model plus one, this extra one corresponds to the systemic 
                // variable.

                /* Delegate the inversion to the copula policy.
                  Notice theres no ptr to the LM here and thus we are ignorant 
                  of the problem dimensions (how many variables and number of 
                  isiosync factors), we only know the total. And one needs to 
                  split since some factors need to be inverted through the 
                  systemic and some through the idiosyncratic inverse 
                  cumulatives. This is trivial in the Gaussian case where even 
                  the dimensions can be ignored. In the general case this means 
                  the copula has to be aware of the problems dimensions; this 
                  is one argument against the decission to make copula policies 
                  static as it is now.
                */
                // use inversecumulativersg.hpp? only is one dimensional there
                x_.value = 
                    ////////////////   copulaPolicyImpl::allFactorCumulInverter(sample.value);
                    copula_.allFactorCumulInverter(sample.value);////<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                // weight stays 1.
                return x_;
            }
            const sample_type& lastSequence() const { return x_; }
            Size dimension() const { return usg.dimension(); }
        private:
            RSG sequenceGenerator_;
            mutable sample_type x_;
        };
        //@}

        // model size, number of latent variables modelled
        Size size() const {return nVariables_;}
        // 
        Size numFactors() const {return nFactors_;}

        /*! Constructs a LM with an arbitrary number of latent variables
          and factors given by the dimensions of the passed matrix.
            @param factorsWeights Ordering is factorWeights_[iName][iFactor]
            @param quadOrder Quadrature load.
            @param ini Initialization variables. Trait type from the copula 
              policy to allow for static policies (this solution needs to be 
              revised, possibly drop the static policy and create a policy 
              member in LatentModel)
        */
        LatentModel(const std::vector<std::vector<Real> >& factorsWeights, 
            Size quadOrder,
            const typename copulaType::initTraits& ini = 
                copulaType::initTraits());
        /*! Constructs a LM with an arbitrary number of latent variables 
          depending only on one random factor but contributing to each latent
          variable through different weights.
            @param factorsWeights Ordering is factorWeights_[iVariable]
            @param quadOrder Quadrature load.
            @param ini Initialization variables. Trait type from the copula 
              policy to allow for static policies (this solution needs to be 
              revised, possibly drop the static policy and create a policy 
              member in LatentModel)
        */
        LatentModel(const std::vector<Real>& factorsWeight, Size quadOrder,
            const typename copulaType::initTraits& ini = 
                copulaType::initTraits());
        /*! Constructs a LM with an arbitrary number of latent variables 
          depending only on one random factor with the same weight for all
          latent variables.
            @param factorsWeight The weight, same for all.
            @param quadOrder Quadrature load.
            @param ini Initialization variables. Trait type from the copula 
              policy to allow for static policies (this solution needs to be 
              revised, possibly drop the static policy and create a policy 
              member in LatentModel)
        */
        LatentModel(const Real correlSqr, Size nVariables, Size quadOrder,
            const typename copulaType::initTraits& ini = 
                copulaType::initTraits());

        const std::vector<std::vector<Real> >& factorWeights() const {
            return factorWeights_;
        }
        const std::vector<Real>& idiosyncFctrs() const {return idiosyncFctrs_;}

        //! Latent variable correlations:
        Real latentVariableCorrel(Size iVar1, Size iVar2) const {
            // true for any normalized combination
            Real init = (iVar1 == iVar2 ? 
                idiosyncFctrs_[iVar1] * idiosyncFctrs_[iVar1] : 0.);
            return std::inner_product(factorWeights_[iVar1].begin(), 
                factorWeights_[iVar1].end(), factorWeights_[iVar2].begin(), 
                    init);
        }
        //! \name Integration facility interface
        //@{
        /*! Integrates an arbitrary scalar function over the density domain(i.e.
         computes its expected value).
        */
        Real integrate(
            const boost::function<Real(const std::vector<Real>& v1)>& f) const {
            // function composition: composes the integrand with the density 
            //   through a product.
            return 
                integrator_.integrate<Real>(boost::bind(std::multiplies<Real>(), 
                boost::bind(&copulaPolicyImpl::density, copula_, _1),
                boost::bind(boost::cref(f), _1)));   
        }
        /*! Integrates an arbitrary vector function over the density domain(i.e.
         computes its expected value).
        */
        Disposable<std::vector<Real> > integrate(
            const boost::function<std::vector<Real>(
                const std::vector<Real>& v1)>& f ) const {
            return integrator_.integrate<std::vector<Real> >(
                boost::bind<Disposable<std::vector<Real> > >(detail::multiplyV(),
                    boost::bind(&copulaPolicyImpl::density, copula_, _1),
                    boost::bind(boost::cref(f), _1)));
        }
        //@}
    protected:
        // Ordering is: factorWeights_[iVariable][iFactor]
        mutable std::vector<std::vector<Real> > factorWeights_;
        // updated only by correlation observability and constructors.
        // \sqrt{1-\sum_k \beta_{i,k}^2} the addition being along the factors. 
        // It has therefore the size of the basket. Cached for perfomance
        mutable std::vector<Real> idiosyncFctrs_;
        //! Number of systemic factors.
        mutable Size nFactors_;//matches idiosyncFctrs_[0].size();i=0 or any
        //! Number of latent model variables, idiosyncratic terms or model dim
        mutable Size nVariables_;// matches idiosyncFctrs_.size() 

        /* \todo Dont tie it to a Quadrature make the integrator algo generic. 
            something in the line of a factory, yet they do not have a common
            ancestor.
        */
        GaussianQuadMultidimIntegrator integrator_;

        mutable copulaType copula_;
    };


    // Defines -----------------------------------------------------------------

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const std::vector<std::vector<Real> >& factorWeights,
        Size quadOrder,
        const typename Impl::initTraits& ini)
    : factorWeights_(factorWeights),
      nFactors_(factorWeights[0].size()), 
      integrator_(factorWeights[0].size(), quadOrder),
      nVariables_(factorWeights.size()), copula_(factorWeights, ini)
    {
        for(Size i=0; i<factorWeights.size(); i++) {
            idiosyncFctrs_.push_back(std::sqrt(1.-
                    std::inner_product(factorWeights[i].begin(), 
                factorWeights[i].end(), 
                factorWeights[i].begin(), 0.)));
            // while at it, check sizes are coherent:
            QL_REQUIRE(factorWeights[i].size() == nFactors_, 
                "Name " << i << " provides a different number of factors");
        }
    }

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const std::vector<Real>& factorWeights,
        Size quadOrder,
        const typename Impl::initTraits& ini)
    : nFactors_(1),
      nVariables_(factorWeights.size()),
      integrator_(1, quadOrder),
    {
        for(Size iName=0; iName < factorWeights.size(); iName++)
            factorWeights_.push_back(std::vector<Real>(1, 
                factorWeights[iName]));
        for(Size iName=0; iName < factorWeights.size(); iName++)
            idiosyncFctrs_.push_back(std::sqrt(1. - 
                factorWeights[iName]*factorWeights[iName]));
        //convert row to column vector....
        copula_ = copulaType(factorWeights_, ini);
    }

    template <class Impl>
    LatentModel<Impl>::LatentModel(
        const Real correlSqr,
        Size nVariables,
        Size quadOrder,
        const typename Impl::initTraits& ini)
    : factorWeights_(nVariables, std::vector<Real>(1, idiosyncFctrsWeight)),
      nFactors_(1), 
      nVariables_(nVariables),
      idiosyncFctrs_(nVariables, 
        std::sqrt(1.-idiosyncFctrsWeight*idiosyncFctrsWeight)),
      integrator_(1, quadOrder),
      copula_(factorWeights_, ini)
    { }

}                    


#endif
