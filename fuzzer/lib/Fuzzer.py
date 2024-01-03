import re
import os
import time
import errno
import random
import psutil
import configs
import pathlib
import argparse
import datetime
import traceback
import itertools
import subprocess
from multiprocessing import Process
from lib import Remove
from lib.Coverage import CoverageSampler
from lib.ShellProcess import ShellProcess
from lib.Schedule import Scheduler

class FuzzArgs:
  def __init__(self, *, wdir, cc, seed, timeout, duration, testcases, mutator):
    self.wdir = wdir
    self.cc = cc
    self.seed = seed
    self.timeout = timeout
    self.duration = duration
    self.testcases = testcases
    self.mutator = mutator

class Fuzzer:
  def __init__(self, fuzz_args):
    self.fuzz_args = fuzz_args
    ### some configs ###
    self.tmpdir = f"{fuzz_args.wdir}/tmpdir"
    self.coverage_logfile = f"{fuzz_args.wdir}/coverage-trend.txt"
    nbrs = fuzz_args.cc.myafl_nbrs
    self.sampler = CoverageSampler(nbrs)
    self.ofiles_cnt = 0
  @staticmethod
  def clean_file(filename):
    os.system(f"rm -rf {filename}")
  @staticmethod
  def wait(file_or_dir, mode=os.F_OK, timeout=1):
    st = time.time()
    ed = st + (timeout_seconds := timeout)
    delay = 0.01
    while time.time() < ed:
      if os.access(file_or_dir, mode): return True
      time.sleep(delay)
      remaining = ed - time.time()
      delay = min(delay * 2, remaining, .05)
    return False

  def mark_current_state(self, state):
    with open(f"{self.fuzz_args.wdir}/state.txt", 'w+') as fp:
      fp.write(state + "\n")

  def next_ifile(self):
    return random.choice(self.fuzz_args.testcases)

  def next_ofile(self):
    self.ofiles_cnt += 1
    return f"{self.fuzz_args.wdir}/{self.ofiles_cnt}.c"

  def prologue(self):
    os.system(f"mkdir -p {self.fuzz_args.wdir}")
    os.system(f"mkdir -p {self.tmpdir}")
    os.environ['TMPDIR'] = self.tmpdir
    random.seed(self.fuzz_args.seed)

  def save_coverage(self, coverage):
    m = coverage.get_covered_branches_count()
    n = coverage.get_total_branches_count()
    with open(self.coverage_logfile, 'a+') as fp:
      now = datetime.datetime.now()
      fp.write(f'{now},{m},{n},{m / n}\n')

  def compile(self, cfile):
    cmd = f"{self.fuzz_args.cc.myafl_path} {cfile} -O2 -o /dev/null -w "
    cmd += ' '.join([f"-I{d}" for d in configs.csmith_incdirs])
    p = ShellProcess(cmd,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        tlimit=self.fuzz_args.timeout)
    retcode, isTimeout, _, _ = p.communicate()
    # perform potential checks over [retcode, isTimeout]

  def fuzz(self):
    self.prologue()
    st_time = datetime.datetime.now()
    ed_time = st_time + self.fuzz_args.duration
    while datetime.datetime.now() < ed_time:
      self.mark_current_state('running')
      # perform mutation
      ifile = self.next_ifile()
      ofile = self.next_ofile()
      self.fuzz_args.mutator.mutate(ifile, ofile)
      if not self.wait(ofile): continue
      # check coverage
      prev_coverage = self.sampler.sample()
      with self.sampler:
        self.compile(ofile)
      curr_coverage = self.sampler.sample()
      if curr_coverage.outperforms(prev_coverage):
        self.fuzz_args.testcases.append(ofile)
        self.save_coverage(curr_coverage)
      else:
        self.clean_file(ofile)
      # clean tmpdir
      Remove.remove_files(self.tmpdir)
    self.mark_current_state('done')
