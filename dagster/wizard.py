import urwid
import sys
#import cgitb
#cgitb.enable(format="")
import json



from collections import defaultdict
class Elements(object):
	def __init__(self):
		self.e = defaultdict(list)
		self.call = {}
	def __call__(self,element_type,args,char='',additional_attributes = None):
		n = element_type(*args)
		if additional_attributes is not None:
			for k in additional_attributes.keys():
				setattr(n,k,additional_attributes[k])
		self.e[element_type.__name__].append([n,char])
		return n
	def register(self,e,call):
		self.call[call] = e
		return e
E = Elements()



header_txt = urwid.Text(u"-------------Dagster Startup Utility-------------")
footer_txt = urwid.Text(u"Press '?' for help on any item")
header_padding = urwid.Padding(header_txt,'center','pack')
header_padding = urwid.AttrMap(header_padding, 'selected')

def menu_button(caption, callback):
	button = E(urwid.Button,[caption])
	urwid.connect_signal(button, 'click', callback)
	return urwid.AttrMap(button, None, focus_map='reversed')

def sub_menu(caption, choices):
	contents = menu(caption, choices)
	def open_menu(button):
		return top.open_box(contents,caption)
	return menu_button([caption, u'...'], open_menu)

def menu(title, choices):
	body = []
	body.extend(choices)
	return urwid.ListBox(urwid.SimpleFocusListWalker(body))

def exit_program(button):
	raise urwid.ExitMainLoop()

outputting_command = False
def exit_program_start(button):
	global outputting_command
	if len(E.call['executable'].edit_text)==0:
		E.call['executable_attr'].set_attr_map({None:"focus line"})
		return
	else:
		E.call['executable_attr'].set_attr_map({None:None})
	
	if len(E.call['processes'].edit_text)==0 or int(E.call['processes'].edit_text)<2:
		E.call['processes_attr'].set_attr_map({None:"focus line"})
		return
	else:
		E.call['processes_attr'].set_attr_map({None:None})
		
	if len(E.call['output'].edit_text)==0:
		E.call['output_attr'].set_attr_map({None:"focus line"})
		return
	else:
		E.call['output_attr'].set_attr_map({None:None})
	outputting_command = True
	raise urwid.ExitMainLoop()

def back_callback(button):
	if top.box_level > 1:
		top.original_widget = top.original_widget[0]
		top.box_level -= 1

'''
global:
  { "OUTPUT_FILE", 'o', "OUTPUT_FILE", 0, "the filename to be outputted to"},
  { "MODE", 'm', "MODE", 0, "The mode of dagster operation, defult is no gnovelty, specified is with gnovelty"},
  { "breadth first search", 'b', "breadth_first_search", 0, "whether dagster should allocate messages in the dag depth first (recommended)"},
  { "enumerate solutions", 'e', "enumerate_solutions", 0, "whether dagster should exit on finding the first solution"},
  { "master sub mode", 'g', "master_sub_mode", 0, "1 if BDDmaster, 0 if TableMaster"},
  { "cnf directory", 'h', "cnf_directory", 0, "the directory name which dagster will output CNF partials to, unspecified = in memory"},

sat tuning:
  { "sat solution interrupt", 'i', "sat_solution_interrupt", 0, "the number of solutions a CDCL will discover before asking master for a possible reassignment"},
  { "sat reporting time", 'j', "sat_reporting_time", 0, "the number of decisions that the CDCL will make before asking master for a possible reassignment"},
  { "tinisat restarting", 'p', "tinisat_restarting", 0, "a flag that is set if tinisat is to do restarts in its decision process."},
  { "solution trimming", 't', "solution_trimming", 0, "a 0/1 flag whether tinisat should attempt to verify and trim variables passing through, 2 two for trimming all but positive literals"},
  { "opportunity modulo", 'x', "opportunity_modulo", 0, "opportunity modulo in the geometric restarting scheme (0 is default, = off)"},
  { "discount factor", 'y', "discount_factor", 0, "discount factor in the geometric restarting scheme (default is 0.95, only applied if opportunity_modulo is nonzero)"},

checkpointing:
  { "checkpoint filename", 'u', "checkpoint_filename", 0, "the checkpoint file to load on initialisation"},
  { "checkpoint frequency", 'v', "checkpoint_frequency", 0, "the number of seconds between checkpoint dumps"},
  { "checkpoint file prefix", 'z', "checkpoint_file_prefix", 0, "the filename prefix controlling where to dump the checkpointing files (if enabled by checkpoint_frequency)"},

bdd:
  { "BDD_COMPILATION", 'c', "BDD_COMPILATION", 0, "one of {cubes, minisat, paths}. The CNF representation of BDDs."},

gnovelty:
  { "number of gnovelties per solver", 'k', "novelty_number", 0, "number of gnovelties per sat solver (only in gnovelty mode)"},
  { "SLS_HEURISTIC", 'a', "SLS_HEURISTIC", 0, "the type of heuristic guidance being provided by gNovelty+ to a CDCL solver"},
  { "decision_interval", 'd', "decision_interval", 0, "gnovelty decision_interval (only in gnovelty mode)"},
  { "gnovelty solution checking time", 'l', "gnovelty_solution_checking_time", 0, "the number decsions that the CDCL will make before checking for a solution from gnovelties"},
  { "heuristic_rotation_scheme", 'r', "heuristic_rotation_scheme", 0, "gnovelty heuristic_rotation_scheme (only in gnovelty mode)"},
  { "suggestion_size", 's', "suggestion_size", 0, "gnovelty suggestion_size (only in gnovelty mode)"},
  { "DLS", 'w', "DLS", 0, "'1' if gNovelty+ is to use clause weights"},

'''


