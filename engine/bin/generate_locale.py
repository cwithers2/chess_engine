#!/usr/bin/env python3
macro_prefix    = "UCI_"

commands = {
	"source"  : "srv/commands.txt",
	"header"  : "include/commands.h",
	"tokens"  : "src/commands.c",
	"guard"   : "COMMANDS_H",
	"prefix"  : "UI_COMMAND_",
	"array"   : "UI_COMMANDS"
}

responses = {
	"source"  : "srv/responses.txt",
	"header"  : "include/responses.h",
	"tokens"  : "src/responses.c",
	"guard"   : "RESPONSES_H",
	"prefix"  : "UI_RESPONSE_",
	"array"   : "UI_RESPONSES"
}

def load_tokens(f):
	tokens = {"normal" : {}, "special" : {} }
	with open(f, "r") as fp:
		for line in fp:
			key, val = line.rstrip().split()
			if key[0] == '_':
				tokens["special"][key] = val
			else:
				tokens["normal"][key] = val
	return tokens

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
	for key, val in tokens["normal"].items():
		print(macro_definition(data["prefix"], key, val), file=out)
	print("//special tokens:", file=out)
	for key, val in tokens["special"].items():
		print(macro_definition(data["prefix"], key[1:], val), file=out)
	print("#endif", file=out)

def generate_tokens(data, tokens, out):
	array = data["array"]
	ts = tokens["normal"]
#	print(f"#include <{data['header']}>", file=out)
	print(f"static const char* {macro_prefix}{array}[] = ", '{', file=out)
	for key in ts:
		print(f"\t{macro_token(data['prefix'], key)},", file=out)
	print("};", file=out)
	print(f"#define {macro_prefix}{array}_COUNT {len(ts.keys())}", file=out)

def write_files(data):
	tokens = load_tokens(data["source"])
	write_data(data["header"], generate_header, data, tokens)
	write_data(data["tokens"], generate_tokens, data, tokens)
	
if __name__ == "__main__":
	write_files(commands)
	write_files(responses)

		
