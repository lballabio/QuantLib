"""Tests for LazyObject with calculation caching.

Transpiled from upstream/test-suite/lazyobject.cpp.
Uses test doubles instead of Stock/Handle/Instrument.
"""

from quantlib.patterns.lazyobject import LazyObject
from quantlib.patterns.observable import Observable, Observer

# --- Test helpers (replacing C++ SimpleQuote, Stock, Flag, Handle) ---


class SimpleQuote(Observable):
    """Test double for SimpleQuote."""

    def __init__(self, value=0.0):
        super().__init__()
        self._value = value

    def value(self):
        return self._value

    def setValue(self, value):
        if value != self._value:
            self._value = value
            self.notifyObservers()


class Stock(LazyObject):
    """Test double for Stock — a LazyObject that depends on a quote.

    Mirrors upstream/ql/instruments/stock.hpp: NPV = quote.value().
    """

    def __init__(self, quote):
        super().__init__()
        self._quote = quote
        if quote is not None:
            self.registerWith(quote)

    def NPV(self):
        self.calculate()
        return self._npv

    def performCalculations(self):
        self._npv = self._quote.value() if self._quote else 0.0

    def isExpired(self):
        return False


class Flag(Observer):
    """Test double for Flag — observer that tracks if update was called."""

    def __init__(self):
        super().__init__()
        self._up = False

    def update(self):
        self._up = True

    def isUp(self):
        return self._up

    def lower(self):
        self._up = False


class TearDown:
    """Save/restore LazyObject.Defaults state."""

    def __init__(self):
        self._was_forwarding = LazyObject.Defaults.instance().forwardsAllNotifications()

    def restore(self):
        if self._was_forwarding:
            LazyObject.Defaults.instance().alwaysForwardNotifications()
        else:
            LazyObject.Defaults.instance().forwardFirstNotificationOnly()


class TestDiscardNotifications:
    """Transpiled from testDiscardingNotifications / testDiscardingNotificationsByDefault."""

    def setup_method(self):
        self._teardown = TearDown()

    def teardown_method(self):
        self._teardown.restore()

    def test_discard_against_default(self):
        """LazyObject can discard notifications after the first (against default)."""
        LazyObject.Defaults.instance().alwaysForwardNotifications()

        q = SimpleQuote(0.0)
        s = Stock(q)
        f = Flag()
        f.registerWith(s)

        s.forwardFirstNotificationOnly()

        s.NPV()
        q.setValue(1.0)
        assert f.isUp(), "Observer was not notified of change"

        f.lower()
        q.setValue(2.0)
        assert not f.isUp(), "Observer was notified of second change"

        f.lower()
        s.NPV()
        q.setValue(3.0)
        assert f.isUp(), "Observer was not notified of change after recalculation"

    def test_discard_by_default(self):
        """LazyObject discards notifications after the first by default setting."""
        LazyObject.Defaults.instance().forwardFirstNotificationOnly()

        q = SimpleQuote(0.0)
        s = Stock(q)
        f = Flag()
        f.registerWith(s)

        s.NPV()
        q.setValue(1.0)
        assert f.isUp(), "Observer was not notified of change"

        f.lower()
        q.setValue(2.0)
        assert not f.isUp(), "Observer was notified of second change"

        f.lower()
        s.NPV()
        q.setValue(3.0)
        assert f.isUp(), "Observer was not notified of change after recalculation"


class TestForwardNotifications:
    """Transpiled from testForwardingNotifications / testForwardingNotificationsByDefault."""

    def setup_method(self):
        self._teardown = TearDown()

    def teardown_method(self):
        self._teardown.restore()

    def test_forward_by_default(self):
        """LazyObject forwards all notifications by default."""
        LazyObject.Defaults.instance().alwaysForwardNotifications()

        q = SimpleQuote(0.0)
        s = Stock(q)
        f = Flag()
        f.registerWith(s)

        s.NPV()
        q.setValue(1.0)
        assert f.isUp(), "Observer was not notified of change"

        f.lower()
        q.setValue(2.0)
        assert f.isUp(), "Observer was not notified of second change"

    def test_forward_against_default(self):
        """LazyObject can forward all notifications (against default)."""
        LazyObject.Defaults.instance().forwardFirstNotificationOnly()

        q = SimpleQuote(0.0)
        s = Stock(q)
        f = Flag()
        f.registerWith(s)

        s.alwaysForwardNotifications()

        s.NPV()
        q.setValue(1.0)
        assert f.isUp(), "Observer was not notified of change"

        f.lower()
        q.setValue(2.0)
        assert f.isUp(), "Observer was not notified of second change"


class TestNotificationLoop:
    """Transpiled from testNotificationLoop — tests recursion guard."""

    def setup_method(self):
        self._teardown = TearDown()

    def teardown_method(self):
        self._teardown.restore()

    def test_recursive_notification(self):
        """Recursive notification loops are handled gracefully."""
        LazyObject.Defaults.instance().alwaysForwardNotifications()

        q = SimpleQuote(0.0)
        s1 = Stock(q)
        s2 = Stock(None)
        s3 = Stock(None)

        # Create a cycle: s3 -> s2 -> s1 -> s3
        s3.registerWith(s2)
        s2.registerWith(s1)
        s1.registerWith(s3)

        f = Flag()
        f.registerWith(s3)
        q.setValue(2.0)

        assert f.isUp(), "Observer was not notified of change"

        # Break the cycle to allow GC
        s1.unregisterWithAll()
        s2.unregisterWithAll()
        s3.unregisterWithAll()


class TestLazyObjectBasics:
    """Additional tests for LazyObject methods."""

    def test_freeze_unfreeze(self):
        """Freeze prevents recalculation, unfreeze re-enables it."""
        q = SimpleQuote(10.0)
        s = Stock(q)

        assert s.NPV() == 10.0

        s.freeze()
        q.setValue(20.0)
        assert s.NPV() == 10.0  # Still cached

        s.unfreeze()
        assert s.NPV() == 20.0  # Recalculated

    def test_recalculate(self):
        """recalculate() forces recalculation even when frozen."""
        q = SimpleQuote(10.0)
        s = Stock(q)
        assert s.NPV() == 10.0  # Initial calculation
        s.freeze()

        q.setValue(20.0)
        assert s.NPV() == 10.0  # Frozen

        s.recalculate()
        assert s.NPV() == 20.0  # Force recalculated
        assert s.isCalculated()

    def test_is_calculated(self):
        """isCalculated tracks calculation state."""
        q = SimpleQuote(10.0)
        s = Stock(q)

        assert not s.isCalculated()
        s.NPV()
        assert s.isCalculated()
        q.setValue(20.0)
        assert not s.isCalculated()

    def test_set_calculated(self):
        """setCalculated manually sets the flag."""
        q = SimpleQuote(10.0)
        s = Stock(q)
        s.setCalculated(True)
        assert s.isCalculated()

    def test_defaults_singleton(self):
        """LazyObject.Defaults is a proper singleton."""
        d1 = LazyObject.Defaults.instance()
        d2 = LazyObject.Defaults.instance()
        assert d1 is d2
