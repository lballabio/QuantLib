"""Singleton pattern base class."""


class Singleton:
    """Base class for singleton pattern. Subclasses get one instance per class.

    Usage::

        class MySettings(Singleton):
            def __init__(self):
                self.value = 0

        MySettings.instance().value  # Access the unique instance
    """

    @classmethod
    def instance(cls):
        """Return the unique instance, creating it on first call."""
        if not hasattr(cls, "_instance") or cls._instance is None:
            cls._instance = cls()
        return cls._instance
