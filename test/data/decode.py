

def dec_string(value):
    return ''.join([chr(int(v, 16)) for v in value.split(' ')])

if __name__ == '__main__':
    import argparse as ap

    parser = ap.ArgumentParser()
    parser.add_argument('value',
                        help='Encoded value (hex)')
    args = parser.parse_args()
    
    print(dec_string(args.value))
