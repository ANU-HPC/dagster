import click
from tqdm import tqdm
import json
import re

@click.command()
@click.argument('input_cnf', type=click.File('r'))
@click.argument('json_config', type=click.File('r'))
@click.argument('output_cnf', type=click.File('w'))
def cnf_splitter(input_cnf,json_config,output_cnf):
	config = json.load(json_config)
	
	input_cnf.seek(0,2) # move the cursor to the end of the file
	size = input_cnf.tell()
	input_cnf.seek(0)
	
	pbar = tqdm(total=size)
	line = input_cnf.readline()
	while line!='':
		pbar.update(len(line))
		pattern = re.findall("{([^}]+)}",line)
		if pattern:
			for g in pattern:
				line = line.replace("{{{}}}".format(g), str(eval(g,config)))  ## WARNING: EVAL is cheap.
		output_cnf.write(line)
		line = input_cnf.readline()
	input_cnf.close()
	print("DONE")


if __name__ == '__main__':
	cnf_splitter()


