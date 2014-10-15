import fscc

if __name__ == '__main__':
    p = fscc.Port(0)

    status = p.rx_multiple

    p.rx_multiple = True
    p.rx_multiple = False
