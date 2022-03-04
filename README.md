### Note: This is a fork of MulticoreWare's repo: https://bitbucket.org/multicoreware/libfgm

&nbsp;

# Libfgm
- Libfgm can be used to extract characteristics of the film grain and add it to a binary file.
- Libfgm is available under APACHE 2.0 license
## Requirements:
- OpenCV 4.5 or higher (http://opencv.org/downloads.html);
- One of your favourite IDE/compiler(with C++11 support).

### Building

```bash
g++ -O3 *.cpp $(pkg-config opencv4 --cflags --libs)
```

## Sample commandline:
- libfgm --input noise.yuv --width W --height H --output filtered.yuv --model model.bin [ --pix-fmt <yuv pixel format> --kalman-weight <integer> --wiener-weight <integer> --wiener-filter <0/1/2> --frames <integer> --skip <integer> --frequency-filter <0/1>]
### Help:
- --input <input film grain noisy file>
- --output <filtered output file>
- --model <output model file with film grain parameters>
- --width W - width of the input video
- --height H - height of the input video
- --wiener-weight <integer> Range: [1,9]; default: 1
- --kalman-weight <integer> Range: [0,9]; default: 1
- --wiener-filter <0/1/2> 0 - 3x3 filter; 1 - 5x5 filter; 2 - 7x7 filter; default: 0
- --frames <integer> - count of frames to process
- --skip <integer> - count of frames to skip initially
- --pix-fmt <yuv pixel format> [yuv420p, yuv422p, yuv444p, yuv422p10, yuv444p10, yuv420p10]; default: yuv420p
- --model-id <0/1>; default: 0
        0 = FrequencyFilteringModel
        1 = AutoRegressionModel
