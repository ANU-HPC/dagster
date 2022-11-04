import click
from tqdm import tqdm

@click.command()
@click.argument('input1', type=click.File('r'))
def cnf_splitter(input1):
	line = input1.readline()
	assert line[0]=='p'
	count = 0
	f = None
	while line!='':
		if line[0]=='p':
			if f is not None:
				f.close()
			f = open("cnf_part{}.cnf".format(count),'w')
			count += 1
		f.write(line)
		line = input1.readline()
	f.close()
	print("DONE")


if __name__ == '__main__':
	cnf_splitter()


