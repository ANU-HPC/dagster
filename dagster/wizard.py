'''
______________
| THE WIZARD |
^^^^^^^^^^^^^^

A console graphical interface designed to present information and options about how to configure Dagster program
Output from the wizard is a commandline of an appropriate invocation to stdout

The wizard is designed to be easy to use, and quickly designed and sketchy to be adaptable to any required changes
to dagster interface requirments.

The wizard allows many configuration options and the possibility to save the configuration to a json formatted config file,
to be re loaded in with the --config_input=<config_file> commandline flag

The wizard relies apon the running of a python 3+ version, with an environment with urwid and click libaries installed.
'''


import sys
import json
import urwid
import click



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
	
	if len(E.call['cnf'].edit_text)==0:
		E.call['cnf_attr'].set_attr_map({None:"focus line"})
		return
	else:
		E.call['cnf_attr'].set_attr_map({None:None})
	
	if len(E.call['dag'].edit_text)==0:
		E.call['dag_attr'].set_attr_map({None:"focus line"})
		return
	else:
		E.call['dag_attr'].set_attr_map({None:None})
	
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
	
	if E.call['config_output_enabled'].state==True and len(E.call['config_output'].edit_text)==0:
		E.call['config_output_attr'].set_attr_map({None:"focus line"})
		return
	else:
		E.call['config_output_attr'].set_attr_map({None:None})
	
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
cdck_base_selection = []


