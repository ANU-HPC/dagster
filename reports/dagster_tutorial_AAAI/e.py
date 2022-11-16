from manim import *
import math


#https://docs.manim.community/en/stable/reference/manim.mobject.svg.svg_mobject.SVGMobject.html
#https://docs.manim.community/en/stable/tutorials/quickstart.html
#https://docs.manim.community/en/stable/guides/using_text.html

Font_heading = 24
Font_text = 12
Font_text_small = 10
Font_latex1 = 20




Font_heading = 28
Font_text = 18
Font_text_small = 14
Font_latex1 = 28


class DagsterTutorial(Scene):
	def construct(self):
		heading1 = Text("Dagster: Parallel Structured Search", font_size=Font_heading)
		heading2 = Text("Mark Burgess, Charles Gretton, Josh Milthorpe,", font_size=Font_text).shift(DOWN)
		heading22 = Text("Luke Croak, Thomas Willingham, Alwen Tiu", font_size=Font_text).next_to(heading2,DOWN)
		heading3 = Text("Australian National University", font_size=Font_text).next_to(heading22,DOWN)
		self.add(heading1)
		self.add(heading2)
		self.add(heading22)
		self.add(heading3)
		#self.play(FadeIn(heading1),FadeIn(heading2))
		self.wait(5)
		self.play(FadeOut(heading1),FadeOut(heading2),FadeOut(heading22),FadeOut(heading3))
		self.remove(*[mob for mob in self.mobjects])
		
		#9s
		
		
		subtext0 = Text("Summary: Dagster is:", font_size=Font_heading)
		subtext1 = Text("- Open source distributed HPC tool for SAT and #SAT", font_size=Font_text)
		subtext2 = Text("- INPUT: CNF and decomposition into subproblems via a DAG", font_size=Font_text)
		subtext3 = Text("- subproblem solutions are resolved to solve The Problem", font_size=Font_text)
		#subtext4 = Text("   and their solutions are resolved together to solve the problem", font_size=Font_text)
		group1 = VGroup(subtext0, subtext1, subtext2, subtext3).arrange(DOWN, center=False, aligned_edge=LEFT)
		self.add(group1)
		self.play(FadeIn(group1))
		self.wait(24)
		self.play(FadeOut(group1))
		self.remove(*[mob for mob in self.mobjects])
		
		#34s
		
		heading3 = Text("Satisfiability (SAT) problems:", font_size=Font_heading)
		tex = Tex("\\begin{itemize}\n\\item A series of binary variables: $x_1,x_2,\\dots,x_n$\n\\item A set of constraints on those variables.\n\\item Solution is binary values that satisfy constraints.\\end{itemize}", font_size=Font_latex1)
		group1 = VGroup(heading3, tex).arrange(DOWN, center=False, aligned_edge=LEFT)
		self.add(group1)
		self.play(FadeIn(group1))
		self.wait(12)
		self.play(FadeOut(group1))
		self.remove(*[mob for mob in self.mobjects])
		
		#48s
		
		subtext1 = Text("SAT rendered as Conjunctive Normal Form (CNF):", font_size=Font_text)
		tex = Tex("$(x_1 \\vee \\neg x_2) \\wedge (\\neg x_2 \\vee x_3)$", font_size=Font_latex1).next_to(subtext1,DOWN)
		subtext2 = Text("Which is represented in the DIMACS CNF file:", font_size=Font_text).next_to(tex,DOWN)
		Cnf_text = Text("p cnf 3 2\n 1 -2 0\n -2 3 0", font="UbuntuMono", font_size=Font_text).next_to(subtext2,DOWN)
		self.add(subtext1)
		self.play(FadeIn(subtext1))
		self.add(tex)
		self.play(Write(tex))
		self.add(subtext2)
		self.play(FadeIn(subtext2))
		self.add(Cnf_text)
		self.play(Write(Cnf_text))
		#group1 = VGroup(subtext1, tex, subtext2, Cnf_text).arrange(DOWN, center=False, aligned_edge=LEFT).set_x(-1).set_y(-1)
		#self.add(group1)
		#self.play(Write(group1))
		self.wait(34)
		#self.play(FadeOut(group1))
		self.remove(*[mob for mob in self.mobjects])
		
		#1m26s
		
		heading1 = Text("SAT problems:", font_size=Font_heading).set_y(2)
		heading2 = Text("Puzzle solving / tiling problems:", font_size=Font_text)
		o1 = SVGMobject("./tiling1.svg", height=0.9)
		heading3 = Text("Verification / Bounded Model Checking:", font_size=Font_text)
		o2 = SVGMobject("./icon1.svg", height=0.9)
		heading4 = Text("Graph problems:", font_size=Font_text)
		o3 = SVGMobject("./graph1.svg", height=0.9)
		group1 = VGroup(heading2,o1).arrange(DOWN).next_to(heading1,DOWN)
		group2 = VGroup(heading3,o2).arrange(DOWN).next_to(group1,DOWN)
		group3 = VGroup(heading4,o3).arrange(DOWN).next_to(group2,DOWN)
		self.add(heading1)
		self.play(FadeIn(heading1))
		self.add(group1)
		self.play(FadeIn(group1))
		self.add(group2)
		self.play(FadeIn(group2))
		self.add(group3)
		self.play(FadeIn(group3))	
		self.wait(19)
		self.play(FadeOut(heading1,group1,group2,group3))
		self.remove(*[mob for mob in self.mobjects])
		
		#1m50s
		
		heading1 = Text("SAT Decomposition:", font_size=Font_heading).set_y(2)
		heading2 = Text("Logically Separate Parts:", font_size=Font_text)
		o1 = SVGMobject("./puzzle1.svg", height=1)
		heading3 = Text("Prefered Solving order:", font_size=Font_text)
		o2 = SVGMobject("./plan.svg", height=1)
		group1 = VGroup(heading2,o1).arrange(DOWN).next_to(heading1,DOWN)
		group2 = VGroup(heading3,o2).arrange(DOWN).next_to(group1,DOWN)
		self.add(heading1)
		self.play(FadeIn(heading1))
		self.add(group1)
		self.play(FadeIn(group1))		
		self.add(group2)
		self.play(FadeIn(group2))		
		self.wait(29)
		self.play(FadeOut(heading1,group1,group2))
		self.remove(*[mob for mob in self.mobjects])
		
		#2m23s
		
		tex = Tex('''
\-\hspace{1cm}\\\\
\\begin{figure}[]
\centering
\\begin{minipage}{0.25\\textwidth}
\centering
\\begin{Verbatim}[frame=single, label=CNF file]
p cnf 7 7
2 1 0
-2 3 0
5 -4 0
-3 0
4 3 0
4 -6 7 0
-4 6 0
\end{Verbatim}
\end{minipage}
\\hspace{2cm}
\\begin{minipage}{0.25\\textwidth}
\centering
\\begin{Verbatim}[frame=single, label=DAG file]
DAG-FILE
NODES:4
GRAPH:
0->2:1,3
1->2:3,4
2->3:4
CLAUSES:
0:0,1
1:2,3
2:4
3:5,6
REPORTING:
4,6,7
\end{Verbatim}
\end{minipage}
\end{figure}
\\vspace{3cm}''', font_size=26)
		self.add(tex)
		self.play(Write(tex))		
		self.wait(148)
		self.play(FadeOut(tex))
		self.remove(*[mob for mob in self.mobjects])
		
		#demo insert
		
		# 4m53s

		heading1 = Text("A Demonstration Case Study:", font_size=Font_heading)
		heading2 = Text("Adjacency matrices with largest determinant", font_size=Font_text)
		group1 = VGroup(heading1,heading2).arrange(DOWN, center=False, aligned_edge=LEFT)
		self.add(group1)
		self.play(FadeIn(group1))		
		self.wait(11)
		self.play(FadeOut(group1))
		
		#5m6s
		
		graph_dots = [Dot([1.3*math.sin(r*math.pi/180)-2.5, 1.3*math.cos(r*math.pi/180)+1.3, 0]) for r in range(0,360,40)]
		graph_lines = []
		for i in range(len(graph_dots)):
			for j in range(i+1,len(graph_dots)):
				if j%2==0 and i%3==0:
					graph_lines.append(Line(graph_dots[i].get_center(), graph_dots[j].get_center()).set_color(ORANGE))
		#group = Group(graph_dots+graph_lines)
		#self.add(group)
		for d in graph_dots:
			self.add(d)
			#self.play(Write(d))
		for l in graph_lines:
			self.add(l)
			self.play(Write(l,run_time=0.2))
		tex = Tex('''\\begin{figure}[]
\\centering
A=\\begin{equation*}\\begin{pNiceMatrix}[margin]
0 & 0 & 1 & 0 & 1 & 0 & 1 & 0 & 1  \\\\
0 & 0 & 0 & 0 & 0 & 0 & 0 & 0 & 0  \\\\
1 & 0 & 0 & 0 & 0 & 0 & 0 & 0 & 0  \\\\
0 & 0 & 0 & 0 & 1 & 0 & 1 & 0 & 1  \\\\
1 & 0 & 0 & 1 & 0 & 0 & 0 & 0 & 0  \\\\
0 & 0 & 0 & 0 & 0 & 0 & 0 & 0 & 0  \\\\
1 & 0 & 0 & 1 & 0 & 0 & 0 & 0 & 1  \\\\
0 & 0 & 0 & 0 & 0 & 0 & 0 & 0 & 0  \\\\
1 & 0 & 0 & 1 & 0 & 0 & 1 & 0 & 0  \\\\
\\end{pNiceMatrix}\\end{equation*}
\\end{figure}''', font_size=20).set_x(1.5).set_y(+1.3)
		self.add(tex)
		self.play(Write(tex))
		heading1 = Tex("What graphs has maximal $\det(A)$?", font_size=Font_latex1).set_y(-1.5)
		heading2 = Tex("Similar to Hadamard's maximal determinant problem", font_size=Font_latex1).next_to(heading1,DOWN)
		heading3 = Tex("Unanswered problem: \\url{https://mathoverflow.net/questions/386168/which-graphs-on-n-vertices-have-the-largest-determinant}", font_size=Font_latex1).next_to(heading2,DOWN)
		self.add(heading1)
		self.play(FadeIn(heading1))
		self.add(heading2)
		self.play(FadeIn(heading2))
		self.add(heading3)
		self.play(FadeIn(heading3))
		#self.play(Write(heading1),Write(heading1),Write(heading1))
		self.wait(25)
		self.remove(*[mob for mob in self.mobjects])
		
		#5m38s
		
		heading1 = Text("Approach using #SAT", font_size=Font_heading).set_x(-2).set_y(1)
		heading2 = Text("Unfortunately, determinant is involved expression", font_size=Font_text)
		heading3 = Text("Solve for weaker condition, of locally maximal determinant", font_size=Font_text)
		heading4 = Text(" to any 0-1 swap in A, consider, matrix determinant lemma:", font_size=Font_text)
		group1 = VGroup(heading1,heading2,heading3,heading4).arrange(DOWN, center=False, aligned_edge=LEFT)
		tex = Tex("$ \det(A+uv^T)=(1+v^TA^{-1}u)\det(A) $", font_size=Font_latex1).next_to(group1,DOWN)
		self.add(group1)
		self.play(FadeIn(group1))	
		self.add(tex)
		self.play(Write(tex))	
		self.wait(57)
		self.play(FadeOut(group1))
		self.remove(*[mob for mob in self.mobjects])
		
		#6m38s
		
		tex = Tex("$\\text{Matrix Determinant Lemma:}~~~\det(A+uv^T)=(1+v^TA^{-1}u)\det(A) $", font_size=Font_latex1).set_y(1)
		tex2 = Tex("$\\text{Consider:}~~u=[0,0,1,0]^T=e_i~~~v=[0,1,0,0]^T=e_j$", font_size=Font_latex1).next_to(tex,DOWN)
		tex3 = Tex('''\\begin{equation*}\\text{Then:}~~~uv^T=e_{i,j}~~~\\text{and:}~~v^TA^{-1}u=A^{-1}_{i,j}\\end{equation*}''', font_size=Font_latex1).next_to(tex2,DOWN)
		tex4 = Tex("$\\text{Thus:}~~~ \det(A\pm e_{i,j})=(1\pm A^{-1}_{i,j})\det(A) $", font_size=Font_latex1).next_to(tex3,DOWN)
		framebox1 = SurroundingRectangle(tex4,buff=0.1)
		#group1 = VGroup(tex,tex2,tex3).arrange(DOWN, center=False, aligned_edge=LEFT)
		#self.add(group1)
		#self.play(FadeIn(group1))		
		self.add(tex)
		self.play(Write(tex))
		self.add(tex2)
		self.play(Write(tex2))
		self.add(tex3)
		self.play(Write(tex3))
		self.add(tex4)
		self.play(Write(tex4))
		self.play(Create(framebox1))
		self.wait(27)
		self.play(FadeOut(tex))
		self.play(FadeOut(tex2))
		self.play(FadeOut(tex3))
		self.play(FadeOut(tex4),FadeOut(framebox1))
		#self.play(FadeOut(group1))
		self.remove(*[mob for mob in self.mobjects])
		
		#7m14
		
		heading24 = Tex("For determinant to be maximal", font_size=Font_heading)
		heading241 = Tex("if $A_{i,j}$ is zero, then $A^{-1}_{i,j}$ is non-positive", font_size=Font_latex1).next_to(heading24,DOWN)
		heading25 = Tex("if $A_{i,j}$ is one, then $A^{-1}_{i,j}$ is non-negative", font_size=Font_latex1).next_to(heading241,DOWN)
		heading3 = Text("(Note: because adjacency matrix flips pairs of elements, is more complicated (need Sherman–Morrison formula), but same conclusion holds)", font_size=Font_text_small).next_to(heading25,DOWN)
		self.add(heading24)
		self.play(FadeIn(heading24))
		self.add(heading241)
		self.play(FadeIn(heading241))
		self.add(heading25)
		self.play(FadeIn(heading25))
		self.add(heading3)
		self.play(FadeIn(heading3))
		self.wait(19)
		self.play(FadeOut(heading24,heading241,heading25,heading3))
		self.remove(*[mob for mob in self.mobjects])
		
		#7m38s
		
		tex2 = Tex("Inverse defined normally $~~~AA^{-1}=I$", font_size=Font_latex1)
		tex3 = Tex("$\\text{Thus:}~~~~A \\cdot\\text{adj}(A)=\det(A)I$", font_size=Font_latex1).next_to(tex2,DOWN)
		heading1 = Text("assuming the determinant is unknown but positive", font_size=Font_text).next_to(tex3,DOWN)
		tex4 = Tex("Assert $AB=aI$ for positive $a$.", font_size=Font_latex1).next_to(heading1,DOWN)
		self.add(tex2)
		self.play(FadeIn(tex2))
		self.add(tex3)
		self.play(FadeIn(tex3))
		self.add(heading1)
		self.play(FadeIn(heading1))
		self.add(tex4)
		self.play(FadeIn(tex4))
		self.wait(25)
		self.play(FadeOut(tex2,tex3,heading1,tex4))
		self.remove(*[mob for mob in self.mobjects])
		
		#8m8s
		
		tex2 = Tex("We have, Inverse relationship $AB=aI$, A is binary, B is integer, and elementwise relation between A and B", font_size=Font_latex1)
		tex3 = Tex("We symmetry break on vertex enumeration, and thereby have our model counting problem", font_size=Font_latex1).next_to(tex2,DOWN)
		heading1 = Text("We break the problem into two parts:", font_size=Font_text).next_to(tex3,DOWN)
		self.add(tex2)
		self.play(FadeIn(tex2))
		self.add(tex3)
		self.play(FadeIn(tex3))
		self.add(heading1)
		self.play(FadeIn(heading1))
		self.wait(17)
		self.play(FadeOut(tex2,tex3,tex3,heading1))
		self.remove(*[mob for mob in self.mobjects])
		
		#8m29s
		
		
		w = 7
		initial_matrix = [[0,0,0,0,0,r'\_',r'\_'],
[0,0,0,0,0,r'\_',r'\_'],
[0,0,0,0,0,r'\_',r'\_'],
[0,0,0,0,0,r'\_',r'\_'],
[0,0,0,0,0,r'\_',r'\_'],
[r'\_',r'\_',r'\_',r'\_',r'\_',r'\_',r'\_'],
[r'\_',r'\_',r'\_',r'\_',r'\_',r'\_',r'\_']]
		matrix2 = Matrix(initial_matrix)
		ent = matrix2.get_entries()
		odd_entries = [ent[0]]+[ent[4*w+4]]
		upper_left = VGroup(*odd_entries)
		matrix2.add(SurroundingRectangle(upper_left))
		
		initial_matrix = [[0,1,0,1,0,r'\_',r'\_'],
[1,0,0,1,0,r'\_',r'\_'],
[0,0,0,0,0,r'\_',r'\_'],
[1,1,0,0,1,r'\_',r'\_'],
[0,0,0,1,0,r'\_',r'\_'],
[r'\_',r'\_',r'\_',r'\_',r'\_',r'\_',r'\_'],
[r'\_',r'\_',r'\_',r'\_',r'\_',r'\_',r'\_']]
		matrix3 = Matrix(initial_matrix)
		ent = matrix3.get_entries()
		odd_entries = [ent[0]]+[ent[4*w+4]]
		upper_left = VGroup(*odd_entries)
		matrix3.add(SurroundingRectangle(upper_left))
		
		initial_matrix = [[0,1,0,1,0,1,0],
[1,0,0,1,0,0,1],
[0,0,0,0,0,1,1],
[1,1,0,0,1,0,0],
[0,0,0,1,0,0,0],
[1,0,1,0,0,0,1],
[0,1,1,0,0,1,0]]
		matrix4 = Matrix(initial_matrix)
		ent = matrix4.get_entries()
		odd_entries = [ent[0]]+[ent[4*w+4]]
		upper_left = VGroup(*odd_entries)
		matrix4.add(SurroundingRectangle(upper_left))
		
		initial_matrix = [[0,1,1,0,1,r'\_',r'\_'],
[1,0,0,0,0,r'\_',r'\_'],
[1,0,0,1,1,r'\_',r'\_'],
[0,0,1,0,1,r'\_',r'\_'],
[1,0,1,1,0,r'\_',r'\_'],
[r'\_',r'\_',r'\_',r'\_',r'\_',r'\_',r'\_'],
[r'\_',r'\_',r'\_',r'\_',r'\_',r'\_',r'\_']]
		matrix5 = Matrix(initial_matrix)
		ent = matrix5.get_entries()
		odd_entries = [ent[0]]+[ent[4*w+4]]
		upper_left = VGroup(*odd_entries)
		matrix5.add(SurroundingRectangle(upper_left))

		initial_matrix = [[0,1,1,0,1,1,0],
[1,0,0,0,0,0,1],
[1,0,0,1,1,1,1],
[0,0,1,0,1,0,0],
[1,0,1,1,0,0,0],
[1,0,1,0,0,0,1],
[0,1,1,0,0,1,0]]
		matrix6 = Matrix(initial_matrix)
		ent = matrix6.get_entries()
		odd_entries = [ent[0]]+[ent[4*w+4]]
		upper_left = VGroup(*odd_entries)
		matrix6.add(SurroundingRectangle(upper_left))
		
		self.add(matrix2)
		self.play(FadeIn(matrix2))
		self.wait(5.4)
		self.play(FadeOut(matrix2), FadeIn(matrix3))
		self.wait(5.4)
		self.play(FadeOut(matrix3), FadeIn(matrix4))
		self.wait(5.4)
		self.play(FadeOut(matrix4), FadeIn(matrix5))
		self.wait(5.4)
		self.play(FadeOut(matrix5), FadeIn(matrix6))
		self.wait(5.4)
		self.play(FadeOut(matrix6))
		self.remove(*[mob for mob in self.mobjects])
		
		#9m02s
		
		
		tex = Tex('''
\\begin{minipage}{0.5\\textwidth}
\centering
\\begin{Verbatim}[frame=single, label=DAG file]
DAG-FILE
NODES:2
GRAPH:
0->1:1-25
CLAUSES:
0:17832-17836,17841-17843,17848
1:0-18885
REPORTING:
1-25
\\end{Verbatim}
\\end{minipage}''', font_size=28)
		self.add(tex)
		self.play(FadeIn(tex))		
		self.wait(32)
		self.play(FadeOut(tex))
		self.remove(*[mob for mob in self.mobjects])
		
		#graph of results
		
		#9m36
		
		heading2 = Text("Full Publication accepted into PRICAI 2022 conference",font_size=Font_text)
		subtext0 = Text("Featuring other case studies:", font_size=Font_text)
		subtext1 = Text("- Pentomino Tiling problems", font_size=Font_text)
		subtext2 = Text("- Costas Array Counting", font_size=Font_text)
		subtext3 = Text("- Bounded Model Checking", font_size=Font_text)
		group1 = VGroup(subtext0, subtext1, subtext2, subtext3).arrange(DOWN, center=False, aligned_edge=LEFT).next_to(heading2,DOWN)
		heading1 = Tex("Source Code: \\url{https://github.com/ANU-HPC/dagster}", font_size=Font_latex1).next_to(group1,DOWN)
		
		self.add(heading2)
		self.play(FadeIn(heading2))		
		self.add(group1)
		self.play(FadeIn(group1))	
		self.add(heading1)
		self.play(FadeIn(heading1))		
		self.wait(18)
		self.play(FadeOut(heading2,group1,heading1))
		self.remove(*[mob for mob in self.mobjects])
		
		#9m58s
		
		
		heading2 = Text("Thankyou",font_size=Font_heading)
		heading3 = Text("mark.burgess@anu.edu.au",font_size=Font_text).next_to(heading2,DOWN)
		
		self.add(heading2)
		self.play(FadeIn(heading2),FadeIn(heading3))
		self.wait(2)
		#self.play(FadeOut(heading2),FadeOut(heading3))
		self.remove(*[mob for mob in self.mobjects])
		
		# 13:05
		
		#self.play(FadeIn(SVGMobject("./a.svg")))
		#circle = Circle()  # create a circle
		#circle.set_fill(PINK, opacity=0.5)  # set the color and transparency
		#self.play(Create(circle))  # show the circle on screen
		
		
		
		
		
