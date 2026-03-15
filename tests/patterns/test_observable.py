"""Tests for Observable, Observer, and ObservableSettings.

Transpiled from upstream/test-suite/observable.cpp with simplified helpers.
Thread-safe tests (testAsyncGarbagCollector, testMultiThreadingGlobalSettings)
and tests requiring unbuilt modules (testDeepUpdate with FlatForward/Euribor,
testDeferredObserverLifetime with ZeroCouponInflationSwapHelper) are excluded.
"""

import gc
import random
import weakref

from quantlib.patterns.observable import Observable, ObservableSettings, Observer

# --- Test helpers (replacing C++ SimpleQuote, Flag, etc.) ---


class SimpleQuote(Observable):
    """Minimal test double for SimpleQuote — an Observable that holds a value."""

    def __init__(self, value=0.0):
        super().__init__()
        self._value = value

    def value(self):
        return self._value

    def setValue(self, value):
        if value != self._value:
            self._value = value
            self.notifyObservers()


class UpdateCounter(Observer):
    """Observer that counts how many times update() was called."""

    def __init__(self):
        super().__init__()
        self._counter = 0

    def update(self):
        self._counter += 1

    def counter(self):
        return self._counter


class Flag(Observer):
    """Observer that raises/lowers a boolean flag on update."""

    def __init__(self):
        super().__init__()
        self._up = False

    def update(self):
        self._up = True

    def isUp(self):
        return self._up

    def lower(self):
        self._up = False


class DummyObserver(Observer):
    """Observer with a no-op update."""

    def __init__(self):
        super().__init__()

    def update(self):
        pass


class TestObservableSettings:
    """Transpiled from testObservableSettings in observable.cpp."""

    def setup_method(self):
        """Ensure updates are enabled before each test."""
        ObservableSettings.instance().enableUpdates()

    def teardown_method(self):
        """Restore updates after each test."""
        ObservableSettings.instance().enableUpdates()

    def test_basic_notification(self):
        quote = SimpleQuote(100.0)
        counter = UpdateCounter()
        counter.registerWith(quote)

        assert counter.counter() == 0
        quote.setValue(1.0)
        assert counter.counter() == 1

    def test_disable_updates_no_defer(self):
        quote = SimpleQuote(100.0)
        counter = UpdateCounter()
        counter.registerWith(quote)

        quote.setValue(1.0)
        assert counter.counter() == 1

        ObservableSettings.instance().disableUpdates(False)
        quote.setValue(2.0)
        assert counter.counter() == 1  # Blocked

        ObservableSettings.instance().enableUpdates()
        assert counter.counter() == 1  # Not deferred, so still 1

    def test_disable_updates_with_defer(self):
        quote = SimpleQuote(100.0)
        counter = UpdateCounter()
        counter.registerWith(quote)

        quote.setValue(1.0)
        assert counter.counter() == 1

        ObservableSettings.instance().disableUpdates(True)
        quote.setValue(3.0)
        assert counter.counter() == 1  # Blocked

        ObservableSettings.instance().enableUpdates()
        assert counter.counter() == 2  # Deferred update fired

    def test_multiple_deferred_updates_coalesce(self):
        """Multiple changes while deferred result in a single update per observer."""
        quote = SimpleQuote(100.0)
        counter = UpdateCounter()
        counter2 = UpdateCounter()
        counter.registerWith(quote)
        counter2.registerWith(quote)

        ObservableSettings.instance().disableUpdates(True)
        for i in range(10):
            quote.setValue(float(i))
        assert counter.counter() == 0

        ObservableSettings.instance().enableUpdates()
        assert counter.counter() == 1
        assert counter2.counter() == 1


class TestEmptyObserver:
    """Transpiled from testEmptyObserverList in observable.cpp."""

    def test_unregister_with_no_prior_registration(self):
        """unregisterWith on an observer that never registered should not raise."""
        observer = DummyObserver()
        quote = SimpleQuote(10.0)
        observer.unregisterWith(quote)


