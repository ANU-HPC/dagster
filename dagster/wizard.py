import urwid
#import cgitb
import sys
#cgitb.enable(format="")



from collections import defaultdict
class Elements(object):
	def __init__(self):
		self.e = defaultdict(list)
		self.call = {}
	def __call__(self,element_type,args,char=''):
		n = element_type(*args)
		self.e[element_type].append([n,char])
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
    button = urwid.Button(caption)
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
    E.register(urwid.AttrMap(E.register(E(urwid.Edit, [u'Dagster Executable Location: '], '#d'), 'executable'), None, focus_map='reversed'), 'executable_attr'),
    E.register(urwid.AttrMap(E.register(E(urwid.IntEdit, [u'number of MPI processes: ',1], '#0'), 'processes'), None, focus_map='reversed'), 'processes_attr'),
    E.register(urwid.AttrMap(E.register(E(urwid.Edit, [u'Output Filename: '], 'o'), "output"), None, focus_map='reversed'), 'output_attr'),
    urwid.Divider(),
    urwid.AttrMap(E(urwid.CheckBox, ["enumerating all solutions"], 'e'), None, focus_map='reversed'),
    urwid.AttrMap(E(urwid.CheckBox, ["Using breadth-first search paradigm"], 'b'), None, focus_map='reversed'),
    urwid.AttrMap(E(urwid.CheckBox, ["CNF file splitting"], "!h"), None, focus_map='reversed'),
    urwid.AttrMap(E(urwid.Edit, [u'    |_ splitting directory: '], 'h'), None, focus_map='reversed'),
    urwid.Divider(),
    sub_menu(u'Checkpointing Configuration', [
        urwid.AttrMap(E(urwid.CheckBox, ["Dumping Checkpoints"], "!zv"), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.Edit, [u'    |_ Checkpoint output Filename: '],'z'), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.IntEdit, [u'    |_ Checkpoint Frequency (seconds): ',1],'v'), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.AttrMap(E(urwid.CheckBox, ["Loading a Checkpoint"], "!u"), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.Edit, [u'    |_ Checkpoint Input Filename: '],'u'), None, focus_map='reversed'),
        urwid.Divider(),
        menu_button(u'Back', back_callback),
    ]),
    urwid.Divider(),
    sub_menu(u'CDCL Tuning Options', [
        urwid.AttrMap(E(urwid.CheckBox, ["TiniSat restarting",True],'p'), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.Text("TiniSat solution trimming:"),
        urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'Off'], '@t0'), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'On'], '@t1'), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'Positive literals only'], '@t2'), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.AttrMap(E(urwid.IntEdit, [u'SAT reporting time: ',40], 'j'), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.IntEdit, [u'SAT solution interrupt: ',40], 'i'), None, focus_map='reversed'),
        
        urwid.Divider(),
        urwid.AttrMap(E(urwid.CheckBox, ["Geometric restarting"], '!xy'), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.IntEdit, [u'    |_ Opportunity modulo: ',40],'x'), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.Edit, [u'    |_ discount factor: ','0.95'],'y'), None, focus_map='reversed'),
        urwid.Divider(),
        menu_button(u'Back', back_callback),
    ]),
    urwid.Divider(),
    urwid.AttrMap(E(urwid.CheckBox, ["Using Strengthener"], '#2'), None, focus_map='reversed'),
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
frame = urwid.Frame(body=top,footer=footer_txt, header=header_padding)




# load input command file (if there is) into the gui
if (len(sys.argv)>1):
	print("inputting file {}".format(sys.argv[1]))



# enter main loop, and exit if selected
urwid.MainLoop(frame, palette).run()
if not outputting_command:
	sys.exit(1)



# load all the element values into a config dictionary
config = {}
for v in E.e[urwid.Edit]:
	config[v[1]] = v[0].edit_text
for v in E.e[urwid.IntEdit]:
	config[v[1]] = v[0].edit_text
for v in E.e[urwid.CheckBox]:
	config[v[1]] = int(v[0]._state)
for v in E.e[urwid.RadioButton]:
	config[v[1]] = v[0]._state
config_copy = config.copy() # mutable copy

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


