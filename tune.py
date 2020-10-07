'''generate the configuration to tune and set the decimation factor'''
import argparse
import struct
import sys

parser = argparse.ArgumentParser()
parser.add_argument('--freq', type=int, required=True, help='Tuner center frequency in hertz')
parser.add_argument('--downsample', type=int, default=32, help='Downsampling factor. Default = 32')
args = parser.parse_args()

cfg = struct.pack('>II', args.freq, args.downsample)
sys.stdout.buffer.write(cfg)
