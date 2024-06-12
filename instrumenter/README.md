This project is a instrumenter modified from AFL++'s afl-cc.

**Our modification**:
1. We introduce a global counter dispatching server to make to basic block id free of collision, such collision is critical in large code base like gcc and clang. You may find detail about basic block collision [here](https://github.com/AFLplusplus/AFLplusplus/blob/stable/instrumentation/README.lto.md), AFL++'s solution to this problem is rather slow in practical and seems contain bug in our evaluation.
2. We uses a compact representation of coverage map, similar to AFL++'s `trace_mini`.

**Requirements**:
To compile and run `my-afl-cc`, we recommend using Ubuntu (version 22.04 or later) with the following requirements.
- `libllvm15`
- `libclang-15-dev`
- `llvm-15-tools`
- `cmake` (>= 3.27)
- `python` (>= 3.9.0)

**Compile**:
```
cd instrumenters/my-afl-cc
make LLVM_CONFIG=$(which llvm-config-15)
```

**Run**:
To clarify how to compile a C/C++ project using `my-afl-cc`, we provide an example that showcases compilation steps of Clang.
```
export MY_AFL_PATH=/* path to my-afl-cc */

git clone git@github.com:llvm/llvm-project

PRJDIR=$(pwd)/llvm-project
OBJDIR=$(pwd)/llvm-object
mkdir -p $OBJDIR; cd $OBJDIR

cmake $PRJDIR/llvm -DLLVM_TOOL_CLANG_BUILD=ON \
  -DCMAKE_C_COMPILER=$MY_AFL_PATH/afl-gcc-fast \
  -DCMAKE_CXX_COMPILER=$MY_AFL_PATH/afl-g++-fast
python3 $MY_AFL_PATH/server.py --bbmap-dir basic-block-maps -- make clang
```
