/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Johannes Goettker-Schnetmann
 Copyright (C) 2015 Klaus Spanderen

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

/*! \file localvolendcalculator.cpp
    \brief local volatility risk neutral terminal density calculation
*/

#include <ql/quote.hpp>
#include <ql/timegrid.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/schemes/douglasscheme.hpp>
#include <ql/experimental/finitedifferences/fdmlocalvolfwdop.hpp>
#include <ql/experimental/finitedifferences/localvolrndcalculator.hpp>

#include <boost/make_shared.hpp>
#include <iostream>

namespace QuantLib {
	LocalVolRNDCalculator::LocalVolRNDCalculator(
		const boost::shared_ptr<Quote>& spot,
		const boost::shared_ptr<YieldTermStructure>& rTS,
		const boost::shared_ptr<YieldTermStructure>& qTS,
		const boost::shared_ptr<LocalVolTermStructure>& localVol,
		Size xGrid, Size tGrid, Real eps)
	: xGrid_(xGrid),
	  tGrid_(tGrid),
	  eps_     (eps),
	  spot_	   (spot),
	  localVol_(localVol),
	  rTS_     (rTS),
	  qTS_     (qTS),
	  timeGrid_(new TimeGrid(localVol->maxTime(), tGrid)),
	  xm_      (new Matrix(tGrid, xGrid)),
	  pm_      (new Matrix(tGrid, xGrid)) {
		registerWith(spot_);
		registerWith(rTS_);
		registerWith(qTS_);
		registerWith(localVol_);
	}

	LocalVolRNDCalculator::LocalVolRNDCalculator(
		const boost::shared_ptr<Quote>& spot,
		const boost::shared_ptr<YieldTermStructure>& rTS,
		const boost::shared_ptr<YieldTermStructure>& qTS,
		const boost::shared_ptr<LocalVolTermStructure>& localVol,
		const boost::shared_ptr<TimeGrid>& timeGrid,
		Size xGrid, Real eps)
	: xGrid_(xGrid),
	  tGrid_(timeGrid->size()-1),
	  eps_     (eps),
	  spot_    (spot),
	  localVol_(localVol),
	  rTS_     (rTS),
	  qTS_     (qTS),
	  timeGrid_(timeGrid),
	  xm_      (new Matrix(tGrid_, xGrid_)),
	  pm_      (new Matrix(tGrid_, xGrid_)) {
		registerWith(spot_);
		registerWith(rTS_);
		registerWith(qTS_);
		registerWith(localVol_);
	}

	Real LocalVolRNDCalculator::pdf(Real x, Time t) const {
		calculate();
		return 1.0;
	}

	Real LocalVolRNDCalculator::cdf(Real x, Time t) const {
		calculate();
		return 1.0;
	}

	Real LocalVolRNDCalculator::invcdf(Real p, Time t) const {
		calculate();
		return 1.0;
	}

	Disposable<Array> LocalVolRNDCalculator::x(Time t) const {
        calculate();

		const Size idx = timeGrid_->index(t);
		QL_REQUIRE(idx <= xm_->rows(), "inconsistent time " << t << " given");

		if (idx > 0) {
			Array retVal(xm_->row_begin(idx-1), xm_->row_end(idx-1));
			return retVal;
		}
		else {
			Array retVal(xm_->columns(), std::log(spot_->value()));
			return retVal;
		}
	}

	boost::shared_ptr<TimeGrid> LocalVolRNDCalculator::timeGrid() const {
		return timeGrid_;
	}

