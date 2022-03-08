#*************************
#Copyright 2021 Mark Burgess
#
#This file is part of Dagster.
#
#Dagster is free software; you can redistribute it 
#and/or modify it under the terms of the GNU General 
#Public License as published by the Free Software 
#Foundation; either version 2 of the License, or
#(at your option) any later version.
#
#Dagster is distributed in the hope that it will be
#useful, but WITHOUT ANY WARRANTY; without even the
#implied warranty of MERCHANTABILITY or FITNESS FOR 
#A PARTICULAR PURPOSE. See the GNU General Public 
#License for more details.
#
#You should have received a copy of the GNU General 
#Public License along with Dagster.
#If not, see <http://www.gnu.org/licenses/>.
#*************************

import fire

def dumb_dag_generator(cnf_out, dag_out):
	with open(cnf_out,"r") as f:
		lines = [ff.strip() for ff in f.readlines()]
	header = None
	for l in lines:
		if l[0]=='p':
			header = l
			break
	header = header.split(" ")
	assert len(header)==4
	vc = int(header[2])
	clauses = int(header[3])
	with open(dag_out,"w") as f:
		f.write("DAG-FILE\nNODES:1\nGRAPH:\nCLAUSES:\n0:{}-{}\nREPORTING:\n{}-{}\n".format(0,clauses-1,1,vc))

if __name__ == '__main__':
        fire.Fire(dumb_dag_generator)
