# -*- coding: utf-8 -*-

import sys
import time
import re
from collections import defaultdict

'''
k = 0
try:
    buff = ''
    while True:
        buff += streamin.read(1)
        if buff.endswith('\n'):
            print(buff[:-1])
            buff = ''
            k = k + 1
except KeyboardInterrupt:
   sys.stdout.flush()
   pass
print(k)
'''

assert len(sys.argv)==3, "must supply <DAG_FILENAME>, and <STREAM FILE>"

import urwid
from urwid.widget import FLOW, FIXED, PACK, BOX, GIVEN, WEIGHT, LEFT, RIGHT, RELATIVE, TOP, BOTTOM, CLIP, RELATIVE_100
#import cgitb
import sys
#cgitb.enable(format="")






header_txt = urwid.Text(u"-------------Dagster Monitor-------------")
footer_txt = urwid.Text(u"Press '?' for help on any item")
header_padding = urwid.Padding(header_txt,'center','pack')
header_padding = urwid.AttrMap(header_padding, 'selected')

def menu_button(caption, callback):
    button = urwid.Button(caption)
    urwid.connect_signal(button, 'click', callback)
    return urwid.AttrMap(button, None, focus_map='reversed')

def sub_menu(caption, choices):
    contents = menu(caption, choices)
    def open_menu(button):
        return top.open_box(contents,caption)
    return menu_button([caption, u'...'], open_menu)

def menu(choices):
    body = []
    body.extend(choices)
    return urwid.ListBox(urwid.SimpleFocusListWalker(body))

def exit_program(button):
    raise urwid.ExitMainLoop()

def exit_program_start(button):
	raise urwid.ExitMainLoop()

def back_callback(button):
    if top.box_level > 1:
        top.original_widget = top.original_widget[0]
        top.box_level -= 1


# map of forward and reverse mappings betwee nodes
dag_map_forward = defaultdict(list)
dag_map_backward = defaultdict(list)
# set of all nodes
dag_nodes = set()

# load and store connections between dag nodes
with open(sys.argv[1], "r") as dag_file:
	dag_data = dag_file.readlines()
for data_line in dag_data:
	link_result = re.findall("^(\d+)->(\d+):", data_line)
	if len(link_result)==1:
		a = int(link_result[0][0])
		b = int(link_result[0][1])
		dag_map_forward[a].append(b)
		dag_map_backward[b].append(a)
		dag_nodes.add(a)
		dag_nodes.add(b)
dag_nodes = sorted(list(dag_nodes))

# break the nodes into connections that are accending on index, and thoes that are decending (left and right respectively)
# also sort the connections
left_nodes = []
right_nodes = []
for d in dag_nodes:
	dag_map_forward[d] = sorted(dag_map_forward[d])
	dag_map_backward[d] = sorted(dag_map_backward[d])
	for dd in dag_map_forward[d]:
		if dd>d:
			left_nodes.append(d)
			break
	for dd in dag_map_forward[d]:
		if dd<d:
			right_nodes.append(d)
			break

# load all unicode symbols to use
lines = {'vline':u'│', 'hline':u'─', 'dhline':u'═', 'ltee':u'┤', 'utee':u'┴', 'rtee':u'├', 'dtee':u'┬', 'plus':u'┼'}
sharp_corners = {'tlcorner':u'┌', 'trcorner':u'┐', 'blcorner':u'└', 'brcorner':u'┘'}
curved_corners = {'tlcorner':u'╭', 'trcorner':u'╮', 'brcorner':u'╯', 'blcorner':u'╰'}
arrows = {'larrow':u'←','rarrow':u'→','lrarrow':u'⇄'}
arrows2 = {'larrow':u'⇽','rarrow':u'⇾','lrarrow':u'⇿'}
arrows3 = {'larrow':u'↽','rarrow':u'⇀','lrarrow':u'⇌'}
fills = {'fill':' '}
lines.update(curved_corners)
lines.update(arrows3)
lines.update(fills)


