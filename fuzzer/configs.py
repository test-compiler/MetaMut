import os

####### CONFIGS ######
grayc_home  = f"..."
mut_s_home  = f"..."
mut_u_home  = f"..."
csmith_home = f"..."
afl_home    = f"..."

grayc_bin  = f"..."
mut_s_bin  = f"..."
mut_u_bin  = f"..."
csmith_bin = f"..."
afl_fuzz_bin = f"..."

csmith_incdirs = [
  f"{csmith_home}/runtime",
  f"{csmith_home}/output/runtime"
]

myafl_gcc_objdir = f"..."
myafl_gcc_bin = f"..."
myafl_gcc_nbrs = 0 # my-afl-cc reported number of branches

myafl_llvm_objdir = f"..."
myafl_clang_bin = f"..."
myafl_clang_nbrs = 0 # my-afl-cc reported number of branches

aflpp_gcc_objdir = f"..."
aflpp_gcc_bin = f"..."
aflpp_gcc_envs = { "AFL_SKIP_CPUFREQ": "1" }

aflpp_llvm_objdir = f"..."
aflpp_clang_bin = f"..."
aflpp_clang_envs = {
    "AFL_SKIP_CPUFREQ": "1", "AFL_MAP_SIZE": "...", }
