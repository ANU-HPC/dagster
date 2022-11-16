

master_string = "\\node (master)	at ({0},{1})	[align=center] {{Master}};"

strings = [
"\\node (worker_group_label{0})	at ({1},{2})	[above,gray] {{Worker group {0}}};",
"\draw [gray]  ({1},{2}) -- ({3},{2}) -- ({3},{4}) -- ({1},{4}) -- cycle ;",
"\\node (strengthener{0})	at ({1},{2})	[align=center] {{Strengthener{0}}};",
"\\node (tinisat{0})		at ({1},{2})	[align=center] {{TiniSAT{0}}};",
"\\node (gnovelty{0}1)	at ({1},{2})	[align=center] {{Gnovelty{0},1}};",
"\\node (gnovelty{0}2)	at ({1},{2})	[align=center] {{Gnovelty{0},2}};",
"\\node (gnovelty{0}3)	at ({1},{2})	[align=center] {{Gnovelty{0},3}};",
"\draw [-,out=0,in=180,looseness=0.35] (master.east) to ({1},{2});",
"\draw [->,out=0,in=180,looseness=0.75] ({1},{2}) to node[above,pos=0.3]{{\\footnotesize messages}} (tinisat{0});",
"\draw [-,out=90,in=0,looseness=1.55] (tinisat{0}) to ({1},{2});",
"\draw [->,out=180,in=90,looseness=1.05] ({1},{2}) to  (master.south);",
"\draw [-,out=90,in=0,looseness=0.90] (gnovelty{0}1.south) to ({1},{2});",
"\draw [-,out=90,in=0,looseness=0.70] (gnovelty{0}2.south) to ({1},{2});",
"\draw [-,out=90,in=0,looseness=0.65] (gnovelty{0}3.south) to node[below,pos=0.5]{{\\footnotesize solutions}} ({1},{2});",
"\draw [->,out=315,in=270,looseness=1.35] (tinisat{0}) to (gnovelty{0}1.north);",
"\draw [->,out=320,in=270,looseness=1.05] (tinisat{0}) to (gnovelty{0}2.north);",
"\draw [->,out=320,in=270,looseness=0.85] (tinisat{0}) to node[above,pos=0.5]{{\\footnotesize prefix}} (gnovelty{0}3.north);",
"\draw [<->,out=270,in=90,looseness=1.55] (tinisat{0}) to node[right,pos=0.8]{{\\footnotesize clauses}} (strengthener{0}.south);"
]

master_substitution = [-8,2]
substitutions = [
[[5,1]],
[[2,1],[28,14]],
[[6,3]],
[[6,8]],
[[12,8]],
[[18,8]],
[[24,8]],
[[-1,8]],
[[-1,8]],
[[4,11]],
[[4,11]],
[[4,11]],
[[4,11]],
[[4,11]],
[],
[],
[],
[]
]


scale_factor = 0.5
i=0
process = lambda x,y: [(x)*scale_factor,(y+15*i)*scale_factor]

print(master_string.format(*process(*master_substitution)))
print("")
for i in range(3):
	process = lambda x,y: [(x)*scale_factor,(y+15*i)*scale_factor]
	for j,s in enumerate(strings):
		arr = [i+1]+sum([process(*p) for p in substitutions[j]],[])
		print(s.format(*arr))
	print("")
