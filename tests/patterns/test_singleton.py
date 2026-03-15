"""Tests for Singleton base class.

Hand-written (no upstream C++ test file).
"""

from quantlib.patterns.singleton import Singleton


class MySingleton(Singleton):
    def __init__(self):
        self.value = 42


class AnotherSingleton(Singleton):
    def __init__(self):
        self.name = "other"


class TestSingleton:
    def test_instance_identity(self):
        """Same instance returned each time."""
        a = MySingleton.instance()
        b = MySingleton.instance()
        assert a is b

    def test_subclass_isolation(self):
        """Different subclasses get independent instances."""
        a = MySingleton.instance()
        b = AnotherSingleton.instance()
        assert a is not b
        assert isinstance(a, MySingleton)
        assert isinstance(b, AnotherSingleton)

    def test_instance_has_state(self):
        """Instance preserves state across calls."""
        s = MySingleton.instance()
        s.value = 99
        assert MySingleton.instance().value == 99
        # Reset for test isolation
        s.value = 42
