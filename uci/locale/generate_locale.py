#!/usr/bin/env python3
macro_prefix    = "UCI_"

commands_source = "./commands.txt"
commands_target = "./locale/commands.h"
commands_prefix = "UI_COMMAND_"
commands_array  = "UI_COMMANDS"

responses_source = "./responses.txt"
responses_target = "./locale/responses.h"
responses_prefix = "UI_RESPONSE_"
responses_array  = "UI_RESPONSES"

def load_data(f):
	data = {}
	with open(f, "r") as fp:
		for line in fp:
			key, val = line.rstrip().split()
			data[key] = val
	return data

def macro_token(prefix, key):
	return f"{macro_prefix}{prefix}{key}"
	
def macro_definition(prefix, key, val):
	return f'#define {macro_token(prefix, key):<35s}"{val}"'

def generate_file(tokens, prefix, array, out):
	print("// * auto generated * Private file * Do not #include *", file=out)
	for key, val in tokens.items():
		print(macro_definition(prefix, key, val), file=out)
	print(f"\nstatic const char* {macro_prefix}{array}[] = ", '{', file=out)
	for key in tokens:
		print(f"\t{macro_token(prefix,key)},", file=out)
	print("};\n", file=out)
	print(f"#define {macro_prefix}{array}_COUNT {len(tokens.keys())}",file=out)
			
if __name__ == "__main__":
	commands = load_data(commands_source)
	with open(commands_target, "+w") as f:
		generate_file(commands, commands_prefix, commands_array, f)
	responses = load_data(responses_source)
	with open(responses_target, "+w") as f:
		generate_file(responses, responses_prefix, responses_array, f)

		
