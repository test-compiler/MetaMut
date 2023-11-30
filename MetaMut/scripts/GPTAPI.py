import openai
import configs
import pathlib
import tiktoken

def num_tokens_from_messages(messages, model="gpt-3.5-turbo-0613"):
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
    "gpt-4-32k-0613",
    }:
    tokens_per_message = 3
    tokens_per_name = 1
  elif model == "gpt-3.5-turbo-0301":
    tokens_per_message = 4  # every message follows <|start|>{role/name}\n{content}<|end|>\n
    tokens_per_name = -1  # if there's a name, the role is omitted
  elif "gpt-3.5-turbo" in model:
    print("Warning: gpt-3.5-turbo may update over time. Returning num tokens assuming gpt-3.5-turbo-0613.")
    return num_tokens_from_messages(messages, model="gpt-3.5-turbo-0613")
  elif "gpt-4" in model:
    print("Warning: gpt-4 may update over time. Returning num tokens assuming gpt-4-0613.")
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
  def __init__(self):
    self.system_prompt = {"role": "system", "content": "You are an AI programming assistant."}
    self.conversation = [ ]
    self.model = "gpt-3.5-turbo-16k"
    self.curr_completion = None
    openai.api_key = configs.openai_api_key
  def send(self, text):
    request = { "role": "user", "content": text }

    # drop previous tokens
    while True:
      messages = [ self.system_prompt,
          *self.conversation, request ]
      remain_tokens = 16385 - num_tokens_from_messages(
          messages, self.model)
      if remain_tokens >= 512: break
      # print(f'drop history {self.conversation[0]}')
      self.conversation.pop(0)

    self.completion = openai.ChatCompletion.create(
      model = "gpt-3.5-turbo-16k",
      temperature = 0.8,
      max_tokens = remain_tokens,
      messages = messages
    )

    self.conversation.append(request)
    response = self.completion.choices[0].message.to_dict()
    self.conversation.append(response)
    # print(self.conversation)
    return response['content']