# begin formatting the left connections (thoes that make accending connections) as on the left of the display
def paint_pipes(dag_nodes,left_nodes,dag_map_forward):
	s = [[]]
	for d in dag_nodes:
		to_right = False
		from_right = False
		for n in left_nodes:
			turning_down_from_right = False
			turning_right_from_above = False
			going_through_from_above = False
			
			if d==n:
				turning_down_from_right = True
				from_right = True
			for dd in dag_map_forward[n]:
				if dd >n:
					if dd>d and n<d:
						going_through_from_above = True
					if dd==d:
						turning_right_from_above = True
						to_right = True
			
			if turning_down_from_right:
				s[-1].append(1)
			elif going_through_from_above and turning_right_from_above:
				s[-1].append(11)
			elif going_through_from_above:
				s[-1].append(2)
			elif turning_right_from_above:
				s[-1].append(3)
			else:
				if from_right and to_right:
					s[-1].append(4)
				elif from_right or to_right:
					s[-1].append(5)
				else:
					s[-1].append(6)
		if to_right and from_right:
			s[-1].append(7)
		elif to_right:
			s[-1].append(8)
		elif from_right:
			s[-1].append(9)
		else:
			s[-1].append(10)
		s.append([])
	return s[:-1]


# given dag information, return a series of strings cooresponding to connected lines
def format_dag(dag_nodes,left_nodes,right_nodes,dag_map_forward):
	character_map = {1:lines['tlcorner'], 2:lines['vline'], 3:lines['blcorner'], 4:lines['dhline'], 5:lines['hline'], 6:lines['fill'], 7:lines['lrarrow'], 8:lines['rarrow'], 9:lines['larrow'], 10:lines['fill'], 11:lines['rtee']}
	reverse_character_map = {1:lines['brcorner'], 2:lines['vline'], 3:lines['trcorner'], 4:lines['dhline'], 5:lines['hline'], 6:lines['fill'], 7:lines['lrarrow'], 8:lines['larrow'], 9:lines['rarrow'], 10:lines['fill'], 11:lines['ltee']}

	p1 = paint_pipes(dag_nodes,left_nodes,dag_map_forward)
	p1 = ["".join([character_map[ppp] for ppp in pp]) for pp in p1]

	reverse_dag_nodes = dag_nodes[::-1]
	node_map = {dag_nodes[i]:reverse_dag_nodes[i] for i in range(len(dag_nodes))}
	p2 = paint_pipes(dag_nodes,[node_map[r] for r in right_nodes[::-1]], {node_map[j]:[node_map[kk] for kk in k] for j,k in dag_map_forward.items()})
	p2 = ["".join([reverse_character_map[ppp] for ppp in pp[::-1]]) for pp in p2[::-1]]

	p3 = [p1[i] +" {} ".format(i)+ p2[i] for i in range(len(p1))]
	return p3


dag_strings = format_dag(dag_nodes,left_nodes,right_nodes,dag_map_forward)

node_data = [{"workers":0,"in":0,"completed":0,"out":0,"avtime":-1,"sdtime":-1,"savtime":-1,"ssdtime":-1,"uavtime":-1,"usdtime":-1} for i in range(len(dag_nodes))]
workers = []
workers2 = []
kill_mode = False


menu_array = [
	urwid.Text('Dag FILE: {}      Stream input: {}'.format(sys.argv[1],sys.argv[2])),
	urwid.Divider(),
]

dag_node_lines = []
worker_line = urwid.Text("Workers:")
worker_line2 = urwid.Text("Master Poll:")
status_line = urwid.Text("Master Status:")
for d in dag_strings:
	dag_node_lines.append( urwid.Text(u" {}   | AWAITING DAGSTER STREAM...".format(d)) )
menu_array += dag_node_lines
menu_array += [
	urwid.Divider(),
	worker_line,
	worker_line2,
	status_line,
	urwid.Divider(),
	menu_button(u'Exit', exit_program),
]
menu_top = menu(menu_array)


