import urwid
import cgitb
cgitb.enable(format="")


header_txt = urwid.Text(u"-------------Dagster Startup Utility-------------")
footer_txt = urwid.Text(u"Press '?' for help on any item")
header_padding = urwid.Padding(header_txt,'center','pack')
header_padding = urwid.AttrMap(header_padding, 'selected')


radio_button_group = []

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

def item_chosen(button):
    response = urwid.Text([u'You chose ', button.label, u'\n'])
    done = menu_button(u'Ok', exit_program)
    zog = urwid.AttrMap(urwid.IntEdit(u'oogabooga: ',42), None, focus_map='reversed')
    zog2 = urwid.AttrMap(urwid.RadioButton(radio_button_group,u'oogaboo: '), None, focus_map='reversed')
    zog3 = urwid.AttrMap(urwid.RadioButton(radio_button_group,u'oogabga: '), None, focus_map='reversed')
    zog4 = urwid.AttrMap(urwid.RadioButton(radio_button_group,u'oogooga: '), None, focus_map='reversed')
    top.open_box(urwid.Filler(urwid.Pile([response, done, zog,zog2,zog3,zog4])),"zozoz")

def exit_program(button):
    raise urwid.ExitMainLoop()

outputting_command = False
def exit_program_start(button):
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

from collections import defaultdict


bdd_config = []
sls_heuristic = []
sls_heuristic_rotation_scheme = []
tinisat_solution_trimming = []

class Elements(object):
	def __init__(self):
		self.e = defaultdict(list)
	def __call__(self,element_type,args,char="z"):
		n = element_type(*args)
		self.e[element_type].append([n,char])
		return n

E = Elements()

menu_top = menu(u'Main Menu', [
    urwid.AttrMap(E(urwid.Edit, [u'Dagster Executable Location: ']), None, focus_map='reversed'),
    urwid.AttrMap(E(urwid.IntEdit, [u'number of MPI processes: ',1]), None, focus_map='reversed'),
    urwid.AttrMap(E(urwid.Edit, [u'Output Filename: ']), None, focus_map='reversed'),
    urwid.Divider(),
    urwid.AttrMap(E(urwid.CheckBox, ["enumerating all solutions"]), None, focus_map='reversed'),
    urwid.AttrMap(E(urwid.CheckBox, ["Using breadth-first search paradigm"]), None, focus_map='reversed'),
    urwid.AttrMap(E(urwid.CheckBox, ["CNF file splitting"]), None, focus_map='reversed'),
    urwid.AttrMap(E(urwid.Edit, [u'    |_ splitting directory: ']), None, focus_map='reversed'),
    urwid.Divider(),
    sub_menu(u'Checkpointing Configuration', [
        urwid.AttrMap(E(urwid.CheckBox, ["Dumping Checkpoints"]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.Edit, [u'    |_ Checkpoint output Filename: ']), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.IntEdit, [u'    |_ Checkpoint Frequency (seconds): ',1]), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.AttrMap(E(urwid.CheckBox, ["Loading a Checkpoint"]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.Edit, [u'    |_ Checkpoint Input Filename: ']), None, focus_map='reversed'),
        urwid.Divider(),
        menu_button(u'Back', back_callback),
    ]),
    urwid.Divider(),
    sub_menu(u'CDCL Tuning Options', [
        urwid.AttrMap(E(urwid.CheckBox, ["TiniSat restarting"]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.CheckBox, ["TiniSat solution trimming"]), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.Text("TiniSat solution trimming:"),
        urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'Off']), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'On']), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'Positive literals only']), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.AttrMap(E(urwid.IntEdit, [u'SAT reporting time: ',40]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.IntEdit, [u'SAT solution interrupt: ',40]), None, focus_map='reversed'),
        
        urwid.Divider(),
        urwid.AttrMap(E(urwid.CheckBox, ["Geometric restarting"]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.IntEdit, [u'    |_ Opportunity modulo: ',40]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.Edit, [u'    |_ discount factor: ']), None, focus_map='reversed'),
        urwid.Divider(),
        menu_button(u'Back', back_callback),
    ]),
    urwid.Divider(),
    urwid.AttrMap(E(urwid.CheckBox, ["Using Strengthener"]), None, focus_map='reversed'),
    sub_menu(u'gNovelty+ Configuration', [
        urwid.AttrMap(E(urwid.CheckBox, ["Using gNovelty+"]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.IntEdit, [u'    |_ gNovelty+ per CDCL: ',1]), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ decision interval: ',30]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ solution checking time: ',30]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ suggestion size: ',30]), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.CheckBox, ["gNovelty+ clause weights: "]), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.Text("CDCL heuristic rotation scheme:"),
        urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic,u'slsfirst']), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic,u'cdclfirst']), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.Text("SLS suggestion heuristic:"),
        urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic_rotation_scheme,u'most recent flip (ie `ghosts` mode)']), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic_rotation_scheme,u'most relevent flip (non `ghosts` mode)']), None, focus_map='reversed'),
        urwid.Divider(),
        menu_button(u'Back', back_callback),
    ]),
    sub_menu(u'Master Configuration', [
        urwid.AttrMap(E(urwid.CheckBox, ["Using BDD master"]), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.AttrMap(E(urwid.RadioButton, [bdd_config,u'- BDD paths encoding']), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.RadioButton, [bdd_config,u'- BDD cubes encoding']), None, focus_map='reversed'),
        urwid.AttrMap(E(urwid.RadioButton, [bdd_config,u'- BDD minisat encoding']), None, focus_map='reversed'),
        urwid.Divider(),
        urwid.AttrMap(E(urwid.CheckBox, ["Using Dumb Table master (WARNING: unsafe)"]), None, focus_map='reversed'),
        urwid.Divider(),
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

import os

urwid.MainLoop(frame, palette).run()
#if outputting_command:
os.system("clear")
os.system("echo 'mpirun -n 44 ../../dagster -e 0 -b 1 -c 4 -e 2 -g 2 -h /qfwfw/ -o output.txt my.dag my.cnf'")
for v in E.e[urwid.Edit]:
	print(v[1], v[0].edit_text)
for v in E.e[urwid.IntEdit]:
	print(v[1], v[0].edit_text)
for v in E.e[urwid.CheckBox]:
	print(v[1], v[0]._state)
for v in E.e[urwid.RadioButton]:
	print(v[1], v[0]._state)
print(bdd_config)
print(sls_heuristic)
print(sls_heuristic_rotation_scheme)
print(tinisat_solution_trimming)
#urwid.RadioButton


