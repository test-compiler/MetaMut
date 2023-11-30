import os
import sys
import time
import pathlib
import datetime
import traceback
from AzureAPI import Context as AzureAPI
from FakeAPI import Context as FakeAPI
from MutGen import MutatorGenerator
from MutGen import ExceedMaxQueryTimes

def check_logfile_exists(logfile):
  if os.access(logfile, os.F_OK):
    print(f'skipping {logfile}')
    return True
  return False

def create_api_and_mutator_generator(ApiCtor, logfile, existing_mutators):
  api = ApiCtor()
  mg = MutatorGenerator(api, logfile)
  mg.eval_ctx['existing_mutators'].extend(existing_mutators)
  return api, mg

def clean_workbench():
  os.system(f"rm -rf lib/workbench/*")

def run_mutator_generator(mg):
  try:
    impl = mg.run()
  except KeyboardInterrupt as e:
    raise
  except Exception as e:
    return e
  return impl

def write_to_file(impl):
  filename = f"lib/mutators/{impl.name}.cpp"
  print(f"saving {impl.name} to {filename}")
  pathlib.Path(filename).write_text(impl.code)

def handle_exception(e, logfile, existing_mutators):
  if type(e) == ExceedMaxQueryTimes:
    name = e.__dict__.get('name', None)
    desc = e.__dict__.get('desc', None)
    if name:
      existing_mutators.append([name, str(desc)])

  # dump exception
  s  = f"======= {datetime.datetime.now()} =======\n"
  s += ''.join(traceback.format_exception(type(e), e, e.__traceback__))
  with open(logfile, 'a+', errors='ignore') as fp:
    fp.write(s)
  print(s)

def close_api(api):
  if hasattr(api, 'close'):
    api.close()

def mainloop(ApiCtor, maxIter):
  total = 0
  existing_mutators = []
  while total < maxIter:
    logfile = f'logs/mutator-{total}.txt'
    if check_logfile_exists(logfile):
      total += 1
      continue

    api, mg = create_api_and_mutator_generator(ApiCtor, logfile, existing_mutators)
    clean_workbench()

    impl = run_mutator_generator(mg)
    if isinstance(impl, Exception):
      handle_exception(impl, logfile, existing_mutators)
    else:
      write_to_file(impl)

    total += 1
    close_api(api)

if __name__ == "__main__":
  os.system(f'mkdir -p logs output')
  os.system(f'cd output; cmake ..; make -j8')
  os.system(f'mkdir -p lib/workbench lib/mutators')
  # fake_run()
  # think_mutators()
  # mainloop(lambda : FakeAPI('logs/mutator-1.txt'), 8)
  mainloop(lambda:AzureAPI(), 200)