bdd_config = []
bdd_mode = []
sls_heuristic = []
sls_heuristic_rotation_scheme = []
tinisat_solution_trimming = []


menu_top = menu(u'Main Menu', [
	E.register(urwid.AttrMap(E.register(E(urwid.Edit, [u'Dagster Executable Location: '], '#d', {'help':"The location of the Dagster executable, as might be referenced from the commandline, relative to the current working directory or otherwise"}), 'executable'), None, focus_map='reversed'), 'executable_attr'),
	E.register(urwid.AttrMap(E.register(E(urwid.IntEdit, [u'number of MPI processes: ',1], '#0', {'help':"The number of processes initialised by MPI call. one of these processes will be the Master process, the others will be divided into worker groups, each worker group will be a CDCL and/or Strengthener and/or multiple SLS processes - these will be configured by other options. keep in mind that the Dagster initialisation can fail, if it isnt possible for Dagster to cleanly allocate a CDCL and Strengthener to every group where Strengthener is enabled."}), 'processes'), None, focus_map='reversed'), 'processes_attr'),
	E.register(urwid.AttrMap(E.register(E(urwid.Edit, [u'Output Filename: '], 'o', {'help':"The file (as may be addressed from the commandline) where the output from the dagster run will be stored - comandline usage defaults to 'dag_out.txt', otherwise specified here."}), "output"), None, focus_map='reversed'), 'output_attr'),
	urwid.Divider(),
	urwid.AttrMap(E(urwid.CheckBox, ["enumerating all solutions"], 'e', {"help":"the Flag which if set, makes dagster try to enumerate all solutions to the problem, carrying all solutions from all subprobles through the dag to completion. warning: if the problem is badly formatted or the DAG improperly designed, this may result in combinatorially many solutions taking large amount of time to complete.\n If set false, then Dagster will exit as soon as one solution is found to a terminal node in the DAG, this can also take a while depending on whether 'using breadth first search' is enabled, as opposed to 'depth first search', prefer disabling 'breadth first search' when not enumerating all solutions."}), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.CheckBox, ["Using breadth-first search paradigm"], 'b', {"help":"When using breadth first search, Dagster will attempt to schedule work to workers one depth level in the Dag at a time, completing each depth before moving onto the next - in this way Dagster will attempt to comprehensively trace all messages through the dag, disabling this option informs Dagster to engage 'depth first search' and thus preferentially scheduling work deeper in the dag, and thus attempting to race to the end to generate a solution"}), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.CheckBox, ["CNF file splitting"], "!h", {"help":"If enabled, Dagster will split the input CNF into parts in the 'splitting directory', these parts will then be subsequently loaded into dagster workers as needed, this mode enables Dagster to work on larger CNFs with many than it could sensibly store in memory at once"}), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.Edit, [u'	|_ splitting directory: '], 'h', {"help":"The directory (which will be created if not existant) where Dagster will dump the CNF parts if 'CNF file splitting' is enabled"}), None, focus_map='reversed'),
	urwid.Divider(),
	sub_menu(u'Checkpointing Configuration', [
		urwid.AttrMap(E(urwid.CheckBox, ["Dumping Checkpoints"], "!zv", {"help":"The flag to tell Dagster to dump its progress periodically to backup 'checkpoint' files, the output filename/s and the frequency at which this should be done is specified by the 'checkpoint output filename' and 'checkpoint frequency' fields, the most recent 5 checkpoint files are overwritten in a round robin fashion"}), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.Edit, [u'	|_ Checkpoint output Filename: '],'z',{"help":"The full filename (relative to current working directory) base which Dagster will use to dump the checkpoint files"}), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.IntEdit, [u'	|_ Checkpoint Frequency (seconds): ',1],'v', {"help":"The number of seconds Dagster should wait before between dumping checkpoint files"}), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.AttrMap(E(urwid.CheckBox, ["Loading a Checkpoint"], "!u", {"help":"A checkbox to determine if dagster should load a checkpoint on initialisation, and if so, then the filename of the checkpoint will be specified by 'checkpoint input filename' field"}), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.Edit, [u'	|_ Checkpoint Input Filename: '],'u',{"help":"The filename (relative to current working directory) which Dagster will load in on initialisation, only used if 'loading a checkpoint' field is set"}), None, focus_map='reversed'),
		urwid.Divider(),
		menu_button(u'Back', back_callback),
	]),
	urwid.Divider(),
	sub_menu(u'CDCL Tuning Options', [
		urwid.AttrMap(E(urwid.CheckBox, ["TiniSat restarting",True],'p',{"help":"Flag to set whether the CDCL solver will use restarting at all (!), defaults to true, if set to false it can speed up some problem solving in specific instances."}), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.Text("TiniSat solution trimming:"),
		urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'Off'], '@t0',{"help":"the CDCL solvers, when they generate a valid solution will not attempt to trim unnessisary literals, but will pass a valuation to all literals in the CNF part which the CDCL was working on. Note: this is usually a bad option, and having solution trimming 'on' is usually better"}), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'On'], '@t1',{"help":"the CDCL solvers, when they generate a valid solution will attempt to trim unnessisary literals, this it will pass a valuation to a subset of literals to satisfy the CNF part which the CDCL was working on. Note: this is usually a safe and good option"}), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'Positive literals only'], '@t2',{"help":"the CDCL solvers, when they generate a valid solution will trim ALL negative literal valuations in the solutions that it finds, Note: this is a VERY usually option to enable, and is only usefull when positive literal evaluations tacitly encode all the valuations of relevent negative literals in a particular problem"}), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.AttrMap(E(urwid.IntEdit, [u'SAT reporting time: ',40], 'j', {"help":"the CDCL worker will periodically report back Master process to give master the opportunity to reassign the CDCL worker, this field controls how many decisions the CDCL solver should conduct before polling the master for a potential reassignment, the larger the number the less frequent the worker will poll master, and the lower the communication overhead (particularly if there are many workers) however the smaller the number, the more often the workers will poll the master for potential reassignment, possibly creating larger overhead"}), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.IntEdit, [u'SAT solution interrupt: ',40], 'i', {"help":"the CDCL worker will periodically report back Master process to give master the opportunity to reassign the CDCL worker, this field controls how many solutions the CDCL solver should find before polling the master for a potential reassignment, the larger the number the less frequent the worker will poll master, and the lower the communication overhead (particularly if there are many workers) however the smaller the number, the more often the workers will poll the master for potential reassignment, possibly creating larger overhead"}), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.AttrMap(E(urwid.CheckBox, ["Geometric restarting"], '!xy', {"help":"Controls if the CDCL processes should use a special geometric restarting strategy, whereby the likelihood of a CDCL restart is perpetually increased over time, this increase is advantageous for for UNSAT problems, having this mode enabled means that the CDCL gradually shifts to a restarting strategy more optimal for UNSAT problems the longer the solution process is taking. This option can be advantageous where there is UNSAT problems present in the solving process."}), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.IntEdit, [u'	|_ Opportunity modulo: ',40],'x', {"help":"The number of decisions that the CDCL process will make in the geometric restarting scheme before altering the probability of restarting, the smaller this number the more often and quicker Dagster will be at increasing the restarting rate"}), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.Edit, [u'	|_ discount factor: ','0.95'],'y', {"help":"The proportion by which dagster will increase the restarting rate every 'opportunity modulo' decisions in the geometric restarting scheme, for instance 0.95 means that the restarting rate will increase by 5% every 'opportunity modulo' decisions"}), None, focus_map='reversed'),
		urwid.Divider(),
		menu_button(u'Back', back_callback),
	]),
	urwid.Divider(),
	urwid.AttrMap(E(urwid.CheckBox, ["Using Strengthener"], '#2', {"help":""}), None, focus_map='reversed'),
	urwid.Divider(),
	urwid.Text("SLS Helpers:"),
	urwid.AttrMap(E(urwid.CheckBox, ["Using gNovelty+ Helpers"], '!kdlswra #1'), None, focus_map='reversed'),
	sub_menu(u'gNovelty+ Configuration', [
		urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ per CDCL: ',1], 'k'), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ decision interval: ',30], 'd'), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ solution checking time: ',30] ,'l'), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ suggestion size: ',30], 's'), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.CheckBox, ["gNovelty+ clause weights "], 'w'), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.Text("CDCL heuristic rotation scheme:"),
		urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic,u'slsfirst'], '@rslsfirst'), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic,u'cdclfirst'], '@rcdclfirst'), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.Text("SLS suggestion heuristic:"),
		urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic_rotation_scheme,u'most recent flip (ie `ghosts` mode)'], '@aghosts'), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic_rotation_scheme,u'most relevent flip (non `ghosts` mode)'], '@anonghosts'), None, focus_map='reversed'),
		urwid.Divider(),
		menu_button(u'Back', back_callback),
	]),
	urwid.Divider(),
	urwid.Text("Solutions Handler:"),
	urwid.AttrMap(E(urwid.RadioButton, [bdd_mode, u'Using Table Solutions Handler'], '@g0'), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.RadioButton, [bdd_mode, "Using Dumb Table Solutions Handler"], '@g2'), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.RadioButton, [bdd_mode, "Using BDD Solutions Handler"], '@g1'), None, focus_map='reversed'),
	sub_menu(u'BDD handler Configuration', [
		urwid.AttrMap(E(urwid.RadioButton, [bdd_config,u'- BDD paths encoding'], '@cpaths'), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [bdd_config,u'- BDD cubes encoding'], '@ccubes'), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [bdd_config,u'- BDD minisat encoding'], '@cminisat'), None, focus_map='reversed'),
		menu_button(u'Back', back_callback),
	]),
	urwid.Divider(),
	menu_button(u'Start', exit_program_start),
	urwid.Divider(),
	menu_button(u'Exit', exit_program),
	
])

