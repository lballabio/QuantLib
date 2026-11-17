/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/handle.hpp>
#include <ql/quotes/simplequote.hpp>
#include <utility>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    void checkEmptyHandle(const Handle<Quote>& handle, bool hasLink) {
        BOOST_CHECK(handle.empty());
        Handle<Quote> copy = handle;
        BOOST_CHECK(copy.empty());
        BOOST_CHECK(handle == copy);
        BOOST_CHECK(!(handle != copy));
        BOOST_CHECK(!(handle < copy));
        BOOST_CHECK(!(copy < handle));

        Handle<Quote> defaultHandle;
        BOOST_CHECK_EQUAL(handle == defaultHandle, !hasLink);
        BOOST_CHECK_EQUAL(handle != defaultHandle, hasLink);
        BOOST_CHECK_EQUAL((handle < defaultHandle) != (defaultHandle < handle), hasLink);
        Handle<Quote> other(nullptr);
        BOOST_CHECK(!(handle == other));
        BOOST_CHECK(handle != other);
        BOOST_CHECK((handle < other) != (other < handle));

        const ExpectedErrorMessage emptyHandleError("empty Handle cannot be dereferenced");
        BOOST_CHECK_EXCEPTION(handle.currentLink(), Error, emptyHandleError);
        BOOST_CHECK_EXCEPTION(handle.operator->(), Error, emptyHandleError);
        BOOST_CHECK_EXCEPTION(*handle, Error, emptyHandleError);
        BOOST_CHECK_EQUAL(bool(ext::shared_ptr<Observable>(handle)), hasLink);
        UpdateCounter observer;
        BOOST_CHECK_EQUAL(observer.registerWith(handle).second, hasLink);
        BOOST_CHECK_EQUAL(observer.unregisterWith(handle), hasLink ? 1 : 0);
    }

}

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(HandleTests)

BOOST_AUTO_TEST_CASE(testEmptyHandles) {
    BOOST_TEST_MESSAGE("Testing empty handle construction and registration...");

    const ext::shared_ptr<Quote> nullQuote;
    checkEmptyHandle(Handle<Quote>(), false);
    checkEmptyHandle(Handle<Quote>(nullQuote), true);
    checkEmptyHandle(Handle<Quote>(nullptr), true);
}

BOOST_AUTO_TEST_CASE(testEmptyRelinkableHandles) {
    BOOST_TEST_MESSAGE("Testing initially empty relinkable handles...");

    const ext::shared_ptr<Quote> nullQuote;
    for (auto handle : {RelinkableHandle<Quote>(), RelinkableHandle<Quote>(nullQuote),
                        RelinkableHandle<Quote>(nullptr)}) {
        checkEmptyHandle(handle, true);
        Handle<Quote> copy = handle;
        auto quote = ext::make_shared<SimpleQuote>(1.0);
        handle.linkTo(quote);
        BOOST_CHECK(handle.currentLink() == quote);
        BOOST_CHECK(copy.currentLink() == quote);
    }
}

BOOST_AUTO_TEST_CASE(testMovedOutHandles) {
    BOOST_TEST_MESSAGE("Testing moved-out handle access and relinking checks...");

    auto quote = ext::make_shared<SimpleQuote>(1.0);
    Handle<Quote> source(quote);
    UpdateCounter observer;
    observer.registerWith(source);
    Handle<Quote> destination(std::move(source));
    checkEmptyHandle(source, false);
    BOOST_CHECK(destination.currentLink() == quote);
    quote->setValue(2.0);
    BOOST_CHECK_EQUAL(observer.counter(), 1);

    RelinkableHandle<Quote> relinkable(quote);
    Handle<Quote> copy = relinkable;
    observer.registerWith(copy);
    RelinkableHandle<Quote> moved(std::move(relinkable));
    checkEmptyHandle(relinkable, false);
    const ext::shared_ptr<Quote> replacement = ext::make_shared<SimpleQuote>(3.0);
    const ExpectedErrorMessage movedOutError("attempting to link a moved-out Handle");
    BOOST_CHECK_EXCEPTION(relinkable.linkTo(replacement), Error, movedOutError);
    BOOST_CHECK_EXCEPTION(relinkable.linkTo(ext::shared_ptr<Quote>(replacement)), Error,
                          movedOutError);
    BOOST_CHECK_EXCEPTION(relinkable.reset(), Error, movedOutError);
    moved.linkTo(replacement);
    BOOST_CHECK(copy.currentLink() == replacement);
    BOOST_CHECK_EQUAL(observer.counter(), 2);
    moved.reset();
    checkEmptyHandle(copy, true);
    BOOST_CHECK_EQUAL(observer.counter(), 3);
}

BOOST_AUTO_TEST_CASE(testRegisterAsObserver) {
    BOOST_TEST_MESSAGE("Testing handles with and without observation...");

    for (bool registerAsObserver : {false, true}) {
        auto simpleQuote = ext::make_shared<SimpleQuote>(1.0);
        ext::shared_ptr<Quote> quote = simpleQuote;
        RelinkableHandle<Quote> fromLvalue(quote, registerAsObserver);
        RelinkableHandle<Quote> fromRvalue(ext::shared_ptr<Quote>(quote), registerAsObserver);
        UpdateCounter lvalueObserver, rvalueObserver;
        lvalueObserver.registerWith(fromLvalue);
        rvalueObserver.registerWith(fromRvalue);
        simpleQuote->setValue(2.0);
        const Size initialUpdates = registerAsObserver ? 1 : 0;
        BOOST_CHECK_EQUAL(lvalueObserver.counter(), initialUpdates);
        BOOST_CHECK_EQUAL(rvalueObserver.counter(), initialUpdates);

        // Changing observation mode on the same quote must notify handle observers.
        fromLvalue.linkTo(quote, !registerAsObserver);
        fromRvalue.linkTo(ext::shared_ptr<Quote>(quote), !registerAsObserver);
        BOOST_CHECK_EQUAL(lvalueObserver.counter(), initialUpdates + 1);
        BOOST_CHECK_EQUAL(rvalueObserver.counter(), initialUpdates + 1);

        simpleQuote->setValue(3.0);
        BOOST_CHECK_EQUAL(lvalueObserver.counter(), 2);
        BOOST_CHECK_EQUAL(rvalueObserver.counter(), 2);
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
