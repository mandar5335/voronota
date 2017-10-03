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
	manager.execute_plainly("print-atoms use '{tags het adjuncts tf=0:10}'", std::cout);
	manager.execute_plainly("restrict-atoms use '{tags-not het}'", std::cout);
	manager.execute_plainly("construct-contacts render-default calculate-volumes", std::cout);
	manager.execute_plainly("save-atoms file 'tmp/plain_atoms.txt'", std::cout);
	manager.execute_plainly("save-contacts file 'tmp/plain_contacts.txt'", std::cout);
	manager.execute_plainly("load-atoms file 'tmp/plain_atoms.txt' format plain", std::cout);
	manager.execute_plainly("select-contacts", std::cout);
	manager.execute_plainly("load-contacts file 'tmp/plain_contacts.txt'", std::cout);
	manager.execute_plainly("select-contacts use '{atom-first {match R<PHE>} atom-second {match R<PHE>} min-area 5.0 min-seq-sep 1}' name cs1", std::cout);
	manager.execute_plainly("print-contacts use '{selection cs1}' sort-r area", std::cout);
	manager.execute_plainly("print-contacts use '{no-solvent min-seq-sep 2}' sort-r area limit 3 expand", std::cout);
	manager.execute_plainly("print-contacts use '{no-solvent min-seq-sep 2}' sort-r area limit 3 expand inter-residue", std::cout);
	manager.execute_plainly("select-atoms use '{match r<64>&A<C,N,O,CA,CB>}' name as1", std::cout);
	manager.execute_plainly("print-atoms use '{selection as1}' sort tags", std::cout);
	manager.execute_plainly("print-atoms use '{match r<64>&A<C,N,O,CA,CB>}' sort atmn expand", std::cout);
	manager.execute_plainly("rename-selection-of-atoms nosel1 nodel2", std::cout);
	manager.execute_plainly("delete-selections-of-contacts nosel1", std::cout);
	manager.execute_plainly("list-selections-of-atoms", std::cout);
	manager.execute_plainly("list-selections-of-contacts", std::cout);
	manager.execute_plainly("print-history last 5", std::cout);
	manager.execute_plainly("print-history successful", std::cout);
}