# All the help tips, indexed by a key string
help = {"Dagster Executable Location":{'help':"The location of the Dagster executable, as might be referenced from the commandline, relative to the current working directory or otherwise"},
"CNF File Input":{'help':"The location of the CNF File"},
"DAG File Input":{'help':"The location of the DAG File"},
"number of MPI processes":{'help':"The number of processes initialised by MPI call. one of these processes will be the Master process, the others will be divided into worker groups, each worker group will be a CDCL and/or Strengthener and/or multiple SLS processes - these will be configured by other options. keep in mind that the Dagster initialisation can fail, if it isnt possible for Dagster to cleanly allocate a CDCL and Strengthener to every group where Strengthener is enabled."},
"Output Filename":{'help':"The file (as may be addressed from the commandline) where the output from the dagster run will be stored - comandline usage defaults to 'dag_out.txt', otherwise specified here."},
"enumerating all solutions":{"help":"the Flag which if set, makes dagster try to enumerate all solutions to the problem, carrying all solutions from all subprobles through the dag to completion. warning: if the problem is badly formatted or the DAG improperly designed, this may result in combinatorially many solutions taking large amount of time to complete.\n If set false, then Dagster will exit as soon as one solution is found to a terminal node in the DAG, this can also take a while depending on whether 'using breadth first search' is enabled, as opposed to 'depth first search', prefer disabling 'breadth first search' when not enumerating all solutions."},
"Using breadth-first search paradigm":{"help":"When using breadth first search, Dagster will attempt to schedule work to workers one depth level in the Dag at a time, completing each depth before moving onto the next - in this way Dagster will attempt to comprehensively trace all messages through the dag, disabling this option informs Dagster to engage 'depth first search' and thus preferentially scheduling work deeper in the dag, and thus attempting to race to the end to generate a solution"},
"CNF file splitting":{"help":"If enabled, Dagster will split the input CNF into parts in the 'splitting directory', these parts will then be subsequently loaded into dagster workers as needed, this mode enables Dagster to work on larger CNFs with many than it could sensibly store in memory at once"},
"splitting directory":{"help":"The directory (which will be created if not existant) where Dagster will dump the CNF parts if 'CNF file splitting' is enabled"},
"Dumping Checkpoints":{"help":"The flag to tell Dagster to dump its progress periodically to backup 'checkpoint' files, the output filename/s and the frequency at which this should be done is specified by the 'checkpoint output filename' and 'checkpoint frequency' fields, the most recent 5 checkpoint files are overwritten in a round robin fashion"},
"Checkpoint output Filename":{"help":"The full filename (relative to current working directory) base which Dagster will use to dump the checkpoint files"},
"Checkpoint Frequency":{"help":"The number of seconds Dagster should wait before between dumping checkpoint files"},
"Loading a Checkpoint":{"help":"A checkbox to determine if dagster should load a checkpoint on initialisation, and if so, then the filename of the checkpoint will be specified by 'checkpoint input filename' field"},
"Checkpoint Input Filename":{"help":"The filename (relative to current working directory) which Dagster will load in on initialisation, only used if 'loading a checkpoint' field is set"},
"TiniSat restarting":{"help":"Flag to set whether the CDCL solver will use restarting at all (!), defaults to true, if set to false it can speed up some problem solving in specific instances."},
"tinisat_solution_trimming_Off":{"help":"the CDCL solvers, when they generate a valid solution will not attempt to trim unnessisary literals, but will pass a valuation to all literals in the CNF part which the CDCL was working on. Note: this is usually a bad option, and having solution trimming 'on' is usually better"},
"tinisat_solution_trimming_On":{"help":"the CDCL solvers, when they generate a valid solution will attempt to trim unnessisary literals, this it will pass a valuation to a subset of literals to satisfy the CNF part which the CDCL was working on. Note: this is usually a safe and good option"},
"tinisat_solution_trimming_Positive_literals_only":{"help":"the CDCL solvers, when they generate a valid solution will trim ALL negative literal valuations in the solutions that it finds, Note: this is a VERY usually option to enable, and is only usefull when positive literal evaluations tacitly encode all the valuations of relevent negative literals in a particular problem"},
"SAT reporting time":{"help":"the CDCL worker will periodically report back Master process to give master the opportunity to reassign the CDCL worker, this field controls how many decisions the CDCL solver should conduct before polling the master for a potential reassignment, the larger the number the less frequent the worker will poll master, and the lower the communication overhead (particularly if there are many workers) however the smaller the number, the more often the workers will poll the master for potential reassignment, possibly creating larger overhead"},
"SAT solution interrupt":{"help":"the CDCL worker will periodically report back Master process to give master the opportunity to reassign the CDCL worker, this field controls how many solutions the CDCL solver should find before polling the master for a potential reassignment, the larger the number the less frequent the worker will poll master, and the lower the communication overhead (particularly if there are many workers) however the smaller the number, the more often the workers will poll the master for potential reassignment, possibly creating larger overhead"},
"Geometric restarting":{"help":"Controls if the CDCL processes should use a special geometric restarting strategy, whereby the likelihood of a CDCL restart is perpetually increased over time, this increase is advantageous for for UNSAT problems, having this mode enabled means that the CDCL gradually shifts to a restarting strategy more optimal for UNSAT problems the longer the solution process is taking. This option can be advantageous where there is UNSAT problems present in the solving process."},
"Opportunity modulo":{"help":"The number of decisions that the CDCL process will make in the geometric restarting scheme before altering the probability of restarting, the smaller this number the more often and quicker Dagster will be at increasing the restarting rate"},
"discount factor":{"help":"The proportion by which dagster will increase the restarting rate every 'opportunity modulo' decisions in the geometric restarting scheme, for instance 0.95 means that the restarting rate will increase by 5% every 'opportunity modulo' decisions"},
"Using Strengthener":{"help":"Whether or not the Strengthener processes should be used, particularly if enabled, there will be allocated on strengthener process per CDCL process, which will be used to strengthen (ie. reduce the number of literals) in the conflict clauses of the CDCL procedure."},
"Using gNovelty+ Helpers":{"help":"Whether or not to use the gNovelty+ SLS helper processes, particuarly a number of SLS processes will be allocated per CDCL process, and will help the CDCL by (a) finding solutions and (b) offering variable suggestions for the CDCL to select. SLS helpers are potentially advsiable on suitable problems, particularly thoes which do not have very many variables, since gnovelty variable neighbourhood calculation overhead and storage is potentially quadratic in number of variables."},
"gNovelty+ per CDCL":{"help":"the number of gNovelty+ SLS helpers per CDCL to attempt to allocate"},
"gNovelty+ decision interval":{"help":"The gNovelty+ SLS helpers will be allocated to work at varying depths in the CDCL search, the decision interval is the desired spacing between the gNovelty+ allocated workers in the CDCL search tree, a spacing too large will result in the SLS process being allocated too sparsely, and thus they become less efficient, a spacing too small, and they become entirely localised to the most recent decisios in the CDCL process, and then become entirely inefficient. appropriate decision interval is some reasonable proportion (say 40%) of the expected search depth of the CDCL processes"},
"gNovelty+ solution checking time":{"help":"The CDCL process checks the gNovelty+ SLS helpers for solutions every so many decisions, the more frequently the check is performed, the less likely that the SLS processes will have a backlog of solutions to pass up, however the more inhibited the CDCL is in its own search, if there are expected to be many solutions then decreasing the checking time may be a good idea, however if there are few solutions, then lengthening the solution checking time may be desirable"},
"gNovelty+ suggestion size":{"help":"The size of the buffer that the SLS processes will report to the CDCL process, the greater the size of the buffer, the more comprehensive the SLS processes will report their trace back to the CDCL to help it follow the same path as the SLS. setting this value too high, will overload the CDCL with variable suggestions that are quckly changing and superfluous to the search, setting this value too low will mitigate the effectiveness of SLS suggestions as only the most static variables will be reported back to the CDCL - which may not even be particularly meaningfull. Broadly, the suggestion size should be comparable to the SLS decision interval."},
"gNovelty+ clause weights":{"help":"This option sets the SLS to dynamically try to weigh the satisfaction of particular clauses as its search proceeds, particularly it increases the weight of satisfying persistantly unsatisfied clauses. This is logical for enabling the SLS to escape local minima and find solutions to SAT queries more generally, however it potentially comes at the cost of being a good suggestion mechanism for the CDCL, as it attempts to escape local minima on its own accord rather than reporting suggestions of local minia to the CDCL for it to elminiate. This option is disabled by default, but should be considered if the SAT problem is expected to be friendly to SLS finding solutions over the CDCL process"},
"sls_heuristic_slsfirst":{"help":"The CDCL has a choice of how to select variables, particularly if it should choose suggestions from the SLS over variable selections to satisfy the most recently added learnt clauses (and thereafter defaulting to VSIDS heuristic), this option instructs CDCL to prioritise variable suggestions from SLS, and should be used where SLS suggestions are tuned and expected to be of quality."},
"sls_heuristic_cdclfirst":{"help":"The CDCL has a choice of how to select variables, particularly if it should choose suggestions from the SLS over variable selections to satisfy the most recently added learnt clauses (and thereafter defaulting to VSIDS heuristic), this option instructs CDCL to prioritise variable selections to satisfy recently learned clauses over variable suggestions from SLS."},
"sls_heuristic_rotation_scheme_most recent flip":{"help":"The SLS has options on how to report variable suggestions to the CDCL, particularly if it should report a collated and ranked list of variables ordered according to when they were last flipped, or simply a list of the variables that it most recently flipped (including duplicates), the latter is 'ghost' mode, as duplicate variables suggestions are consequently ignored by the CDCL, whereas the former is non-'ghosts' mode, where flipped variable values have more persistance in the SLS variable suggestions."},
"sls_heuristic_rotation_scheme_most relevent flip":{"help":"The SLS has options on how to report variable suggestions to the CDCL, particularly if it should report a collated and ranked list of variables ordered according to when they were last flipped, or simply a list of the variables that it most recently flipped (including duplicates), the latter is 'ghost' mode, as duplicate variables suggestions are consequently ignored by the CDCL, whereas the former is non-'ghosts' mode, where flipped variable values have more persistance in the SLS variable suggestions."},
"bdd_mode_Using Table Solutions Handler":{"help":"The Solutions Handler stores solutions between nodes as work to seed further computation, and there are different ways of storing these solutions, the Table Solutions Handler is the most simple, storing all the values of all the solutions as-is as a series of integer literals. this uncompressed and simple storage mechanims is uncomplicated and fast in simple cases where there are few messages between DAG nodes, however it will struggle with bloat trying to resolve messages together with DAGs with many solutions between nodes (and particularly with joins), the more expensive BDD Solutions Handler is designed to compress and represent/resolve solutions on a logical level using Binary Decision Diagrams."},
"bdd_mode_Using Dumb Table Solutions Handler":{"help":"Similar to the Table Solutions Handler, except is will refuse to resolve incompatable messages where there is a join in the DAG, this mode will produce an error on any problem where resolving solutions between DAG nodes is required (which is presumably most problems). Hence is only suitable for particular problems where messages between nodes are always cross compatable. This mitigates resolving overhead in these cases."},
"bdd_mode_Using BDD Solutions Handler":{"help":"The BDD Solutions Handler, stores messages/solutions between nodes on the problem DAG, it representes these solutions in their logical form using Binary Decision Diagrams, this compresses the messages in memory and minimises the number of subsequent runs, however it also has the potential to slow down the Master process in processing and compressing the messages that it handles. This BDD Solutions Handler is most appropriate where DAG nodes generate more solutions and there is need to logically resolve them together to minimise the volume of consequent processing required."},
"bdd_config_BDD paths encoding":{"help":"The BDD Solutions Handler (if enabled) needs to be instructed how to represent information that it subequently delivers to CDCL SAT workers, the 'paths' encoding is the simplest, encoding all branches in a BDD as separate clauses to be delivered to the CDCL to instruct it what to do/where not to go, etc. This contrasts with cubes and minisat encoding schemes which communicate this information more compactly at the cost of introducing additional variables"},
"Minisat CDCL Base":{"help":"Make the core CDCL solver used on the worker processes to be the Minisat Solver, this solver uses an array of differen heuristics for variable selection, clause learning and also clause forgetting, Minisat may work better than tinisat on specific CNF instances, but also currently has less options implemented than tinisat currently."},
"Tinisat CDCL Base":{"help":"Make the core CDCL solver used on the worker processes to be the TiniSAT solver, this solver is default historical CDCL process dagster was first built with and has a plurality of configuration options, performs well generally but does not have any learnt clause forgetting mechanisms"},
"Configuration file output":{"help":"The file that the configuration of the wizard should be output to"},
"Configuration output":{"help":"A checkbox indicating if the wizard should put its configuration into a file"},
"minisat_incrementality_Off":{"help":"This mode is default, Minisat workers will not store any information between processing messages sent to them by master, each new message will be handled anew. a diadvantage of this is that when the master sends a worker a sequence of messages that occur on the same node of the DAG there will be no memory of previous computation to accelerate subsequent computation, however a benefit to this mode is that there will be no bloat of that information possible"},
"minisat_incrementality_On":{"help":"This mode is not default, Minisat workers will store learnt clauses from processing messages sent to them by master. when the master sends a worker a sequence of messages that occur on the same node of the DAG there will be a memory of learnt clauses of the previous computation to accelerate subsequent computation."},
"bdd_config_BDD cubes encoding":{"help":""},
"bdd_config_BDD minisat encoding":{"help":""},  #TODO
}


