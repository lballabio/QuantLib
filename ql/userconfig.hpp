/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2011 Ferdinando Ametrano
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

#ifndef quantlib_config_hpp
#define quantlib_config_hpp

/***************************************************************
   User configuration section:
   modify the following definitions to suit your preferences.

   Do not modify this file if you are using a Linux/Unix system:
   it will not be read by the compiler. The definitions below
   will be provided by running ./configure instead.
****************************************************************/

/* If defined, function information is added to the error messages
   thrown by the library.
*/
#ifndef QL_ERROR_FUNCTIONS
//#   define QL_ERROR_FUNCTIONS
#endif

/* If defined, file and line information is added to the error
   messages thrown by the library.
*/
#ifndef QL_ERROR_LINES
//#   define QL_ERROR_LINES
#endif

/* If defined, tracing messages might be emitted by the library
   depending on run-time settings. Enabling this option can degrade
   performance.
*/
#ifndef QL_ENABLE_TRACING
//#   define QL_ENABLE_TRACING
#endif

/* If defined, extra run-time checks are added to a few
   functions. This can prevent their inlining and degrade
   performance.
*/
#ifndef QL_EXTRA_SAFETY_CHECKS
//#   define QL_EXTRA_SAFETY_CHECKS
#endif

/* If defined, indexed coupons (see the documentation) are used in
   floating legs.  If undefined, par coupons are used.
*/
#ifndef QL_USE_INDEXED_COUPON
//#   define QL_USE_INDEXED_COUPON
#endif

/* If defined, singletons will return different instances for
   different threads; in particular, this means that the evaluation
   date, the stored index fixings and any other settings will be
   per-thread.
*/
#ifndef QL_ENABLE_SESSIONS
//#   define QL_ENABLE_SESSIONS
#endif

/* If defined, a thread-safe (but less performant) version of the
   observer pattern is used. You should define it if you want to use
   QuantLib via the SWIG layer within the JVM or .NET ecosystem or
   any other environment with an async garbage collector.
*/
#ifndef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
//#    define QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
#endif

/* If defined, date objects willsupport an intraday datetime
   resolution down to microseconds.  Strictly monotone daycounters
   (`Actual360`, `Actual365Fixed` and `ActualActual`) will take the
   additional information into account and allow for accurate intraday
   pricing.  If undefined, the smallest resolution of date objects is
   a single day.  Intraday datetime resolution is experimental.
*/
#ifndef QL_HIGH_RESOLUTION_DATE
//#    define QL_HIGH_RESOLUTION_DATE
#endif

/* If defined, lazy objects will raise an exception when they detect a
   notification cycle which would result in an infinite recursion
   loop. If undefined, they will break the recursion without throwing.
   Enabling this option is recommended but might cause existing code
   to throw.
*/
#ifndef QL_THROW_IN_CYCLES
//#    define QL_THROW_IN_CYCLES
#endif

/* If defined, lazy objects will forward the first notification
   received, and discard the others until recalculated; the rationale
   is that observers were already notified, and don't need further
   notifications until they recalculate, at which point this object
   would be recalculated too.  After recalculation, this object would
   again forward the first notification received.  Although not always
   correct, this behavior is a lot faster and thus is the current
   default.
*/
#ifndef QL_FASTER_LAZY_OBJECTS
#    define QL_FASTER_LAZY_OBJECTS
#endif

/* If defined, `std::any` and related classes and functions will be
   used instead of `boost::any`. If undefined, the Boost facilities
   will be used.
*/
#ifndef QL_USE_STD_ANY
#    define QL_USE_STD_ANY
#endif

/* If defined, `std::optional` and related classes and functions will
   be used instead of `boost::optional`. If undefined, the Boost
   facilities will be used.
*/
#ifndef QL_USE_STD_OPTIONAL
#    define QL_USE_STD_OPTIONAL
#endif

/* If defined, `std::shared_ptr` and related classes and functions
   will used instead of `boost::shared_ptr`. If undefined, the Boost
   facilities will be used. Note that `std::shared_ptr` does not check
   access and can cause segmentation faults.
*/
#ifndef QL_USE_STD_SHARED_PTR
//#    define QL_USE_STD_SHARED_PTR
#endif

/* If defined, `Null` will be implemented as a template function.
   This allows the code to work with user-defined `Real` types but was
   reported to cause internal compiler errors with Visual C++ 2022 in
   some cases.  If undefined, `Null` will be implemented as a class
   template, as in previous releases.
*/
#ifndef QL_NULL_AS_FUNCTIONS
//#    define QL_NULL_AS_FUNCTIONS
#endif

/* If defined, a parallel unit test runner will be used to execute the
   C++ test suite. This will reduce the runtime on multi core CPUs.
*/
#ifndef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
//#    define QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
#endif

#endif
