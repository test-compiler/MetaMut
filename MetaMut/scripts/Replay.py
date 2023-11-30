import os
import sys
import time
import pathlib
import argparse
import datetime
import traceback
from AzureAPI import Context as AzureAPI
from FakeAPI import Context as FakeAPI
from MutGen import MutatorGenerator
from MutGen import ExceedMaxQueryTimes

def think_mutators():
  existing_mutators = []
  for i in range(100):
    logfile = f'output/logs.txt'
    api = AzureAPI()
    mg = MutatorGenerator(api, logfile)
    mg.eval_ctx['existing_mutators'].extend(existing_mutators)
    mutators = mg.query('ThinkMutator')
    existing_mutators.extend(mutators)
    with open("output/mutators.txt", "a+") as fp:
      for name, desc in mutators:
        fp.write(f"# {name}: {desc.strip()}\n")

def mainloop(existing_logfile):
  api_type = 'Poe'
  total = 0
  existing_mutators = []
  while total < 200:
    logfile = f'logs/mutator-{total}.txt'
    if os.access(logfile, os.F_OK):
      total += 1
      print(f'skipping {logfile}')
      continue

    api = FakeAPI(existing_logfile)
    mg = MutatorGenerator(api, logfile)
    mg.eval_ctx['existing_mutators'].extend(existing_mutators)

    try:
      impl = mg.run()
      filename = f"lib/mutators/{impl.name}.cpp"
      print(f"saving {impl.name} to {filename}")
      pathlib.Path(filename).write_text(impl.code)
    except KeyboardInterrupt as e:
      raise
    except Exception as e:
      # from exception
      if type(e) == ExceedMaxQueryTimes:
        existing_mutators.append([e.name, e.desc])
      # dump exception
      s  = f"======= {datetime.datetime.now()} =======\n"
      s += ''.join(traceback.format_exception(
          type(e), e, e.__traceback__))
      with open(logfile, 'a+', errors='ignore') as fp:
        fp.write(s)
      print(s)

    total += 1
    if hasattr(api, 'close'):
      api.close()

if __name__ == "__main__":
  os.system(f'mkdir -p logs output')
  os.system(f'cd output; cmake ..; make -j8')

  parser = argparse.ArgumentParser('Replay')
  parser.add_argument('-i', dest='input',
      help='specify logfile to replay')
  args = parser.parse_args()

  mainloop(args.input)
