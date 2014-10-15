import fscc

if __name__ == '__main__':
    p = fscc.Port(0)

    p.write(b'Hello world!')
    print(p.read(100)[0])
