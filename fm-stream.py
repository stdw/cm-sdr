import sys
import time 
import queue
import threading
import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as signal
import scipy.fftpack
import sounddevice as sd
from scipy.io.wavfile import write

OUT_FS = 48000      # target
FS_STRETCH = 0.95
SND_DEV_FS = 48000

IN_FS = int(15000000 / 32)
SHIFT = 0.097e6

def demod(raw):
    SAMPLE_RATE = IN_FS
    
    # read and format data
    data = np.frombuffer(raw, dtype=np.dtype('>i2')).astype(np.float32).view(np.complex64) 
  
    # center
    data=data * np.exp(-1j*2*np.pi*SHIFT*np.arange(start=0,step=1,stop=data.size)/SAMPLE_RATE)
    
    # decimate
    target = 200000
    downsample = int(SAMPLE_RATE / target)
    
    if downsample > 1:
        data = signal.decimate(data, downsample, ftype='fir')
        SAMPLE_RATE /= downsample
        
    # fm demodlation
    # https://stackoverflow.com/questions/53706653/continuously-play-sampled-audio-data-in-python 
    data = data[1:] * np.conj(data[:-1])  
    data = np.angle(data)  

    target = OUT_FS
    data_time = np.size(data)/SAMPLE_RATE
    out_samples = int(data_time * target)
    data = signal.resample(data, out_samples)
    SAMPLE_RATE = target

    # de-emphasis filter
    d = SAMPLE_RATE * 75e-6     # Calculate the # of samples to hit the -3dB point  
    x1 = np.exp(-1/d)           # Calculate the decay between each sample  
    b = [1-x1]                  # Create the filter coefficients  
    a = [1,-x1]  
    data = signal.lfilter(b,a,data)  

    # normalize
    if np.max(np.abs(data)) == 0:
        data = np.int16(data)
    else:
        data = np.int16(data/np.max(np.abs(data)) * 32767)

    return data.tobytes()



chunk = 1024*512
blksize = int(len(demod(bytes(chunk))) / 2)

q = queue.Queue(maxsize=5)
event = threading.Event()


def callback(outdata, frames, time1, status):
    global OUT_FS
    global chunk
    data = b''

    try:
        data = q.get_nowait()
    except queue.Empty: # if queue if empty slow down consumption
        OUT_FS = int(OUT_FS / FS_STRETCH)
        chunk = (int(chunk*FS_STRETCH) // 4) * 4
        print(f'Queue is empty. Increased sample rate: {OUT_FS}', file=sys.stderr)

    if len(data) < len(outdata):
        outdata[:len(data)] = data
        outdata[len(data):] = b'\x00' * (len(outdata) - len(data))
    else:
        outdata[:] = data[:len(outdata)]


for i in range(2):
    data = demod(sys.stdin.buffer.read(chunk))
    q.put_nowait(data)

stream = sd.RawOutputStream(
    samplerate=SND_DEV_FS, blocksize=blksize,
    device=0, channels=1, dtype='int16', 
    callback=callback, finished_callback=event.set)

with stream:
    raw = sys.stdin.buffer.read(chunk)
    while len(raw) > 0:
        data = demod(raw)
        try:
            q.put_nowait(data)
        except: # if queue is full, speed up consumption
            OUT_FS = int(OUT_FS * FS_STRETCH)
            chunk = (int(chunk/FS_STRETCH) // 4) * 4
            print(f'Queue is full. Decreased sample rate: {OUT_FS}')
        raw = sys.stdin.buffer.read(chunk)
    event.wait()


#if __name__ == '__main__':
#    main()
