/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Yue Tian

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

/*! \file multibootstrap.hpp
    \brief multiple bootstrapper for simultanaous bootstrap of multiple curves.
*/

#ifndef quantlib_multi_bootstrap_hpp
#define quantlib_multi_bootstrap_hpp

#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/bootstraphelper.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/armijo.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/leastsquare.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/termstructures/iterativebootstrap.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

	// penalty function class for multiple curve
    template <class Curve>
    class MultiCurvePenaltyFunction : public CostFunction {
	public:
        typedef typename Curve::traits_type Traits;
        typedef typename Traits::helper helper;
        typedef
          typename std::vector< boost::shared_ptr<helper> >::const_iterator helper_iterator;
      
        MultiCurvePenaltyFunction(std::vector<Curve*> curves,
								  std::vector<Size> initialIndexes,
								  std::vector<helper_iterator> rateHelpersStarts,
								  std::vector<helper_iterator> rateHelpersEnds)
        : curves_(curves), initialIndexes_(initialIndexes),
          rateHelpersStarts_(rateHelpersStarts), rateHelpersEnds_(rateHelpersEnds) {
        }

        Real value(const Array& x) const;
        Disposable<Array> values(const Array& x) const;

      private:
        std::vector<Curve*> curves_;
        std::vector<Size> initialIndexes_;
        std::vector<helper_iterator> rateHelpersStarts_;
        std::vector<helper_iterator> rateHelpersEnds_;
    };


	template <class Curve>
	class MultiCurveOptimizer : public LazyObject {
	  public:
	    MultiCurveOptimizer(bool forcePositive = true) 
		: accuracy_(1.0e-12), forcePositive_(forcePositive){};
		void addTermStructure(Curve* c) {
			for (Size i=0; i<c->instruments_.size(); ++i){
				this->registerWith(c->instruments_[i]);
			}
			ts_.push_back(c);
			accuracy_ = accuracy_ > c->accuracy_ ? c->accuracy_ : accuracy_;
		};
		void optimize() const { calculate(); };
		//! \name Observer interface
        //@{
		void update();
        //@}
	  private:
		//! \name LazyObject interface
        //@{
        void performCalculations() const;
        //@}
		Real accuracy_;
		bool forcePositive_;
		std::vector<Curve*> ts_;
	};

	template <class Curve>
	void MultiCurveOptimizer<Curve>::update() {
		LazyObject::update();
	}

	template <class Curve>
	void MultiCurveOptimizer<Curve>::performCalculations() const {

        std::vector<Size> initialIndexes;
        std::vector< typename MultiCurvePenaltyFunction<Curve>::helper_iterator > rateHelpersStarts;
        std::vector< typename MultiCurvePenaltyFunction<Curve>::helper_iterator > rateHelpersEnds;

		Size nInsts = 0;
		for(Size i = 0; i < ts_.size(); i++) {
			ts_[i]->bootstrap_.initialize();
			nInsts += ts_[i]->instruments_.size();
			initialIndexes.push_back(1);
			rateHelpersStarts.push_back(ts_[i]->instruments_.begin());
			rateHelpersEnds.push_back(ts_[i]->instruments_.end());
		}

        LevenbergMarquardt solver(accuracy_,
                                  accuracy_,
                                  accuracy_);
		
        EndCriteria endCriteria(20*nInsts, 10, 0.00, accuracy_, 0.00);
        PositiveConstraint posConstraint;
        NoConstraint noConstraint;
        Constraint& solverConstraint = forcePositive_ ?
            static_cast<Constraint&>(posConstraint) :
            static_cast<Constraint&>(noConstraint);

		Array startArray(nInsts);

		Size pos = 0;
		for(Size i = 0; i < ts_.size(); i++) {
			for (Size j = 0; j < ts_[i]->instruments_.size(); j++) {
				startArray[pos] = ts_[i]->data_[j + initialIndexes[i]];
				pos ++;
			}
		}

		MultiCurvePenaltyFunction<Curve> currentCost(
                    ts_,
                    initialIndexes,
                    rateHelpersStarts,
                    rateHelpersEnds);

        Problem toSolve(currentCost, solverConstraint, startArray);

        EndCriteria::Type endType = solver.minimize(toSolve, endCriteria);

        // check the end criteria
        QL_REQUIRE(endType == EndCriteria::StationaryFunctionAccuracy ||
                    endType == EndCriteria::StationaryFunctionValue,
                    "Unable to strip yieldcurve to required accuracy " );

		
	}

    //! multiple bootstrapper for simultanaous bootstrap of multiple curves.
    /*! 
		\TODO: template <class Curve> should be unique for all curves.
    */
    template <class Curve>
    class MultiBootstrap {
        typedef typename Curve::traits_type Traits;
        typedef typename Curve::interpolator_type Interpolator;
      public:

        MultiBootstrap(boost::shared_ptr<MultiCurveOptimizer<Curve> > optimizer = 
										boost::shared_ptr<MultiCurveOptimizer<Curve> >(new MultiCurveOptimizer<Curve>() ));
        void setup(Curve* ts);
        void calculate() const;

      private:
        friend MultiCurveOptimizer<Curve>;
		void initialize() const;
        mutable bool validCurve_;
        Curve* ts_;
		boost::shared_ptr<MultiCurveOptimizer<Curve> > multiCurveOptimizer_;
    };


    // template definitions

    template <class Curve>
    MultiBootstrap<Curve>::MultiBootstrap(boost::shared_ptr<MultiCurveOptimizer<Curve> > optimizer)
    : ts_(0), validCurve_(false), multiCurveOptimizer_(optimizer)
    {}

    template <class Curve>
    void MultiBootstrap<Curve>::setup(Curve* ts) {

        ts_ = ts;

        Size n = ts_->instruments_.size();
        QL_REQUIRE(n >= Interpolator::requiredPoints,
                   "not enough instruments: " << n << " provided, " <<
                   Interpolator::requiredPoints << " required");

        for (Size i=0; i<n; ++i){
            ts_->registerWith(ts_->instruments_[i]);
        }

		multiCurveOptimizer_->addTermStructure(ts_);
    
    }

    template <class Curve>
    void MultiBootstrap<Curve>::initialize() const {
   
        Size nInsts = ts_->instruments_.size();

        // ensure rate helpers are sorted
        std::sort(ts_->instruments_.begin(), ts_->instruments_.end(),
                  detail::BootstrapHelperSorter());

        // check that there is no instruments with the same maturity
        for (Size i=1; i<nInsts; ++i) {
            Date m1 = ts_->instruments_[i-1]->latestDate(),
                 m2 = ts_->instruments_[i]->latestDate();
            QL_REQUIRE(m1 != m2,
                       "two instruments have the same maturity ("<< m1 <<")");
        }

        // check that there is no instruments with invalid quote
        for (Size i=0; i<nInsts; ++i)
            QL_REQUIRE(ts_->instruments_[i]->quote()->isValid(),
                       io::ordinal(i+1) << " instrument (maturity: " <<
                       ts_->instruments_[i]->latestDate() <<
                       ") has an invalid quote");

        // setup instruments
        for (Size i=0; i<nInsts; ++i) {
            // don't try this at home!
            // This call creates instruments, and removes "const".
            // There is a significant interaction with observability.
            ts_->instruments_[i]->setTermStructure(const_cast<Curve*>(ts_));
        }
        // set initial guess only if the current curve cannot be used as guess
        if (validCurve_)
            QL_ENSURE(ts_->data_.size() == nInsts+1,
                      "dimension mismatch: expected " << nInsts+1 <<
                      ", actual " << ts_->data_.size());
        else {
            ts_->data_ = std::vector<Rate>(nInsts+1);
            ts_->data_[0] = Traits::initialValue(ts_);
        }

        // calculate dates and times
        ts_->dates_ = std::vector<Date>(nInsts+1);
        ts_->times_ = std::vector<Time>(nInsts+1);
        ts_->dates_[0] = Traits::initialDate(ts_);
        ts_->times_[0] = ts_->timeFromReference(ts_->dates_[0]);
        for (Size i=0; i<nInsts; ++i) {
            ts_->dates_[i+1] = ts_->instruments_[i]->latestDate();
            ts_->times_[i+1] = ts_->timeFromReference(ts_->dates_[i+1]);
            if (!validCurve_)
                ts_->data_[i+1] = ts_->data_[i];
        }

		ts_->interpolation_ =
            ts_->interpolator_.interpolate(ts_->times_.begin(),
                                            ts_->times_.end(),
                                            ts_->data_.begin());

        
    }

	template <class Curve>
    void MultiBootstrap<Curve>::calculate() const {
		validCurve_ = false;
		multiCurveOptimizer_->optimize();
		validCurve_ = true;
    
	}

    template <class Curve>
    Real MultiCurvePenaltyFunction<Curve>::value(const Array& x) const {
		Array::const_iterator guessIt = x.begin();
		for(Size n = 0; n < curves_.size(); n++) {
			Size nInsts = curves_[n]->instruments_.size();
			Size i = initialIndexes_[n];
			for(Size j = 0; j < nInsts; j++) {
				Traits::updateGuess(curves_[n]->data_, *guessIt, j+i);
				++guessIt;
			}
			curves_[n]->interpolation_.update();
		} 

        Real penalty = 0.0;
		for(Size n = 0; n < curves_.size(); n++) {
			helper_iterator instIt = rateHelpersStarts_[n];
			while (instIt != rateHelpersEnds_[n]) {
				Real quoteError = (*instIt)->quoteError();
				penalty += std::fabs(quoteError);
				++instIt;
			}
		}
        return penalty;
    }

    template <class Curve>
    Disposable<Array> MultiCurvePenaltyFunction<Curve>::values(const Array& x) const {
		Array::const_iterator guessIt = x.begin();
		for(Size n = 0; n < curves_.size(); n++) {
			Size nInsts = curves_[n]->instruments_.size();
			Size i = initialIndexes_[n];
			for(Size j = 0; j < nInsts; j++) {
				Traits::updateGuess(curves_[n]->data_, *guessIt, j + i);
				++guessIt;
			}
			curves_[n]->interpolation_.update();
		}

        Array penalties(x.size());
		Array::iterator penIt = penalties.begin();
		for(Size n = 0; n < curves_.size(); n++) {
			helper_iterator instIt = rateHelpersStarts_[n];
			while (instIt != rateHelpersEnds_[n]) {
				Real quoteError = (*instIt)->quoteError();
				*penIt = std::fabs(quoteError);
				++instIt;
				++penIt;
			}
		}
        return penalties;
    }

  

}

#endif
