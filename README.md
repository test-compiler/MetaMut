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

Our project has already generated over 100 semantic aware mutators. Leveraging these mutators, we have successfully discovered over 100 bugs in two extensively tested modern C/C++ compilers, namely GCC and Clang.

**GCC/Clang Bugs**: We have reported 104 bugs to GCC/Clang. A comprehensive list of these reported bugs is available [here](data/bugs.md).

**Coverage Comparation**: We conducted a comparative coverage experiment with existing tools. Detailed data from this study can be found [here](data/CoverageTrend).

## MetaMut Implementation
We are pleased to announce the publication of our project, MetaMut. You can access it in [MetaMut](MetaMut).

**Requirements**:
To compile and run MetaMut, we recommend using Ubuntu (version 22.04 or later) with the following requirements.
- `libllvm12`
- `libclang-12-dev`
- `llvm-12-tools`
- `cmake` (>= 3.27)
- `python` (>= 3.9.0)

**Run**:
To execute MetaMut, you'll first need to input your Azure GPT-4 API key into the [configs.py](MetaMut/scripts/configs.py) file.

Afterwards, you can run MetaMut by following these steps:
```
cd MetaMut;
python3 scripts/main.py
```

## Generated Mutators
We have also published our generated mutators. You can access it in [mutators](mutators).
- The [supervised](mutators/supervised) version involves expert interaction
- The [unsupervised](mutators/unsupervised) version is fully automated
  * For unsupervised version, the generation logs can be found [here](mutators/unsupervised/logs).

**Requirements**:
To compile these mutators, we recommend using Ubuntu (version 22.04 or later) with the following requirements.
- `libllvm12`
- `libclang-12-dev`
- `llvm-12-tools`
- `cmake` (>= 3.27)

**Compile and Run**:
To run our generated mutators, follow the steps below using the supervised version as an example.
```
cd mutators/supervised
mkdir output && cd output
cmake .. && make

./MutatorCollection --list-mutators # list all available mutators
./MutatorCollection --mutator=XXX -i input.c -o output.c # perform mutation
```

## Misc
- Our coverage experiment relies on a modified version of AFL++'s instrumenter. You can find the complete source code [here](instrumenters/my-afl-cc).
- We used a simple fuzzer for our coverage experiment. You can find the complete source code [here](fuzzer).
- We have made modifications to AFL++ for statistical purposes, specifically to measure the throughput of compilable mutants. You can find our modified version of AFL++ [here](AFLplusplus).
