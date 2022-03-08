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
import os

def system_wrap(c):
        print(c)
        os.system(c)

def make_unique(f_in, f_out):
	with open(f_in,"r") as f:
		lines = [ff.strip() for ff in f.readlines()]
	lines = [l for l in lines if l[0] in 'p123456789-']
	with open(f_out,"w") as f:
		for l in lines:
			f.write("{}\n".format(l))
	while True:
		system_wrap("{} {} ./TEMP_FILE.txt".format(os.environ.get('TINISAT_LOCATION',"../dagster/standalone_tinisat/tinisat"),f_out))
		try:
			with open("./TEMP_FILE.txt","r") as ff:
				data = ff.readlines()
			assert(len(data)==2)
			sol = " ".join([str(-int(d)) for d in data[1].strip().split(" ")])
			with open(f_out,"r") as ff:
				line0 = ff.readline().strip().split(" ")
			line0[3] = str(int(line0[3])+1)
			line0 = " ".join(line0)
			with open(f_out,"a") as ff:
				ff.write("{}\n".format(sol))
			system_wrap("sed -i '1s/.*/{}/' {}".format(line0,f_out))
			system_wrap("rm ./TEMP_FILE.txt")
		except Exception as e:
			print(e)
			break
	with open(f_out,"r") as ff:
		line0 = ff.readline().strip().split(" ")
	line0[3] = str(int(line0[3])-1)
	line0 = " ".join(line0)
	system_wrap("sed -i '1s/.*/{}/' {}".format(line0,f_out))
	system_wrap("sed -i '$ d' {}".format(f_out))


if __name__ == '__main__':
        fire.Fire(make_unique)


