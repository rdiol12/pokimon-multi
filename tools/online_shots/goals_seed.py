"""Other players have nearly reached the week's goals."""
import sys


def seed(port, out):
    sys.path.insert(0, "server")
    gary = Fake(port, "Gary", 4242)  # noqa: F821 (run.py provides Fake)
    for i in range(298):
        gary.ask(f"OEV CATCH 263 {1 if i < 2 else 0} 0 16 {1 if i < 63 else 0}")
    test2 = Fake(port, "TEST2", 0x1002)  # noqa: F821
    test2.ask("OEV CATCH 263 0 0 16 0")
    gary.close()
    test2.close()
