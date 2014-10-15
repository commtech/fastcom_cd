import fscc

if __name__ == '__main__':
    p = fscc.Port(0)

    # TX
    p.purge(True, False)

    # RX
    p.purge(False, True)

    # TX & RX
    p.purge(True, True)