	void LocalVolRNDCalculator::performCalculations() const {
		const Time sT = timeGrid_->at(1);
		Time t = std::min(sT, 1.0/365);

		const Volatility vol = localVol_->localVol(0.5*t, spot_->value());

		const Volatility stdDev = vol * std::sqrt(t);
		Real xm = - 0.5 * stdDev * stdDev +
			std::log(spot_->value() * qTS_->discount(t)/rTS_->discount(t));

		const Real normInvEps = InverseCumulativeNormal()(1 - eps_);
		Real sLowerBound = xm - normInvEps * stdDev;
		Real sUpperBound = xm + normInvEps * stdDev;

		boost::shared_ptr<Fdm1dMesher> mesher(
			new Concentrating1dMesher(sLowerBound, sUpperBound, xGrid_,
				std::make_pair(xm, 0.05), true));

	    Array p(mesher->size());
	    Array x(mesher->locations().begin(), mesher->locations().end());

	    GaussianDistribution gaussianPDF(xm, vol * std::sqrt(t));

	    for (Size idx=0; idx < p.size(); ++idx) {
	        p[idx] = gaussianPDF(x[idx]);
	    }
	    p = rescalePDF(x, p);

	    QL_REQUIRE(x.size() > 10, "x grid is too small. "
	    						  "Minimum size is greater than 10");

	    const Size b = std::max(Size(5), Size(x.size()*0.05));

	    boost::shared_ptr<DouglasScheme> evolver(
	    	new DouglasScheme(0.5,
				boost::make_shared<FdmLocalVolFwdOp>(
					boost::make_shared<FdmMesherComposite>(mesher),
					spot_, rTS_, qTS_, localVol_)));

	    pFct_.resize(tGrid_);

	    for (Size i=1; i <= tGrid_; ++i) {
	    	const Time dt = timeGrid_->at(i) - t;
	    	if (dt < QL_EPSILON)
	    		continue; // too small step

	    	// leaking probability mass?
	    	const Real maxLeftValue =
	    		std::max(std::fabs(*std::min_element(p.begin(), p.begin()+b)),
	    				 std::fabs(*std::max_element(p.begin(), p.begin()+b)));
	    	const Real maxRightValue =
	    		std::max(std::fabs(*std::min_element(p.end()-b, p.end())),
	    				 std::fabs(*std::max_element(p.end()-b, p.end())));

	    	if (std::max(maxLeftValue, maxRightValue) > eps_) {
	    		const Real oldLowerBound = sLowerBound;
	    	    const Real oldUpperBound = sUpperBound;

	    		xm = DiscreteSimpsonIntegral()(x, x*p);

	    		if (maxLeftValue > eps_)
	    			sLowerBound -= 0.1*xm;
	    		if (maxRightValue > eps_)
	    			sUpperBound += 0.1*xm;

	    		std::cout << "regrid " << maxLeftValue << " "
	    				  << sLowerBound << " " << sUpperBound
	    				  << " " << std::exp(xm) << " " << t << std::endl;

	    		mesher = boost::shared_ptr<Fdm1dMesher>(
	    			new Concentrating1dMesher(sLowerBound, sUpperBound, xGrid_,
	    				std::make_pair(xm, 0.05), true));

	    	    const CubicNaturalSpline pSpline(x.begin(), x.end(), p.begin());
	    	    const Array xn(mesher->locations().begin(),
	    	    			   mesher->locations().end());
	    	    Array pn(xn.size(), 0.0);

	    	    for (Size j=0; j < xn.size(); ++j) {
	    	    	if (xn[j] >= oldLowerBound && xn[j] <= oldUpperBound)
	    	    		pn[j] = pSpline(xn[j]);
	    	    }

		    	x = xn;
		    	p = rescalePDF(xn, pn);

			    evolver = boost::make_shared<DouglasScheme>(0.5,
			    	boost::make_shared<FdmLocalVolFwdOp>(
						boost::make_shared<FdmMesherComposite>(mesher),
						spot_, rTS_, qTS_, localVol_));
	    	}
	    	rescalePDF(Array(x), Array(p));

	        evolver->setStep(dt);
			evolver->step(p, t + dt);
			t+=dt;

			std::copy(x.begin(), x.end(), xm_->row_begin(i-1));
			std::copy(p.begin(), p.end(), pm_->row_begin(i-1));
			pFct_[i-1] = boost::make_shared<CubicNaturalSpline>(
				xm_->row_begin(i-1), xm_->row_end(i-1), pm_->row_begin(i-1));
	    }
	}

	Disposable<Array> LocalVolRNDCalculator::rescalePDF(
		const Array& x, const Array& p) {

	    const CubicNaturalSpline pSpline(x.begin(), x.end(), p.begin());

		const Real scale = DiscreteSimpsonIntegral()(x, p);

		//const Real scale = GaussLobattoIntegral(10000, 100*QL_EPSILON)(pSpline, x.front(), x.back());

		std::cout << "rescale factor is " << scale << std::endl;

		Array retVal = p*(1/scale);
		return retVal;
	}
}