class CascadingBoxes(urwid.WidgetPlaceholder):
	max_box_levels = 4

	def __init__(self, box, caption):
		super(CascadingBoxes, self).__init__(urwid.SolidFill(u' '))
		self.box_level = 0
		self.open_box(box,caption)

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
			align='center', width=('relative', 60),
			valign='middle', height=('relative', 60),
			min_width=20, min_height=8,
			left=self.box_level * 3,
			right=(self.max_box_levels - self.box_level - 1) * 3,
			top=self.box_level * 2,
			bottom=(self.max_box_levels - self.box_level - 1) * 2)
		self.box_level += 1

	def keypress(self, size, key):
		if key == 'esc' and self.box_level > 1: #if escape pressed bump up the active overlay
			self.original_widget = self.original_widget[0]
			self.box_level -= 1
		elif key=='?': # if help key pressed, format a new overlay/window with appropriately generated help message from the active widget object
			selected_widget = self.original_widget.focus._original_widget.get_focus()[0].original_widget
			if 'help' in dir(selected_widget):
				help_string = selected_widget.help
			else:
				help_string = "Help for widget {}".format( str(selected_widget) )
			if 'caption' in dir(selected_widget):
				caption_string = [ss for ss in selected_widget.caption if ss in 'abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ']
				caption_string = "".join(caption_string).replace("  ", " ").replace("  ", " ").replace("  ", " ").strip()
			else:
				caption_string = "X"
			caption_string = "HELP for {}".format(caption_string)
			self.open_help_box(urwid.Filler(urwid.Pile([urwid.Text(help_string),menu_button(u'Back', back_callback)])),caption_string)
		else: # otherwise register the keypress
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

