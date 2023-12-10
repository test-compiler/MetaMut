import re
import time
import tiktoken
import openai
import traceback
import random
import configs

def num_max_tokens(model):
  r = re.search('(\d+)k', model, re.I)
  if r: return int(r.group(1)) * 1024 + 1
  if re.search('gpt-4', model, re.I):
    return 8*1024 + 1
  if re.search('gpt-3.5', model, re.I) or \
     re.search('gpt-35', model, re.I):
    return 4*1024 + 1
  raise ValueError(f"invalid model {model}")

def num_tokens_from_messages(messages, model="gpt-3.5-turbo-0613"):
  model = model.replace('gpt-35', 'gpt-3.5')
  """Return the number of tokens used by a list of messages."""
  try:
    encoding = tiktoken.encoding_for_model(model)
  except KeyError:
    print("Warning: model not found. Using cl100k_base encoding.")
    encoding = tiktoken.get_encoding("cl100k_base")
  if model in {
    "gpt-3.5-turbo-0613",
    "gpt-3.5-turbo-16k-0613",
    "gpt-4-0314",
    "gpt-4-32k-0314",
    "gpt-4-0613",
    "gpt-4-32k-0613", }:
    tokens_per_message = 3
    tokens_per_name = 1
  elif model == "gpt-3.5-turbo-0301":
    tokens_per_message = 4  # every message follows <|start|>{role/name}\n{content}<|end|>\n
    tokens_per_name = -1  # if there's a name, the role is omitted
  elif "gpt-3.5-turbo" in model:
    # print("Warning: gpt-3.5-turbo may update over time. Returning num tokens assuming gpt-3.5-turbo-0613.")
    return num_tokens_from_messages(messages, model="gpt-3.5-turbo-0613")
  elif "gpt-4" in model:
    # print("Warning: gpt-4 may update over time. Returning num tokens assuming gpt-4-0613.")
    return num_tokens_from_messages(messages, model="gpt-4-0613")
  else:
    raise NotImplementedError(
      f"""num_tokens_from_messages() is not implemented for model {model}. See https://github.com/openai/openai-python/blob/main/chatml.md for information on how messages are converted to tokens."""
    )
  num_tokens = 0
  for message in messages:
    num_tokens += tokens_per_message
    for key, value in message.items():
      num_tokens += len(encoding.encode(value))
      if key == "name":
        num_tokens += tokens_per_name
  num_tokens += 3  # every reply is primed with <|start|>assistant<|message|>
  return num_tokens

class Context:
  def __init__(self, engine='gpt-4', top_p=0.95, temperature=1.0):
    self.system_prompt = {"role": "system",
        "content": "You are an AI programming assistant."}
    self.conversation = [ ]
    self.engine = engine
    self.top_p = top_p
    self.temperature = temperature
    self.curr_completion = None
  def send(self, text):
    while True:
      try:
        return self.send_internal(text)
      except KeyboardInterrupt:
        print("Interrupted by Ctrl+C. stopping the program...")
        raise
      except (openai.error.RateLimitError, openai.error.InvalidRequestError) as e:
        s = ''.join(traceback.format_exception(
            type(e), e, e.__traceback__))
        print(s)
        time.sleep(21 + random.random() * 8)
        continue
      else:
        break
  def send_internal(self, text):
    request = { "role": "user", "content": text }

    # drop previous tokens
    while True:
      messages = [ self.system_prompt,
          *self.conversation, request ]
      remain_tokens = num_max_tokens(self.engine) - \
          num_tokens_from_messages(messages, self.engine)
      if remain_tokens >= 1024: break
      # print(f'drop history {self.conversation[0]}')
      self.conversation.pop(0)

    self.completion = openai.ChatCompletion.create(
      engine=self.engine,
      max_tokens=remain_tokens,
      messages=messages,
      temperature=self.temperature,
      top_p=self.top_p,
      frequency_penalty=0,
      presence_penalty=0,
      stop=None)

    self.conversation.append(request)
    response = self.completion.choices[0].message.to_dict()
    self.conversation.append(response)
    # print(self.conversation)
    return response['content']
