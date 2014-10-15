import fscc

if __name__ == '__main__':
    p = fscc.Port(0)

    status = p.ignore_timeout

    p.ignore_timeout = True
    p.ignore_timeout = False
