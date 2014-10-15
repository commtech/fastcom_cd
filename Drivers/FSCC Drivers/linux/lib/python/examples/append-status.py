import fscc

if __name__ == '__main__':
    p = fscc.Port(0)

    status = p.append_status

    p.append_status = True
    p.append_status = False
