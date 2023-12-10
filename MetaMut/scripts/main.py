import os
import sys
import time
import openai
import pathlib
import argparse
from metamut.Driver import MetaMutDriver
from llm.ReplayAPI import Context as ReplayAPI

def parse_args():
  parser = argparse.ArgumentParser('MetaMut.driver')
  parser.add_argument('--replay-logs',
      dest='replay_logs', nargs='+',
      help='Replay the generation process from the specified log files.')
  parser.add_argument('--print-model-list',
      dest='print_model_list', action='store_true',
      help='Print a list of available models and exit.')
  parser.add_argument('--invent-only',
      dest='invent_only', action='store_true',
      help='Invent new mutators without generating them.')
  parser.add_argument('--num-mutators', dest='num_mutators',
      type=int, default=100,
      help='specify the number of mutators to be generated or invented (default: 100)')
  return parser.parse_args()

def main(args):
  if args.print_model_list:
    print(openai.Model.list())
    sys.exit(0)

  driver = MetaMutDriver()
  if args.invent_only:
    if args.replay_logs:
      for logfile in args.replay_logs:
        driver.invent_mutators(n=1, api=ReplayAPI(logfile))
    else:
      driver.invent_mutators(n=args.num_mutators)
  elif args.replay_logs:
    driver.replay_logs(args.replay_logs)
  else:
    driver.generate_mutators(n=args.num_mutators)

if __name__ == "__main__":
  main(parse_args())
