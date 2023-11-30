import os
import signal
import subprocess

class ShellProcess(subprocess.Popen):
  print_exec = False
  def __init__(self, cmd, tlimit=30, mlimit=2**30,
      shell=True, stdout=None, stderr=None):
    self.cmd = cmd
    # with open('cmds.list', 'a+') as fp:
    #   fp.write(cmd + "\n")
    self.tlimit = tlimit
    self.mlimit = mlimit
    super().__init__(cmd, shell=shell,
        preexec_fn=os.setpgrp,
        stdout=stdout, stderr=stderr,
        text=True, errors='ignore')
  def communicate(self):
    if self.print_exec:
      print(f"exec: {self.cmd}")
    # run the shell
    retcode, isTimeout = None, False
    stdout, stderr = None, None
    try:
      stdout, stderr = super().communicate(timeout=self.tlimit)
    except subprocess.TimeoutExpired:
      if self.stdout is not None: stdout = ''
      if self.stderr is not None: stderr = ''
      pgrp = os.getpgid(self.pid)
      os.killpg(pgrp, signal.SIGKILL)
      isTimeout=True
    self.wait()
    if self.print_exec:
      print(f"exec: result: {self.poll()}, {isTimeout}")
    return self.poll(), isTimeout, stdout, stderr
  def terminate(self):
    try:
      pgrp = os.getpgid(self.pid)
      os.killpg(pgrp, signal.SIGKILL)
    except ProcessLookupError:
      pass