def update_dag_node_data():
	for i,d in enumerate(dag_strings):
		nd = node_data[i]
		nd['workers'] = 0
		for w in workers:
			if w is None:
				continue
			if w[0]==i:
				nd['workers']+=1
		dag_node_lines[i].set_text(u" {} {} | Workers: {} | in: {}, completed: {}, out: {} | av time: {}s, p.m {}% | SAT av time: {}s, pm{}% | UNSAT av time: {}s, pm{}%  |".format(d,' ' if nd['workers']==0 else u'🡸', nd['workers'],nd['in'],nd['completed'],nd['out'],nd['avtime'],nd['sdtime'],nd['savtime'],nd['ssdtime'],nd['uavtime'],nd['usdtime']))
	s = []
	for w in workers[1:]:
		if w is None:
			s.append("node ###")
		else:
			s.append("node {}".format(w))
	worker_line.set_text("  Workers: \n{}".format("\n".join(s)))
	s = []
	for w in workers2[1:]:
		if w is None:
			s.append("node ### for ###s")
		else:
			if w[1] is None:
				s.append("node {} for ###s".format(w[0]))
			else:
				s.append("node {} for {}s".format(w[0],w[1]))
	worker_line2.set_text("M Workers: \n{}".format("\n".join(s)))
	if kill_mode:
		status_line.set_text("Master Status: In Kill Loop")


class CascadingBoxes(urwid.WidgetPlaceholder):
    max_box_levels = 4

    def __init__(self, box, caption):
        #super(CascadingBoxes, self).__init__(urwid.SolidFill(u'^'))
        super(CascadingBoxes, self).__init__(urwid.LineBox(box,caption))
        self.box_level = 0
        #self.open_box(box,caption)

    def open_box(self, box, caption):
        self.original_widget = urwid.Overlay(urwid.LineBox(box,caption),
            self.original_widget,
            align='center', width=('relative', 80),
            valign='middle', height=('relative', 80),
            min_width=24, min_height=8,
            left=self.box_level * 3,
            right=(self.max_box_levels - self.box_level - 1) * 3,
            top=self.box_level * 2,
            bottom=(self.max_box_levels - self.box_level - 1) * 2)
        self.box_level += 1


    def open_help_box(self, box, caption):
        self.original_widget = urwid.Overlay(urwid.LineBox(box,caption),
            self.original_widget,
            align='center', width=('relative', 40),
            valign='middle', height=('relative', 40),
            min_width=14, min_height=8,
            left=self.box_level * 3,
            right=(self.max_box_levels - self.box_level - 1) * 3,
            top=self.box_level * 2,
            bottom=(self.max_box_levels - self.box_level - 1) * 2)
        self.box_level += 1

    def keypress(self, size, key):
        if key == 'esc' and self.box_level > 1:
            self.original_widget = self.original_widget[0]
            self.box_level -= 1
        elif key=='?':
            self.open_help_box(urwid.Filler(urwid.Pile([urwid.Text("Help is here blah blah blah blah blah"),menu_button(u'Back', back_callback)])),"HELP for item X")
        else:
            return super(CascadingBoxes, self).keypress(size, key)

palette = [
    (None,  'light gray', 'black'),
    ('heading', 'black', 'light gray'),
    ('line', 'black', 'light gray'),
    ('options', 'dark gray', 'black'),
    ('focus heading', 'white', 'dark red'),
    ('focus line', 'black', 'dark red'),
    ('focus options', 'black', 'light gray'),
    ('selected', 'white', 'dark blue')]

top = CascadingBoxes(menu_top,"Main Menu")
console_text = urwid.Text("unparsed console messages")

t2 = urwid.AttrMap(console_text, None, focus_map='reversed')
#l = urwid.ListBox([div,t2])
l = urwid.LineBox(urwid.ListBox([t2]),"Other Console output")
pile = urwid.Pile([top,(WEIGHT,0.3,l)])
frame = urwid.Frame(body=pile,footer=footer_txt, header=header_padding)


import time
import re



streamin = open(sys.argv[2],"r")




eventloop = urwid.SelectEventLoop()

console_texts = []

