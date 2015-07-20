/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file mersennetwister_multithreaded.hpp
    \brief multi threaded mersenne twister (for 8 threads)
*/

#ifndef quantlib_mersennetwister_multithreaded_hpp
#define quantlib_mersennetwister_multithreaded_hpp

#include <ql/experimental/math/dynamiccreator.hpp>


namespace QuantLib {

class MersenneTwisterMultiThreaded {
  public:
    typedef Sample<Real> sample_type;
    static const Size maxNumberOfThreads = 8;

    // if given seed is 0 then a clock based seed is used
    MersenneTwisterMultiThreaded(const unsigned long seed = 0);

    sample_type next(unsigned int threadId) const;
    Real nextReal(unsigned int threadId) const;
    unsigned long operator()(unsigned int threadId) const;
    unsigned long nextInt32(unsigned int threadId) const;

  private:
    mutable boost::shared_ptr<MersenneTwisterCustomRng<Mtdesc19937_0> > m0_;
    mutable boost::shared_ptr<MersenneTwisterCustomRng<Mtdesc19937_1> > m1_;
    mutable boost::shared_ptr<MersenneTwisterCustomRng<Mtdesc19937_2> > m2_;
    mutable boost::shared_ptr<MersenneTwisterCustomRng<Mtdesc19937_3> > m3_;
    mutable boost::shared_ptr<MersenneTwisterCustomRng<Mtdesc19937_4> > m4_;
    mutable boost::shared_ptr<MersenneTwisterCustomRng<Mtdesc19937_5> > m5_;
    mutable boost::shared_ptr<MersenneTwisterCustomRng<Mtdesc19937_6> > m6_;
    mutable boost::shared_ptr<MersenneTwisterCustomRng<Mtdesc19937_7> > m7_;
};


} // namespace QuantLib

#endif
