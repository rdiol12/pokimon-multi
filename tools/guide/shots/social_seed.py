"""Gary (eight badges) is on the mentor list, so a new trainer gets him."""
ENV = {"NETPLAY_FISHT_LENGTH": 0}


def seed(port, out):
    f = Fake(port, "Gary", 4242)  # noqa: F821 (run.py provides Fake)
    f.ask("MENTOR VOLUNTEER 8")
    f.close()
