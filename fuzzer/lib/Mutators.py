import os
import time
import random
import shutil
import pathlib
import configs
import threading
import subprocess
from .ShellProcess import ShellProcess
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

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

class Csmith:
  def __init__(self, mutator_bin):
    self.mutator_bin = mutator_bin
  def __str__(self): return "csmith"
  def mutate(self, ifile, ofile):
    seed = random.randint(0, 2**31 - 1)
    cmd = f"{self.mutator_bin} --seed {seed} > {ofile}"
    p = ShellProcess(cmd,
       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    retcode, _, _, _ = p.communicate()
    return retcode == 0

class GrayC:
  def __init__(self, mutator_bin):
    self.mutator_bin = mutator_bin
  def __str__(self): return "grayc"
  def mutate(self, ifile, ofile):
    os.system(f"cp {ifile} {ofile}")
    wait(ofile)
    seed = random.randint(0, 2**31 - 1)
    cmd  = f'{self.mutator_bin} --seed {seed}'
    cmd += f' --mutations="*" --apply-mutation {ofile}'
    p = ShellProcess(cmd,
       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    retcode, _, _, _ = p.communicate()
    return retcode == 0

class SupervisedMutatorCollection:
  def __init__(self, mutator_bin):
    self.mutator_bin = mutator_bin
  def __str__(self): return "mut_s"
  def mutate(self, ifile, ofile):
    seed = random.randint(0, 2**31 - 1)
    cmd  = f'{self.mutator_bin} --seed {seed}'
    cmd += f' --randomly-try-all-mutators'
    cmd += f' -i {ifile} -o -'
    p = ShellProcess(cmd,
       stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    retcode, _, stdout, _ = p.communicate()
    stdout = f"// from: {cmd}\n{stdout}"
    pathlib.Path(ofile).write_text(stdout)
    return retcode == 0

class UnsupervisedMutatorCollection:
  def __init__(self, mutator_bin):
    self.mutator_bin = mutator_bin
  def __str__(self): return "mut_u"
  def mutate(self, ifile, ofile):
    seed = random.randint(0, 2**31 - 1)
    cmd  = f'{self.mutator_bin} --seed {seed}'
    cmd += f' --randomly-try-all-mutators'
    cmd += f' -i {ifile} -o -'
    p = ShellProcess(cmd,
       stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    retcode, _, stdout, _ = p.communicate()
    stdout = f"// from: {cmd}\n{stdout}"
    pathlib.Path(ofile).write_text(stdout)
    return retcode == 0

class AFLplusplus(FileSystemEventHandler):
  def __init__(self, afl_fuzz_bin, cc_info, idir, odir,
      mutator_lib=None, use_custom_only=False, timeout=1,
      afl_envs={}, optlevel="-O2"):
    self.wdir = odir
    self.afl_fuzz = afl_fuzz_bin
    self.afl_envs = afl_envs
    self.mutator_lib = mutator_lib
    self.use_custom_only = use_custom_only
    self.cc_info = cc_info
    self.cc_path = cc_info.aflpp_path
    seed = random.randint(0, 2**31 - 1)
    self.cmd = f"{afl_fuzz_bin} -i {idir} -o {odir} -t {timeout*1000} -s {seed} -- "
    self.cmd += f"{self.cc_path} {optlevel} -o /dev/null -w -xc @@"
    self.newfiles = set([])
    self.lock = threading.Lock()
    self.afl_p = None
  def __str__(self): return "afl++"
  def on_created(self, event):
    if not event.is_directory:
      self.newfiles.add(str(event.src_path))
  def on_modified(self, event):
    if not event.is_directory:
      self.newfiles.add(str(event.src_path))
  def on_deleted(self, event):
    if not event.is_directory:
      with self.lock:
        if str(event.src_path) in self.newfiles:
          self.newfiles.remove(str(event.src_path))
  def setup_envs(self):
    os.environ.update(self.afl_envs)
    os.environ.update(self.cc_info.aflpp_envs)
    if self.mutator_lib is not None:
      os.environ['AFL_CUSTOM_MUTATOR_LIBRARY'] = self.mutator_lib
    if self.use_custom_only:
      os.environ['AFL_CUSTOM_MUTATOR_ONLY'] = "1"
    os.environ['AFL_NO_UI'] = "1"
    os.environ['AFL_TMPDIR'] = f"{self.wdir}/tmpdir"
    os.system(f"mkdir -p {self.wdir}/tmpdir");
  def run_afl(self, watch_queue=True):
    self.setup_envs()
    watchdir = f"{self.wdir}/default/queue"
    self.afl_p = ShellProcess(self.cmd)
      #stdout=open(f'{self.wdir}/stdout.txt', 'a+'),
      #stderr=open(f'{self.wdir}/stderr.txt', 'a+'))
    # wait until watchdir created
    if watch_queue:
      while not os.access(watchdir, os.X_OK): pass
      self.observer = Observer()
      self.observer.schedule(self, path=watchdir)
      self.observer.start()
  def mutate(self, ifile, ofile):
    if self.afl_p is None:
      self.run_afl()
    while True:
      with self.lock:
        if len(self.newfiles) > 0:
          newfile = self.newfiles.pop()
          shutil.copyfile(newfile, ofile)
          if 'allow_remove' in str(newfile):
            os.system(f'rm -rf {newfile}')
          break
      time.sleep(0.01)
    return True
  def is_finished(self):
    return self.afl_p.poll() is not None
  def clean(self):
    self.afl_p.terminate()
    self.observer.stop()

def make_mutators(cc, seeds_dir, wdir_ctor):
  return [
      Csmith(configs.csmith_bin),
      GrayC(configs.grayc_bin),
      SupervisedMutatorCollection(configs.mut_s_bin),
      UnsupervisedMutatorCollection(configs.mut_u_bin),
      AFLplusplus(configs.afl_fuzz_bin,
        cc, seeds_dir, wdir_ctor("AFL++")),
  ]
