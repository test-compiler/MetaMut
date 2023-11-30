This project is a micro fuzzer designed for collecting coverage.

**Requirements**:
- `watchdog`
- `numpy`
- `psutil`
- `sysv_ipc`
- `python` (>= 3.9.0)

**Run**:
To run this micro fuzzer, you'll first need to config fuzzers and instrumented compilers in [configs.py](configs.py).

Then, follow these steps to run:
```
python3 runCoverageTrend.py \
  --repeat-times=1 --wdir=<wdir> --seeds-dir=<seeds-dir>
```
