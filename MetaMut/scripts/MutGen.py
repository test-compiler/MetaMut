import re
import os
import json
import random
import shutil
import configs
import difflib
import pathlib
import filecmp
import datetime
import subprocess
from enum import Enum
from gdb import GDBController
from ShellProcess import ShellProcess
from jinja2 import Environment, FileSystemLoader

class ExceedMaxQueryTimes(Exception):
  def __init__(self, name, desc):
    self.name = name
    self.desc = desc

class MutatorImpl:
  def __init__(self, name, desc, code):
    self.name = name
    self.desc = desc
    self.code = code

class CheckRes:
  Normal = 1
  Hang = 2
  InvalidChange = 3
  NoChange = 4
  Crash = 5
  NotCompile = 6
  ApplyFail = 7
  def __init__(self, res):
    self.res_code = res
    for k, v in CheckRes.__dict__.items():
      if v == res:
        self.res_str = k
        break
  def format_rcode(self):
    if self.res_code == self.Normal: return "Normal"
    if self.res_code == self.Hang: return "Hang"
    if self.res_code == self.InvalidChange: return "InvalidChange"
    if self.res_code == self.NoChange: return "NoChange"
    if self.res_code == self.Crash: return "Crash"
    if self.res_code == self.NotCompile: return "NotCompile"
    if self.res_code == self.ApplyFail: return "ApplyFail"
    assert False
  def is_valid(self):
    return self.res_code == self.Normal
  def isa(self, code):
    return self.res_code == code
  @classmethod
  def from_apply_fail(cls, isrc):
    inst = cls(CheckRes.ApplyFail)
    inst.isrc = isrc
    return inst
  @classmethod
  def from_normal(cls):
    return cls(CheckRes.Normal)
  @classmethod
  def from_crash(cls, stderr, backtrace):
    inst = cls(CheckRes.Crash)
    inst.stderr = stderr
    inst.backtrace = backtrace
    return inst
  @classmethod
  def from_invalid(cls, isrc, osrc, diff, errors):
    inst = cls(CheckRes.InvalidChange)
    inst.isrc = isrc
    inst.osrc = osrc
    inst.diff = diff
    inst.errors = errors
    return inst
  @classmethod
  def from_hang(cls, isrc, backtrace):
    inst = cls(CheckRes.Hang)
    inst.isrc = isrc
    inst.backtrace = backtrace
    return inst
  @classmethod
  def from_nochange(cls, isrc):
    inst = cls(CheckRes.NoChange)
    inst.isrc = isrc
    return inst
  @classmethod
  def from_not_compile(cls, errors):
    errors = errors[:2000]
    inst = cls(CheckRes.NotCompile)
    inst.errors = errors
    return inst

class ParseUtils:
  codepat = re.compile(
      r'RegisterMutator<(\w+)>\s*\w+\s*\(\s*"([^"]+)"\s*,\s*"([^"]+)"\s*',
      re.I | re.M)
  @classmethod
  def fetch_codeboxes(cls, output):
    codeboxes = []
    isCodeBoxBegin, codeLines = False, []
    for line in output.split('\n'):
      if line.startswith('```'):
        if isCodeBoxBegin:
          if len(line.strip()) == 3:
            codeboxes.append('\n'.join(codeLines))
            isCodeBoxBegin = False
        else:
          codeLines = []
          isCodeBoxBegin = True
      elif isCodeBoxBegin:
        codeLines.append(line)
    return codeboxes
  @classmethod
  def parse_description(cls, text):
    mutators = []
    res = re.search(r'^\W*(\w+)\W*:', text)
    if res:
      text = re.sub(r'^\W*(\w+)\W*:', '', text)
      return [(res.group(1), text.strip())]
    return []
  @classmethod
  def parse_example(cls, text):
    return cls.fetch_codeboxes(text)
  @classmethod
  def parse_code(cls, code):
    for codebox in cls.fetch_codeboxes(code):
      res = ParseUtils.codepat.search(codebox)
      if res:
        name = res.group(1)
        desc = res.group(3)
        return MutatorImpl(name, desc, codebox)
    return None

