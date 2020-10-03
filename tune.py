'''generate the configuration to tune and set the decimation factor'''
import argparse
import struct
import sys

parser = argparse.ArgumentParser()
parser.add_argument('--freq', nargs=1, type=int, required=True, help='Tuner center frequency in hertz')
parser.add_argument('--decimate', nargs=1, default=32, type=int, help='Decimation factor. Default = 32')
args = parser.parse_args()

cfg = struct.pack('>II', args.freq[0], args.decimate[0])
sys.stdout.buffer.write(cfg)