class TestObserverDuringNotification:
    """Transpiled from testAddAndDeleteObserverDuringNotifyObservers in observable.cpp.

    Simplified: uses Python random instead of MersenneTwisterUniformRng.
    """

    def test_add_and_delete_during_notification(self):
        """Adding and deleting observers during notifyObservers should not crash."""
        rng = random.Random(42)

        nr_initial = 20
        nr_delete = 5
        nr_additional = 100
        test_runs = 100

        for _ in range(test_runs):
            observable = Observable()
            expected = []
            additional = []

            class TestObserver(Observer):
                def __init__(self, setup=None):
                    super().__init__()
                    self._setup = setup
                    self._updates = 0

                def update(self):
                    self._updates += 1
                    if self._setup is not None:
                        obs_ref, expected_ref, additional_ref = self._setup
                        for _ in range(nr_additional):
                            new_obs = TestObserver()
                            new_obs.registerWith(obs_ref)
                            additional_ref.append(new_obs)
                        for _ in range(min(nr_delete, len(expected_ref))):
                            j = rng.randint(0, len(expected_ref) - 1)
                            if expected_ref[j] is not self:
                                expected_ref.pop(j)

                def getUpdates(self):
                    return self._updates

            setup = (observable, expected, additional)

            for i in range(nr_initial):
                if i == nr_initial // 3 or i == nr_initial // 2:
                    obs = TestObserver(setup=setup)
                else:
                    obs = TestObserver()
                obs.registerWith(observable)
                expected.append(obs)

            observable.notifyObservers()

            for obs in expected:
                assert obs.getUpdates() > 0, "missed observer update detected"


class TestGarbageCollection:
    """Test that weak references allow GC of observers."""

    def test_observer_gc(self):
        """Observers should be garbage-collected when no strong refs remain."""
        quote = SimpleQuote(1.0)

        observer = UpdateCounter()
        observer.registerWith(quote)
        weak = weakref.ref(observer)

        quote.setValue(2.0)
        assert observer.counter() == 1

        del observer
        gc.collect()

        assert weak() is None

        # Should not raise even though observer is gone
        quote.setValue(3.0)

    def test_register_with_none(self):
        """registerWith(None) should be a no-op."""
        observer = DummyObserver()
        observer.registerWith(None)  # Should not raise


class TestDeepUpdate:
    """Simplified deep update test (no term structure dependencies).

    Tests the deepUpdate mechanism using a chain of observers.
    The full C++ testDeepUpdate depends on FlatForward/Euribor — deferred.
    """

    def test_deep_update_calls_update(self):
        """Default deepUpdate calls update()."""
        quote = SimpleQuote(1.0)
        flag = Flag()
        flag.registerWith(quote)

        quote.setValue(2.0)
        assert flag.isUp()

        flag.lower()
        flag.deepUpdate()
        assert flag.isUp()  # deepUpdate calls update()

    def test_deep_update_propagation(self):
        """deepUpdate can propagate through observer chains."""
        quote = SimpleQuote(1.0)
        flag = Flag()

        class Intermediary(Observable, Observer):
            def __init__(self):
                Observable.__init__(self)
                Observer.__init__(self)

            def update(self):
                self.notifyObservers()

            def deepUpdate(self):
                for obs in list(self._observables):
                    if isinstance(obs, Observer):
                        obs.deepUpdate()
                self.update()

        mid = Intermediary()
        mid.registerWith(quote)
        flag.registerWith(mid)

        # Normal notification works
        quote.setValue(2.0)
        assert flag.isUp()

        # deepUpdate on mid propagates to flag
        flag.lower()
        mid.deepUpdate()
        assert flag.isUp()


class TestUnregisterWithAll:
    def test_unregister_with_all(self):
        """unregisterWithAll removes observer from all observables."""
        q1 = SimpleQuote(1.0)
        q2 = SimpleQuote(2.0)
        counter = UpdateCounter()
        counter.registerWith(q1)
        counter.registerWith(q2)

        q1.setValue(10.0)
        assert counter.counter() == 1

        counter.unregisterWithAll()
        q1.setValue(20.0)
        q2.setValue(30.0)
        assert counter.counter() == 1  # No more updates


class TestRegisterWithObservables:
    def test_register_with_observables(self):
        """registerWithObservables copies another observer's registrations."""
        q1 = SimpleQuote(1.0)
        q2 = SimpleQuote(2.0)

        observer1 = UpdateCounter()
        observer1.registerWith(q1)
        observer1.registerWith(q2)

        observer2 = UpdateCounter()
        observer2.registerWithObservables(observer1)

        q1.setValue(10.0)
        assert observer2.counter() == 1

        q2.setValue(20.0)
        assert observer2.counter() == 2