# construction of menu archietcutre and options/config
menu_top = menu(u'Main Menu', [
	E.register(urwid.AttrMap(E.register(E(urwid.Edit, [u'Dagster Executable Location: '], '#d', help['Dagster Executable Location']), 'executable'), None, focus_map='reversed'), 'executable_attr'),
	E.register(urwid.AttrMap(E.register(E(urwid.Edit, [u'CNF File Input: '], '#w', help['CNF File Input']), 'cnf'), None, focus_map='reversed'), 'cnf_attr'),
	E.register(urwid.AttrMap(E.register(E(urwid.Edit, [u'DAG File Input: '], '#q', help['DAG File Input']), 'dag'), None, focus_map='reversed'), 'dag_attr'),
	E.register(urwid.AttrMap(E.register(E(urwid.IntEdit, [u'Number of MPI processes: ',1], '#0', help['number of MPI processes']), 'processes'), None, focus_map='reversed'), 'processes_attr'),
	E.register(urwid.AttrMap(E.register(E(urwid.Edit, [u'Output Filename: '], 'o', help['Output Filename']), "output"), None, focus_map='reversed'), 'output_attr'),
	urwid.Divider(),
	urwid.AttrMap(E(urwid.CheckBox, ["Enumerating all solutions"], 'e', help['enumerating all solutions']), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.CheckBox, ["Using breadth-first search paradigm"], 'b', help['Using breadth-first search paradigm']), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.CheckBox, ["CNF file splitting"], "!h", help['CNF file splitting']), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.Edit, [u' |_ splitting directory: '], 'h', help['splitting directory']), None, focus_map='reversed'),
	urwid.Divider(),
	sub_menu(u'Checkpointing Configuration', [
		urwid.AttrMap(E(urwid.CheckBox, ["Dumping Checkpoints"], "!zv", help['Dumping Checkpoints']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.Edit, [u' |_ Checkpoint output Filename: '],'z',help['Checkpoint output Filename']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.IntEdit, [u' |_ Checkpoint Frequency (seconds): ',1],'v', help['Checkpoint Frequency']), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.AttrMap(E(urwid.CheckBox, ["Loading a Checkpoint"], "!u", help['Loading a Checkpoint']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.Edit, [u' |_ Checkpoint Input Filename: '],'u',help['Checkpoint Input Filename']), None, focus_map='reversed'),
		urwid.Divider(),
		menu_button(u'Back', back_callback),
	]),
	urwid.Divider(),
	urwid.AttrMap(E(urwid.RadioButton, [cdck_base_selection, 'Tinisat CDCL Base'], '!ptjixykdlswra #4', help['Tinisat CDCL Base']), None, focus_map='reversed'),
	sub_menu(u'  |_ Tinisat Tuning Options', [
		urwid.AttrMap(E(urwid.CheckBox, ["TiniSat restarting",True],'p',help['TiniSat restarting']), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.Text("TiniSat solution trimming:"),
		urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'Off'], '@t0',help['tinisat_solution_trimming_Off']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'On'], '@t1',help['tinisat_solution_trimming_On']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'Positive literals only'], '@t2',help['tinisat_solution_trimming_Positive_literals_only']), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.AttrMap(E(urwid.IntEdit, [u'SAT reporting time: ',20000], 'j', help['SAT reporting time']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.IntEdit, [u'SAT solution interrupt: ',100], 'i', help['SAT solution interrupt']), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.AttrMap(E(urwid.CheckBox, ["Geometric restarting"], '!xy', help['Geometric restarting']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.IntEdit, [u' |_ Opportunity modulo: ',40],'x', help['Opportunity modulo']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.Edit, [u' |_ Discount factor: ','0.95'],'y', help['discount factor']), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.AttrMap(E(urwid.CheckBox, ["Using Strengthener"], '#2', help['Using Strengthener']), None, focus_map='reversed'),
		urwid.Divider(),
		urwid.Text("SLS Helpers:"),
		urwid.AttrMap(E(urwid.CheckBox, ["Using gNovelty+ Helpers"], '!kdlswra #1', help['Using gNovelty+ Helpers']), None, focus_map='reversed'),
		sub_menu(u'gNovelty+ Configuration', [
			urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ per CDCL: ',1], 'k', help['gNovelty+ per CDCL']), None, focus_map='reversed'),
			urwid.Divider(),
			urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ decision interval: ',30], 'd', help['gNovelty+ decision interval']), None, focus_map='reversed'),
			urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ solution checking time: ',50] ,'l', help['gNovelty+ solution checking time']), None, focus_map='reversed'),
			urwid.AttrMap(E(urwid.IntEdit, [u'gNovelty+ suggestion size: ',30], 's', help['gNovelty+ suggestion size']), None, focus_map='reversed'),
			urwid.AttrMap(E(urwid.CheckBox, ["gNovelty+ clause weights "], 'w', help['gNovelty+ clause weights']), None, focus_map='reversed'),
			urwid.Divider(),
			urwid.Text("CDCL heuristic rotation scheme:"),
			urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic,u'slsfirst'], '@rslsfirst', help['sls_heuristic_slsfirst']), None, focus_map='reversed'),
			urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic,u'cdclfirst'], '@rcdclfirst', help['sls_heuristic_cdclfirst']), None, focus_map='reversed'),
			urwid.Divider(),
			urwid.Text("SLS suggestion heuristic:"),
			urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic_rotation_scheme,u'Most recent flip (ie `ghosts` mode)'], '@aghosts', help['sls_heuristic_rotation_scheme_most recent flip']), None, focus_map='reversed'),
			urwid.AttrMap(E(urwid.RadioButton, [sls_heuristic_rotation_scheme,u'Most relevent flip (non `ghosts` mode)'], '@anonghosts', help['sls_heuristic_rotation_scheme_most relevent flip']), None, focus_map='reversed'),
			urwid.Divider(),
			menu_button(u'Back', back_callback),
		]),
		urwid.Divider(),
		menu_button(u'Back', back_callback),
	]),
	urwid.AttrMap(E(urwid.RadioButton, [cdck_base_selection, "Minisat CDCL Base"], '!q #3', help["Minisat CDCL Base"]), None, focus_map='reversed'),
	sub_menu(u'  |_ Minisat Tuning Options', [
		urwid.Text("Incrementality Mode:"),
		urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'Off'], '@q0',help['minisat_incrementality_Off']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [tinisat_solution_trimming,u'On'], '@q1',help['minisat_incrementality_On']), None, focus_map='reversed'),
		urwid.Divider(),
		menu_button(u'Back', back_callback),
	]),
	urwid.Divider(),
	urwid.Text("Solutions Handler:"),
	urwid.AttrMap(E(urwid.RadioButton, [bdd_mode, u'Using Table Solutions Handler'], '@g0', help['bdd_mode_Using Table Solutions Handler']), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.RadioButton, [bdd_mode, "Using Dumb Table Solutions Handler"], '@g2', help['bdd_mode_Using Dumb Table Solutions Handler']), None, focus_map='reversed'),
	urwid.AttrMap(E(urwid.RadioButton, [bdd_mode, "Using BDD Solutions Handler"], '@g1', help['bdd_mode_Using BDD Solutions Handler']), None, focus_map='reversed'),
	sub_menu(u'  |_ BDD handler Configuration', [
		urwid.AttrMap(E(urwid.RadioButton, [bdd_config,u'- BDD paths encoding'], '@cpaths', help['bdd_config_BDD paths encoding']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [bdd_config,u'- BDD cubes encoding'], '@ccubes', help['bdd_config_BDD cubes encoding']), None, focus_map='reversed'),
		urwid.AttrMap(E(urwid.RadioButton, [bdd_config,u'- BDD minisat encoding'], '@cminisat', help['bdd_config_BDD minisat encoding']), None, focus_map='reversed'),
		menu_button(u'Back', back_callback),
	]),
	urwid.Divider(),
	urwid.AttrMap(E.register(E(urwid.CheckBox, ["Configuration output"], "#o", help['Configuration output']), "config_output_enabled"), None, focus_map='reversed'),
	E.register(urwid.AttrMap(E.register(E(urwid.Edit, [u' |_ Configuraiton file output: '], '#v', help['Configuration file output']), "config_output"), None, focus_map='reversed'), 'config_output_attr'),
	urwid.Divider(),
	menu_button(u'Start', exit_program_start),
	urwid.Divider(),
	menu_button(u'Exit', exit_program),
])