class ExecUtils:
  max_exec_times = 3
  @classmethod
  def compile(cls, impl):
    mut_cpp_file = f"lib/workbench/{impl.name}.cpp"
    pathlib.Path(mut_cpp_file).write_text(impl.code)
    p = ShellProcess("cd output; cmake ..; make -j8",
        stdout=subprocess.DEVNULL,
        stderr=subprocess.PIPE, tlimit=300)
    retcode, isTimeout, stdout, stderr = p.communicate()
    if retcode == 0: return CheckRes.from_normal()
    try:
      os.remove(mut_cpp_file)
    except FileNotFoundError:
      pass
    return CheckRes.from_not_compile(stderr)
  @classmethod
  def checkValidity(cls, ifile, ofile):
    p = ShellProcess(f'clang -w {ifile} -c -o /dev/null')
    retcode, _, _, _ = p.communicate()
    if retcode == 0:
      p = ShellProcess(f'clang -w {ofile} -c -o /dev/null',
          stderr=subprocess.PIPE)
      retcode, _, _, stderr = p.communicate()
      if retcode == 1:
        isrc = pathlib.Path(ifile).read_text()
        osrc = pathlib.Path(ofile).read_text()
        diff = difflib.unified_diff(isrc.splitlines(True),
            osrc.splitlines(True),
            fromfile=ifile, tofile=ofile)
        return CheckRes.from_invalid(isrc, osrc, ''.join(diff),
            stderr)
      return CheckRes.from_normal()
    return None # no check res
  @staticmethod
  def format_backtrace(frames):
    ret = ''
    for frame in frames:
      level = frame.get('level', '?')
      addr =  frame.get('addr', '0x?')
      func =  frame.get('func', '?')
      line =  frame.get('line', '?')
      fname = frame.get('file', '?')
      fpath = frame.get('fullname', '?')
      ret += f'#{level} {addr} in {func} at {fname}:{line}\n'
      if line != '?' and fpath != '?':
        p = pathlib.Path(fpath)
        if p.is_file():
          line = int(line) - 1
          lines = p.read_text().splitlines()
          if 0 <= line and line < len(lines):
            ret += f'==> {lines[int(line)]}\n'
    return ret
  @classmethod
  def execute(cls, impl, example):
    ifile = f"{configs.objdir}/input.c"
    ofile = f"{configs.objdir}/output.c"
    pathlib.Path(ifile).write_text(example)
    mut_bin_file = f"{configs.mutator_bin}"
    for i in range(cls.max_exec_times):
      seed = random.randint(0, 2**31-1)
      mut_cmd = f"{mut_bin_file} --mutator={impl.name}" \
        f" -i {ifile} -o {ofile} --seed {seed}"
      p = ShellProcess(mut_cmd,
          stdout=subprocess.PIPE,
          stderr=subprocess.PIPE,
          tlimit=10)
      retcode, isTimeout, stdout, stderr = p.communicate()
      if isTimeout:
        pass
      elif retcode == 0:
        if filecmp.cmp(ifile, ofile): # no change
          continue
        else:
          valid = cls.checkValidity(ifile, ofile)
          if valid is not None: return valid
          continue
      elif retcode == 1:
        if i + 1 == cls.max_exec_times:
          return CheckRes.from_apply_fail(example)
        else:
          continue
      # crash or timeout
      g = GDBController(mut_cmd)
      timeout = not g.run(timeout=5)
      frames = g.traceback_frames()[:10]
      backtrace = cls.format_backtrace(frames)
      if timeout:
        return CheckRes.from_hang(example, backtrace)
      return CheckRes.from_crash(stdout + stderr, backtrace)
    return CheckRes.from_nochange(example)

