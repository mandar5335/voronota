#include "auxiliaries/program_options_handler.h"

#include "common/manipulation_manager_for_atoms_and_contacts.h"

void demo_apilike_functionality(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "none");
	pohw.describe_io("stdout", false, true, "demo output");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	std::cout << "\n";

	common::ManipulationManagerForAtomsAndContacts manager;
	manager.execute_plainly("load-atoms file tests/input/single/structure", std::cout);
	manager.execute_plainly("load-atoms file tests/input/single/structure.cif format mmcif", std::cout);
	manager.execute_plainly("load-atoms file tests/input/single/structure.pdb", std::cout);
	manager.execute_plainly("load-atoms file tests/input/single/structure.pdb include-heteroatoms", std::cout);
	manager.execute_plainly("query-atoms use '{tags het adjuncts tf=0:10}' print", std::cout);
	manager.execute_plainly("restrict-atoms use '{tags-not het}'", std::cout);
	manager.execute_plainly("construct-contacts render-default calculate-volumes", std::cout);
	manager.execute_plainly("save-atoms file 'tmp/plain_atoms.txt'", std::cout);
	manager.execute_plainly("save-contacts file 'tmp/plain_contacts.txt'", std::cout);
	manager.execute_plainly("load-atoms file 'tmp/plain_atoms.txt' format plain", std::cout);
	manager.execute_plainly("query-contacts", std::cout);
	manager.execute_plainly("load-contacts file 'tmp/plain_contacts.txt'", std::cout);
	manager.execute_plainly("query-contacts use '{atom-first {match R<PHE>} atom-second {match R<PHE>} min-area 5.0 min-seq-sep 1}' print name cs1", std::cout);
	manager.execute_plainly("query-contacts use '{no-solvent min-seq-sep 2}' print-sorted-reversed 2 print-limit 3 print-expanded", std::cout);
	manager.execute_plainly("query-contacts use '{no-solvent min-seq-sep 2}' print-sorted-reversed 2 print-limit 3 print-expanded print-inter-residue", std::cout);
	manager.execute_plainly("query-atoms use '{match r<64>&A<C,N,O,CA,CB>}' print name as1", std::cout);
	manager.execute_plainly("query-atoms use '{match r<64>&A<C,N,O,CA,CB>}' print-expanded", std::cout);
	manager.execute_plainly("rename-selection-of-atoms nosel1 nodel2", std::cout);
	manager.execute_plainly("delete-selections-of-contacts nosel1", std::cout);
	manager.execute_plainly("list-selections-of-atoms", std::cout);
	manager.execute_plainly("list-selections-of-contacts", std::cout);
}