# core class widget, hosting self nesting 'original_widget' for cascading windows
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
			elif 'label' in dir(selected_widget):
				caption_string = [ss for ss in selected_widget.label if ss in 'abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ']
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




@click.command()
@click.option('--config_input', type=click.Path(exists=True), default=None, help='The JSON formatted configuration options of the wizard')
def DagsterWizard(config_input):
	"""Wizard interface for configuring and initialising a Dagster run"""

	# load input command file (if there is) into the gui
	if config_input is not None:
		with open(config_input,"r") as f:
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
	
	global top
	top = CascadingBoxes(menu_top,"Main Menu")
	frame = urwid.Frame(body=top,footer=footer_txt, header=header_padding)
	# enter main loop, and exit if selected
	urwid.MainLoop(frame, palette).run()
	
	# exit without processing if exit button pushed
	if not outputting_command:
		return

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
	if config['#o']:
		with open(config['#v'],"w") as f:
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
	# !abcd means delete all config elements a,b,c and d if the element is false
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
						if kill_k in config_copy.keys():
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
	minisat_present = '#3' in special.keys()
	mode_number = 0
	if minisat_present:
		mode_number = 4
	elif gnovelty_present and strengthener_present:
		mode_number = 2
	elif strengthener_present:
		mode_number = 3
	elif gnovelty_present:
		mode_number = 1
	config_copy['m']=mode_number

	#format the command string
	config_keys = sorted(config_copy.keys())
	parameters = " ".join(["-{} {}".format(k,config_copy[k]) for k in config_keys])

	s = "mpirun -n {} {} {} {} {} > stdout.txt 2> stderr.txt".format(special['#0'],special['#d'], special['#q'], special['#w'], parameters)

	#output the command to be run
	print("export GLOG_v=3")
	print("export GLOG_logtostderr=true")
	print(s)
	print("python viewer.py {} stderr.txt".format(special['#q']))



if __name__ == '__main__':
	DagsterWizard()