class MutatorGenerator:
  def __init__(self, LLM, logfile='log.txt',
      max_query_times=20):
    self.LLM = LLM
    self.logfile = logfile
    self.eval_ctx = { 'existing_mutators': [] }
    self.query_times = 0

    p = ShellProcess(
        f'{configs.mutator_bin} --list-mutators',
        stdout=subprocess.PIPE)
    _, _, stdout, _ = p.communicate()
    try:
      for name, desc in json.loads(stdout):
        self.eval_ctx['existing_mutators'].append([name, desc])
    except json.decoder.JSONDecodeError:
      pass

    self.max_query_times = max_query_times
    self.stages = {
        "ThinkMutator": "description",
        "Example": "example",
        "ImplMutator": "code",
        "RefineMutator": "code", }
  def jlog(self, **kwargs):
    with open(self.logfile, 'a+') as fp:
      d = {"time":str(datetime.datetime.now()),
          "rounds":self.query_times, **kwargs}
      s = json.dumps(d)
      fp.write(f'\n>>>>>> {s}\n')
  def tlog(self, text):
    with open(self.logfile, 'a+') as fp:
      fp.write(text)
  def query(self, stage):
    self.curr_stage = stage # for testing
    # fetch template
    templates = list(pathlib.Path(f'stages').glob(f'{stage}-*.txt'))
    t = templates[random.randint(0, len(templates) - 1)]
    # generate query
    env = Environment(loader=FileSystemLoader('stages'))
    template = env.get_template(str(t.name))
    output = template.render(**self.eval_ctx)
    output = re.sub(r'\n\n+', r'\n\n', output)
    self.query_times += 1
    # send query
    self.jlog(stage=stage, action="query")
    self.tlog(output)
    response = self.LLM.send(output)
    self.jlog(stage=stage, action="response")
    self.tlog(response)

    # parse response and return
    if self.stages[stage] == "description":
      return ParseUtils.parse_description(response)
    elif self.stages[stage] == "example":
      return ParseUtils.parse_example(response)
    elif self.stages[stage] == "code":
      impl = ParseUtils.parse_code(response)
      return impl
    assert False, f"no output specification for {stage}"
  def check(self, impl, example):
    # compile
    chkres = ExecUtils.compile(impl)
    if chkres.isa(CheckRes.NotCompile): return chkres
    # execute
    chkres = ExecUtils.execute(impl, example)
    if not chkres.is_valid():
      # clean on execute fail
      mut_cpp_file = f"lib/workbench/{impl.name}.cpp"
      os.system(f'rm {mut_cpp_file}')
    return chkres
  def refine(self, impl, examples):
    random.shuffle(examples)
    while True:
      has_example_fail = False
      for example in examples:
        chkres = self.check(impl, example)
        self.jlog(action="log", chkres=chkres.format_rcode(), example=example)
        if chkres.isa(CheckRes.Normal): continue
        has_example_fail = True
        self.eval_ctx["chkres"] = chkres
        self.eval_ctx['mutator_impl'] = impl.code
        self.curr_mutator = impl.name
        while self.query_times < self.max_query_times:
          impl = self.query('RefineMutator')
          if impl is not None: break
        if impl is None:
          raise ExceedMaxQueryTimes(self.mutator_name, self.mutator_desc)
        self.jlog(action="log", example=example)
        if self.query_times >= self.max_query_times: break
      if not has_example_fail or self.query_times >= self.max_query_times:
        break
    if self.query_times < self.max_query_times:
      return impl
    else:
      raise ExceedMaxQueryTimes(self.mutator_name, self.mutator_desc)
  def query_examples(self):
    rl = []
    while len(rl) == 0:
      examples = self.query('Example')
      for example in examples:
        ifile = f"{configs.objdir}/input.c"
        pathlib.Path(ifile).write_text(example)
        p = ShellProcess(
            f'clang -xc -w {ifile} -c -o /dev/null',
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL)
        retcode, _, _, _ = p.communicate()
        self.jlog(action="log", subaction="check_example", retcode=retcode, example=example)
        if retcode == 0: rl.append(example)
    return rl
  def query_implementation(self):
    while True:
      impl = self.query('ImplMutator')
      if impl is not None: return impl
      self.jlog(action="log", status="impl_is_none")
      if self.query_times >= self.max_query_times:
        raise ExceedMaxQueryTimes(self.mutator_name, self.mutator_desc)
    return None
  def run(self):
    # think mutators
    mutators = self.query('ThinkMutator')
    for name, desc in mutators:
      self.curr_mutator = name
      self.eval_ctx['mutator_name'] = name
      self.eval_ctx['mutator_description'] = desc
      self.mutator_name = name
      self.mutator_desc = desc
      examples = self.query_examples()
      self.impl = self.query_implementation()
      self.impl = self.refine(self.impl, examples)
      self.eval_ctx['existing_mutators'].append((name, desc))
      self.jlog(action="log", status="finalize", name=name, desc=desc)
      return self.impl
