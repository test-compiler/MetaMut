from MutGen import ParseUtils, MutatorImpl, ExecUtils
import pathlib


def parse(f):
  name_desc_str = ""
  examples_str = ""
  qa_round = 0
  with open(f) as fp:
    for line in fp:
      if line.startswith("A<<<<<<<<"):
        qa_round += 1
      elif line.startswith("Q>>>>>>>>"):
        qa_round += 1
      elif qa_round == 2:
        name_desc_str += line
      elif qa_round == 4:
        examples_str += line
  name_desc_str = name_desc_str.replace("```", "").strip()
  (name, desc), = ParseUtils.parse_description(name_desc_str)
  examples = ParseUtils.parse_example(examples_str)
  return name, desc, examples

for f in pathlib.Path('logs').glob("mutator-*.txt"):
  name, desc, examples = parse(f)
  impl = MutatorImpl(name, name, "")
  for i in range(len(examples)):
    example = examples[i]
    chkres = ExecUtils.execute(impl, example)
    rcode = chkres.res_code
    with open(f"output/logs/{name}-{i}.txt", "a+") as fp:
      fp.write(f'### CHECK {f} {i}\n')
      fp.write(f'### RCODE {rcode}\n')
      for k in chkres.__dict__:
        fp.write(f"### {k}\n{chkres.__dict__[k]}\n\n")
