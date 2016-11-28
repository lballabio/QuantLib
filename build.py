import os
import time
from threading import Thread, Event

from conan.packager import ConanMultiPackager
from conanfile import QuantLibConan


reference = "{}/{}".format(QuantLibConan.name, QuantLibConan.version)
username = os.getenv("CONAN_USERNAME", "jgsogo")


def heartbeat(sleep_seconds, stop_event):
    while not stop_event.is_set():
        stop_event.wait(sleep_seconds)
        print(".")


if __name__ == "__main__":
    builder = ConanMultiPackager(username=username,
                                 reference=reference,
                                 stable_branch_pattern='1.9',
                                 visual_runtimes=["MT", "MTd"])
    builder.add_common_builds()

    # Create a Thread to print something each 5 minutes (avoid timeout in travis)
    t_stop = Event()
    t = Thread(target=heartbeat, args=(300, t_stop))
    t.start()
    builder.run()
    t_stop.set()
