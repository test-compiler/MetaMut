import openai

objdir = 'output'
mutator_bin = f'{objdir}/LLMut'

# please change the api_base and api_key to proper values. Different MSR labs are provided with different values.
openai.api_type = "azure"
openai.api_base = "https://promptengineer.openai.azure.com/"
openai.api_version ="2023-07-01-preview"
openai.api_key ="XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
