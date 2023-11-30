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
from lib.Mutators import *
from lib.ShellProcess import ShellProcess
from lib.Schedule import Scheduler
from lib.Fuzzer import FuzzArgs, Fuzzer
from lib.Mutators import make_mutators
from lib.Compilers import make_compilers

def record_exception(wdir, e):
  os.system(f'mkdir -p {wdir}')
  s  = f"======= {datetime.datetime.now()} =======\n"
  s += ''.join(traceback.format_exception(
    type(e), e, e.__traceback__))
  with open(f'{wdir}/exceptions.txt', 'a+', errors='ignore') as fp:
    fp.write(s)
  print(s)

def worker(fuzz_args):
  try:
    fuzzer = Fuzzer(fuzz_args)
    fuzzer.fuzz()
  except KeyboardInterrupt as e:
    raise
  except Exception as e:
    record_exception(fuzz_args.wdir, e)
    if isinstance(e, OSError) and e.errno == errno.ENOSPC:
      raise

def load_testcases(args):
  testcases = []
  for cfile in pathlib.Path(args.seeds_dir).glob('**/*.c'):
    testcases.append(str(cfile))
  random.shuffle(testcases)
  return testcases

def make_worker_list(args):
  worker_args_list = []
  for i in range(args.repeat_times):
    for cc in make_compilers():
      for mutator in make_mutators(cc, args.seeds_dir,
          lambda name: f"{args.wdir}/{name},{cc},{i}"):
        worker_args_list.append(FuzzArgs(
          wdir=f"{args.wdir}/{mutator},{cc},{i}",
          cc=cc,
          seed=random.randint(0, 2**31-1),
          timeout=args.timeout,
          duration=args.duration,
          testcases=testcases,
          mutator=mutator))
  return worker_args_list

def parse_args():
  parser = argparse.ArgumentParser('driver')
  parser.add_argument('-j', dest='jobs',
    default=(psutil.cpu_count() // 2),
    help='specify number of jobs')
  parser.add_argument('--repeat-times', dest='repeat_times',
    default=10, type=int, help='specify repeat times')
  parser.add_argument('--wdir', dest='wdir',
    default=str(os.getcwd()),
    help='specify number of jobs')
  parser.add_argument('--seed', dest='seed',
    required=True, help='specify number of jobs')
  parser.add_argument('--timeout', dest='timeout',
    default=1, help='specify timeout seconds')
  parser.add_argument('--seeds-dir', dest='seeds_dir',
    required=True, type=str,
    help='specify where to store the seeds')
  return parser.parse_args()

if __name__ == "__main__":
  args = parse_args()
  args.duration = datetime.timedelta(hours=24)
  random.seed(args.seed)
  testcases = load_testcases(args)
  worker_args_list = make_worker_list(args)
  scher = Scheduler(worker, worker_args_list, args.jobs)
  scher.execute_jobs()
