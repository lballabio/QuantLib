/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*


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

#include <ql/MarketModels/driftcalculator.hpp>
#include <vector>

using namespace QuantLib;


DriftCalculator::DriftCalculator(
								 const Matrix& pseudo,
								 const Array& displacements,
								 const Array& taus,
								 Size numeraire,
								 Size alive) : 
size_(taus.size()),
pseudo_(pseudo), 
displacements_(displacements), 
taus_(taus), 
numeraire_(numeraire), 
alive_(alive) {

	QL_REQUIRE(size_>0, "");
	QL_REQUIRE(displacements.size() == size_, "");
	QL_REQUIRE(pseudo.rows()==size_, "");
	QL_REQUIRE(alive>=0 && alive<size_, "");
	const Disposable<Matrix> pT = transpose(pseudo_);
	C_ = pseudo_*pT;
}


void DriftCalculator::compute(const Array& forwards, Array& drifts) const {
	
	QL_REQUIRE(forwards.size() == size_, "");
	QL_REQUIRE(drifts.size() == size_, "");

	for(Size i=alive_; i<size_; ++i) {
		drifts[i]=0;
		for(Size k=i+1; k<size_; ++k) {
			const double A = (taus_[k]*(forwards[k]+displacements_[k]) / (1.+taus_[k]*(forwards[k])));
			drifts[i] += -A*C_[k][i];
		}
	}
}