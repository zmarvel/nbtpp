

import struct

def print_hex(seq):
    print(' '.join('{:x}'.format(c) for c in seq))

def enc_str(s):
    print(len(s))
    print_hex(list(map(ord, s)))

def enc_double(d):
    print_hex(struct.pack('>d', d))

def enc_float(f):
    print_hex(struct.pack('>f', f))


if __name__ == '__main__':
    import argparse as ap

    parser = ap.ArgumentParser()
    parser.add_argument('-s', '--string', action='store_true',
                        help='Encode the argument as a string')
    parser.add_argument('-f', '--float', action='store_true',
                        help='Encode the argument as a float')
    parser.add_argument('-d', '--double', action='store_true',
                        help='Encode the argument as a double')
    parser.add_argument('value',
                        help='Unencoded value')
    args = parser.parse_args()

    if args.string:
        enc_str(args.value)
    elif args.float:
        enc_float(float(args.value))
    elif args.double:
        enc_double(float(args.value))
