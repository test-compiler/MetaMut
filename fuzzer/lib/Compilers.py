import os
import time
import random
import shutil
import configs
import pathlib
import threading
import subprocess
from .ShellProcess import ShellProcess
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

class Compiler:
  def __init__(self, name, aflpp_path, aflpp_envs,
      myafl_path, myafl_nbrs):
    self.name = name
    self.aflpp_path = aflpp_path
    self.aflpp_envs = aflpp_envs
    self.myafl_path = myafl_path
    self.myafl_nbrs = myafl_nbrs
  def __str__(self):
    return self.name

def make_compilers():
  return [
      Compiler('clang',
        configs.aflpp_clang_bin,
        configs.aflpp_clang_envs,
        configs.myafl_clang_bin,
        configs.myafl_clang_nbrs),
      Compiler('gcc',
        configs.aflpp_gcc_bin,
        configs.aflpp_gcc_envs,
        configs.myafl_gcc_bin,
        configs.myafl_gcc_nbrs),
  ]