#j = 0
def second_callback():
	#global j
	global kill_mode
	eventloop.alarm(0.3,second_callback)
	#j += 1
	#menu_array[-1].set_text("{} {}".format(j,time.time()))
	
	input_lines = []
	buff = ''
	while True:
		c = streamin.read(1)
		if len(c)==0:
			break
		buff += c
		if buff.endswith('\n'):
			input_lines.append(buff[:-1])
			buff = ''
	# parse the new lines and update state
	for line in input_lines:
		parsed_lines = re.findall("^.+](.+)$",line)
		for line2 in parsed_lines:
			# if a worker reports free
			worker_clear = re.match(".WORKER (\d+): finished generating new solutions, sending assignment request",line2)
			if worker_clear:
				worker = int(worker_clear.groups()[0])
				while len(workers)<=worker:
					workers.append(None)
				workers[worker] = None
				continue
			# master sends worker an assignment
			worker_assignment = re.match(".sending message to worker (\d+) message about node (\d+) assignments",line2)
			if worker_assignment:
				worker = int(worker_assignment.groups()[0])
				node_assigned = int(worker_assignment.groups()[1])
				while len(workers)<=worker:
					workers.append(None)
				workers[worker] = node_assigned
				continue
			# collect master timing information
			'''
			"MASTER: Timings -- 0:1,3:0.00spm0.00%,S0.00spm49.56%,U0.00spm0.00%, 1:3,5:0.00spm11.46%,S0.00spm36.31%,U0.00spm27.00%, 2:1,4:0.00spm0.00%,S0.00spm53.38%,U0.00spm0.00%, 3:4,5:0.00spm11.00%,S0.00spm31.83%,U0.00spm3.72%, 4:16,6:0.00spm21.54%,S0.00spm3.12%,U0.00spm27.14%, 5:1,2:0.00spm0.00%,S0.00spm50.00%,U0.00spm0.00%,"
			'''
			master_timings = re.match(".+MASTER: Timings -- ",line2)
			if master_timings:
				#pattern = re.compile(r"(\b[A-Z]+\b).(\b\d+\b)")
				pattern = re.compile(r".?(\d+):(\d+),(\d+):([\d\.\#]+)spm([\d\.\#]+)%,S([\d\.\#]+)spm([\d\.\#]+)%,U([\d\.\#]+)spm([\d\.\#]+)%")
				for match in pattern.finditer(line2):
					node = int(match.groups()[0])
					node_data[node]['in'] = match.groups()[1]
					node_data[node]['out'] = match.groups()[2]
					node_data[node]['avtime'] = match.groups()[3]
					node_data[node]['sdtime'] = match.groups()[4]
					node_data[node]['savtime'] = match.groups()[5]
					node_data[node]['ssdtime'] = match.groups()[6]
					node_data[node]['uavtime'] = match.groups()[7]
					node_data[node]['usdtime'] = match.groups()[8]
				continue
			# worker debug message
			worker_allocations = re.match(".?MASTER: message buffer -- ",line2)
			if worker_allocations:
				pattern = re.compile(r"(\d+):(\d+),([\d.]+)")
				for match in pattern.finditer(line2):
					worker = int(match.groups()[0])
					node = match.groups()[1]
					time_taken = match.groups()[2]
					while len(workers2)<=worker+1:
						workers2.append(None)
					workers2[worker+1] = (node,time_taken)
				continue
			# message does not match pattern
			if re.match(".+MASTER: generating new messages",line2):
				continue
			if re.match(".+MASTER: sending new reassignments",line2):
				continue
			if re.match(".+WORKER (\d+): waiting for assignment",line2):
				continue
			if re.match(".+MASTER: receiving from workers",line2):
				continue
			if re.match(".+MASTER: polling worker",line2):
				continue
			if re.match(".+MASTER: received MPI_TAG",line2):
				continue
			if re.match(".+MASTER: poll exit loop",line2):
				kill_mode = True
				continue
			kill_signal_match = re.match(".+WORKER (\d+): received kill signal",line2)
			if kill_signal_match:
				worker = int(kill_signal_match.groups()[0])
				workers[worker] = "Killed"
				workers2[worker] = ("Killed",None)
				continue
			if re.match(".+MASTER: terminate trigger true",line2):
				continue
			if re.match(".+MASTER: sending kill signal to",line2):
				continue
			if re.match(".+SOLUTION: message about node",line2):
				continue
			if not master_timings and not worker_assignment and not worker_clear and not worker_allocations:
				console_texts.append(line2)
				console_text.set_text("\n".join(console_texts))
		if len(parsed_lines)==0:
			console_texts.append(line)
			console_text.set_text("\n".join(console_texts))
	update_dag_node_data()

second_callback()

# enter main loop, and exit if selected
urwid.MainLoop(frame, palette, event_loop=eventloop).run()

streamin.close()

