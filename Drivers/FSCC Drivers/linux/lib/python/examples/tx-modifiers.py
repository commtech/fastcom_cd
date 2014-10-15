import fscc

if __name__ == '__main__':
    p = fscc.Port(0)

    modifiers = p.tx_modifiers

    # Enable transmit repeat & transmit on timer
    p.tx_modifiers = fscc.TXT | fscc.XREP

    # Revert to normal transmission
    p.tx_modifiers = fscc.XF