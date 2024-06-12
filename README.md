<p align="center">
  <h1 align="center"><strong>MetaMut</strong></h1>
</p>

<p align="center">
    <a href="https://semver.org" alt="Version">
        <img src="https://img.shields.io/badge/release-v0.1.0-blue" />
    </a>
    <a href="https://google.github.io/styleguide/javaguide.html" alt="Code style">
        <img src="https://img.shields.io/badge/style-Google-blue" />
    </a>
    <a href="https://opensource.org/licenses/MIT" alt="License">
        <img src="https://img.shields.io/github/license/test-compiler/MetaMut" />
    </a>
</p>

MetaMut is a generator to generate semantic aware mutators for testing C/C++ compilers.

Our project has already generated over 200 semantic aware mutators. Leveraging these mutators, we have successfully discovered over 130 bugs in two extensively tested modern C/C++ compilers, namely GCC and Clang.

**GCC/Clang Bugs**: We have reported over 130 bugs to GCC/Clang. A comprehensive list of these reported bugs is available [here](data/bugs.md).

## Installation

To execute MetaMut, we recommend using Ubuntu (version 22.04 or later).
Install the required packages using the following commands:
```
sudo apt-get install llvm-12 llvm-12-dev llvm-12-tools libllvm12  libclang-12-dev
sudo apt-get install cmake gdb gcc g++ python3 python3-pip

sudo pip3 install tiktoken jinja2 openai
```

## Execute MetaMut

To execute MetaMut, you'll first need to fill in your GPT-4 API key in the [configs.py](Core/scripts/configs.py) file.

Afterwards, you can run MetaMut by following these steps:
```
cd Core;
python3 scripts/main.py --num-mutators=100
```

In case that openai updated their API, you may use https interface like this:
```
cd Core;
python3 scripts/main.py --num-mutators=100 --api https.openai
```

## Generated Mutators

We have also published our generated mutators. You can access it in [mutators](mutators).

To compile these mutators:
```
cd mutators
mkdir output && cd output
cmake .. -DLLVM_CONFIG_BINARY=$(which llvm-config-12)
make -j4
```

To run these mutators:
```
$ cat input.c
#include <string.h>
int main() {
  return strlen("123");
}
$ ./muss --randomly-try-all-mutators -i input.c -o output.c -seed 10293
$ cat output.c
#include <string.h>
int main() {
    return ((strlen("123")) | (strlen("123")));
}
$ ./muss --randomly-try-all-mutators -i output.c -o output.c -seed 28596
$ cat output.c
#include <stdio.h>
int main() {
  return ((strlen("123")) ^ (strlen("123")));
}
```

## Fuzzing Compilers

Fuzzing scripts are located in the [fuzzer](fuzzer) directory. For effective fuzzing, instrumented versions of GCC and Clang are required.

You can either compile and instrument the compilers manually using our provided [setup.sh](setup.sh) script or use pre-compiled binaries.

**Manually Instrument Compilers**
```
# install required packages
sudo apt-get install -y flex bison yacc
sudo apt-get install libllvm15 libclang-15-dev llvm-15-tools
sudo apt-get install gcc-11 g++-11 gcc-11-plugin-dev
sudo pip3 install sysv_ipc numpy
sudo update-alternatives --install /usr/bin/llvm-config llvm-config $(which llvm-config-12) 999999

# run setup.sh, this may take several hours to finish
export CC_DWNDIR=$(pwd)/compilers
export CC_OBJDIR=$(pwd)/objects
export JOBS=4
bash setup.sh
```

**Use Pre-compiled Compilers** (only tested under ubuntu 22.04):
```
cd MetaMut/..
git clone git@github.com:test-compiler/MetaMutAssets
cd MetaMut
ln -s ../MetaMutAssets/objects .
```

**Seed Programs**:
We also collect a large set of seeds (continuously update), you can restrieve them via:
```
cd MetaMut/..
git clone git@github.com:test-compiler/MetaMutAssets
cd MetaMut
ln -s ../MetaMutAssets/seeds .
```

After setting up compilers and seeds, run the fuzzer with the following commands:
```
ulimit -s unlimited # Avoid fake crash caused by deep recursion
ulimit -c 0         # Disable core dumps if your kernel is configured to allow them

mkdir workspace; cd workspace
python3 ../fuzzer/run.py -j 4 \
  --wdir $(pwd) \
  --repeat-times 10 \
  --duration 86400 \
  --seeds-dir $(pwd)/../seeds \
  --cc-opt=-O2
```

Options explanation:
- `-j 4`: Utilize up to 4 CPU cores.
- `--repeat-times 10`: Number of fuzzing instances to run for each compiler.
- `--duration 86400`: Each fuzzing instance will use up to 86400 seconds
- `--seeds-dir`: Directory containing seed programs.
- `--cc-opt`: Compiler options to apply during fuzzing, eg. `--cc-opt=-O2 --cc-opt=-O3` will fuzz `-O2` and `-O3`