setattr(menu_top,"E",E)

# load input command file (if there is) into the gui
if (len(sys.argv)>1):
	print("inputting file {}".format(sys.argv[1]))
	with open(sys.argv[1],"r") as f:
		import_data = json.load(f)
	for k in import_data.keys():
		for v in E.e['Edit']:
			if k==v[1]:
				v[0].set_edit_text(import_data[k])
		for v in E.e['IntEdit']:
			if k==v[1]:
				v[0].set_edit_text(import_data[k])
		for v in E.e['CheckBox']:
			if k==v[1]:
				v[0].set_state(import_data[k])
		for v in E.e['RadioButton']:
			if k==v[1]:
				v[0].set_state(import_data[k])

top = CascadingBoxes(menu_top,"Main Menu")
frame = urwid.Frame(body=top,footer=footer_txt, header=header_padding)


# enter main loop, and exit if selected
urwid.MainLoop(frame, palette).run()



if not outputting_command:
	sys.exit(1)


# load all the element values into a config dictionary
config = {}
for v in E.e['Edit']:
	config[v[1]] = v[0].edit_text
for v in E.e['IntEdit']:
	config[v[1]] = v[0].edit_text
for v in E.e['CheckBox']:
	config[v[1]] = int(v[0]._state)
for v in E.e['RadioButton']:
	config[v[1]] = v[0]._state
