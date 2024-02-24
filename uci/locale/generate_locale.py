#!/usr/bin/env python3
macro_prefix    = "UCI_"

commands = {
	"source"  : "commands.txt",
	"header"  : "locale/commands.h",
	"tokens"  : "src/commands.h",
	"guard"   : "COMMANDS_H",
	"prefix"  : "UI_COMMAND_",
	"array"   : "UI_COMMANDS"
}

responses = {
	"source"  : "responses.txt",
	"header"  : "locale/responses.h",
	"tokens"  : "src/responses.h",
	"guard"   : "RESPONSES_H",
	"prefix"  : "UI_RESPONSE_",
	"array"   : "UI_RESPONSES"
}

def load_data(f):
	data = {}
	with open(f, "r") as fp:
		for line in fp:
			key, val = line.rstrip().split()
			data[key] = val
	return data

def write_data(filename, function, data, tokens):
	with open(filename, "+w") as f:
		function(data, tokens, f)

def macro_token(prefix, key):
	return f"{macro_prefix}{prefix}{key}"
	
def macro_definition(prefix, key, val):
	return f'#define {macro_token(prefix, key):<35s}"{val}"'

def generate_header(data, tokens, out):
	print(f"#ifndef {data['guard']}", file=out)
	print(f"#define {data['guard']}", file=out)
	for key, val in tokens.items():
		print(macro_definition(data["prefix"], key, val), file=out)
	print("#endif", file=out)

def generate_tokens(data, tokens, out):
	array = data["array"]
	print(f"#include <{data['header']}>", file=out)
	print(f"static const char* {macro_prefix}{array}[] = ", '{', file=out)
	for key in tokens:
		print(f"\t{macro_token(data['prefix'], key)},", file=out)
	print("};", file=out)
	print(f"#define {macro_prefix}{array}_COUNT {len(tokens.keys())}", file=out)

def write_files(data):
	tokens = load_data(data["source"])
	write_data(data["header"], generate_header, data, tokens)
	write_data(data["tokens"], generate_tokens, data, tokens)
	
if __name__ == "__main__":
	write_files(commands)
	write_files(responses)

		