config_copy = config.copy() # mutable copy

# dump config to file
with open("interface_dump.json","w") as f:
	json.dump(config,f)



# process all elements with the @ and # characters
# @XY means set X to Y if the element is true
# #abracadabra is a special element that is set if true
special = {}
for k in list(config.keys()):
	split_k = k.split(" ")
	i = 0
	while i<len(split_k):
		kk= split_k[i]
		if len(kk)==0:
			i += 1
			continue
		elif kk[0]=='@':
			assert len(kk)>1
			if config[k]:
				config_copy[kk[1]] = kk[2:]
		elif kk[0]=='#':
			if config[k]:
				special[kk] = config[k]
		else:
			i += 1
			continue
		del split_k[i]
	new_split_k = " ".join(split_k)
	if new_split_k!=k:
		config_copy[new_split_k] = config_copy[k]
		del config_copy[k]

# process all elements with the ! element
# !abcd means delete all config elements a,b,c and d if the element is true
for k in list(config_copy.keys()):
	split_k = k.split(" ")
	i = 0
	while i<len(split_k):
		kk= split_k[i]
		if len(kk)==0:
			i += 1
			continue
		elif kk[0]=='!':
			if not config_copy[k]:
				for kill_k in kk[1:]:
					del config_copy[kill_k]
		else:
			i += 1
			continue
		del split_k[i]
	new_split_k = " ".join(split_k)
	if new_split_k!=k:
		config_copy[new_split_k] = config_copy[k]
		del config_copy[k]

# remove redundant or defective config elements, eg. empty strings or labels
for k in list(config_copy.keys()):
	if isinstance(k,str) and len(k)==0:
		del config_copy[k]
	elif isinstance(config_copy[k],str) and len(config_copy[k])==0:
		del config_copy[k]

# special processing for mode flags
gnovelty_present = '#1' in special.keys()
strengthener_present = '#2' in special.keys()
mode_number = 0
if gnovelty_present and strengthener_present:
	mode_number = 2
elif strengthener_present:
	mode_number = 3
elif gnovelty_present:
	mode_number = 1
config_copy['m']=mode_number

#format the command string
config_keys = sorted(config_copy.keys())
parameters = " ".join(["-{} {}".format(k,config_copy[k]) for k in config_keys])
s = "mpirun -n {} {} {}".format(special['#0'],special['#d'], parameters)

#output the command to be run
import sys
sys.stderr.write(s)


