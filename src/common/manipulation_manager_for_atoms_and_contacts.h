#ifndef COMMON_MANIPULATION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_
#define COMMON_MANIPULATION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_

#include "selection_manager_for_atoms_and_contacts.h"

namespace common
{

class ManipulationManagerForAtomsAndContacts
{
public:
	typedef SelectionManagerForAtomsAndContacts::Atom Atom;
	typedef SelectionManagerForAtomsAndContacts::Contact Contact;

	struct DisplayState
	{
		bool visible;
		unsigned int color;

		DisplayState() : visible(true), color(0x777777)
		{
		}

		DisplayState(const bool visible, const unsigned int color) : visible(visible), color(color)
		{
		}
	};

	ManipulationManagerForAtomsAndContacts()
	{
	}

	const std::vector<Atom>& atoms() const
	{
		return atoms_;
	}

	const std::vector<Contact>& contacts() const
	{
		return contacts_;
	}

	const std::vector<DisplayState>& atoms_display_states() const
	{
		return atoms_display_states_;
	}

	const std::vector<DisplayState>& contacts_display_states() const
	{
		return contacts_display_states_;
	}

	void execute(const std::string& command, std::ostream& output)
	{
		if(!command.empty())
		{
			std::istringstream input(command);

			std::string token;
			input >> token;

			input >> std::ws;

			if(token=="read-atoms")
			{
				command_read_atoms(input, output);
			}
			else if(token=="restrict-atoms")
			{
				command_restrict_atoms(input, output);
			}
			else if(token=="query-atoms")
			{
				command_query_atoms(input, output);
			}
			else if(token=="construct-contacts")
			{
				command_construct_contacts(input, output);
			}
			else if(token=="query-contacts")
			{
				command_query_contacts(input, output);
			}
			else if(token=="list-selections-of-atoms")
			{
				list_selections_of_atoms(input, output);
			}
			else if(token=="list-selections-of-contacts")
			{
				list_selections_of_contacts(input, output);
			}
			else if(token=="clear-selections-of-atoms")
			{
				clear_selections_of_atoms(input, output);
			}
			else if(token=="clear-selections-of-contacts")
			{
				clear_selections_of_contacts(input, output);
			}
			else
			{
				throw std::runtime_error(std::string("Unrecognized command."));
			}
		}
	}

private:
	struct SummaryOfAtoms
	{
		std::size_t number_total;
		double volume;

		SummaryOfAtoms() : number_total(0), volume(0.0)
		{
		}

		void feed(const Atom& atom)
		{
			number_total++;
			if(atom.value.props.adjuncts.count("volume")>0)
			{
				volume+=atom.value.props.adjuncts.find("volume")->second;
			}
		}
	};

	struct SummaryOfContacts
	{
		std::size_t number_total;
		std::size_t number_drawable;
		double area;

		SummaryOfContacts() : number_total(0), number_drawable(0), area(0.0)
		{
		}

		void feed(const Contact& contact)
		{
			number_total++;
			area+=contact.value.area;
			if(!contact.value.graphics.empty())
			{
				number_drawable++;
			}
		}
	};

	struct SelectionExpressionParameters
	{
		std::string expression;
		bool full_residues;

		SelectionExpressionParameters() : expression("{}"), full_residues(false)
		{
		}

		bool read(const std::string& type, std::istream& input)
		{
			if(type=="use")
			{
				read_string_considering_quotes(input, expression);
				return true;
			}
			else if(type=="full-residues")
			{
				full_residues=true;
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	static void print_summary_of_atoms(const SummaryOfAtoms& summary, std::ostream& output)
	{
		output << "count=" << summary.number_total;
		if(summary.volume>0.0)
		{
			output << " volume=" << summary.volume;
		}
	}

	static void print_summary_of_contacts(const SummaryOfContacts& summary, std::ostream& output)
	{
		output << "count=" << summary.number_total;
		output << " drawable=" << summary.number_drawable;
		output << " area=" << summary.area;
	}

	static void read_string_considering_quotes(std::istream& input, std::string& output)
	{
		input >> std::ws;
		const int c=input.peek();
		if(c==std::char_traits<char>::to_int_type('"') || c==std::char_traits<char>::to_int_type('\''))
		{
			input.get();
			output.clear();
			std::getline(input, output, std::char_traits<char>::to_char_type(c));
		}
		else
		{
			input >> output;
		}
	}

	void assert_atoms_availability() const
	{
		if(atoms_.empty())
		{
			throw std::runtime_error(std::string("No atoms available."));
		}
	}

	void assert_contacts_availability() const
	{
		if(contacts_.empty())
		{
			throw std::runtime_error(std::string("No contacts available."));
		}
	}

	void reset_atoms(std::vector<Atom>& atoms)
	{
		atoms_.swap(atoms);
		atoms_display_states_.clear();
		atoms_display_states_.resize(atoms_.size(), DisplayState(true, 0xFF7700));
		contacts_.clear();
		contacts_display_states_.clear();
		selection_manager_=SelectionManagerForAtomsAndContacts(&atoms_, 0);
	}

	void reset_contacts(std::vector<Contact>& contacts)
	{
		contacts_.swap(contacts);
		contacts_display_states_.clear();
		contacts_display_states_.resize(contacts_.size(), DisplayState(true, 0x0077FF));
		selection_manager_.set_contacts(&contacts_);
	}

	SummaryOfAtoms collect_summary_of_atoms() const
	{
		SummaryOfAtoms summary;
		for(std::vector<Atom>::const_iterator it=atoms_.begin();it!=atoms_.end();++it)
		{
			summary.feed(*it);
		}
		return summary;
	}

	SummaryOfAtoms collect_summary_of_atoms(const std::set<std::size_t>& ids) const
	{
		SummaryOfAtoms summary;
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			if((*it)<atoms_.size())
			{
				summary.feed(atoms_[*it]);
			}
			else
			{
				throw std::runtime_error(std::string("Invalid atom id encountered when summarizing atoms."));
			}
		}
		return summary;
	}

	SummaryOfContacts collect_summary_of_contacts() const
	{
		SummaryOfContacts summary;
		for(std::vector<Contact>::const_iterator it=contacts_.begin();it!=contacts_.end();++it)
		{
			summary.feed(*it);
		}
		return summary;
	}

	SummaryOfContacts collect_summary_of_contacts(const std::set<std::size_t>& ids) const
	{
		SummaryOfContacts summary;
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			if((*it)<contacts_.size())
			{
				summary.feed(contacts_[*it]);
			}
			else
			{
				throw std::runtime_error(std::string("Invalid contact id encountered when summarizing contacts."));
			}
		}
		return summary;
	}

	void command_read_atoms(std::istringstream& input, std::ostream& output)
	{
		ConstructionOfAtomicBalls::collect_atomic_balls_from_file collect_atomic_balls_from_file;
		std::string atoms_file;
		std::string radii_file;
		double default_radius=ConstructionOfAtomicBalls::collect_atomic_balls_from_file::default_default_radius();
		bool only_default_radius=false;

		{
			std::string token;
			while(input.good())
			{
				input >> token;

				if(token=="file")
				{
					input >> atoms_file;
				}
				else if(token=="radii-file")
				{
					input >> radii_file;
				}
				else if(token=="default-radius")
				{
					input >> default_radius;
				}
				else if(token=="only-default-radius")
				{
					only_default_radius=true;
				}
				else if(token=="mmcif")
				{
					collect_atomic_balls_from_file.mmcif=true;
				}
				else if(token=="heteroatoms")
				{
					collect_atomic_balls_from_file.include_heteroatoms=true;
				}
				else if(token=="hydrogens")
				{
					collect_atomic_balls_from_file.include_hydrogens=true;
				}
				else if(token=="multimodel")
				{
					collect_atomic_balls_from_file.multimodel_chains=true;
				}
				else
				{
					throw std::runtime_error(std::string("Invalid token '")+token+"'.");
				}

				if(input.fail() || token.empty())
				{
					throw std::runtime_error(std::string("Invalid command."));
				}

				input >> std::ws;
			}
		}

		if(atoms_file.empty())
		{
			throw std::runtime_error(std::string("Missing atoms file."));
		}

		if(!radii_file.empty() || only_default_radius || default_radius!=ConstructionOfAtomicBalls::collect_atomic_balls_from_file::default_default_radius())
		{
			collect_atomic_balls_from_file.set_atom_radius_assigner(default_radius, only_default_radius, radii_file);
		}

		std::vector<Atom> atoms;
		if(collect_atomic_balls_from_file(atoms_file, atoms))
		{
			if(atoms.size()<4)
			{
				throw std::runtime_error(std::string("Less than 4 atoms read."));
			}
			else
			{
				reset_atoms(atoms);
				output << "Read atoms from file '" << atoms_file << "' (";
				print_summary_of_atoms(collect_summary_of_atoms(), output);
				output << ")\n";
			}
		}
		else
		{
			throw std::runtime_error(std::string("Failed to read atoms from file."));
		}
	}

	void command_restrict_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		SelectionExpressionParameters selection_expression;

		{
			std::string token;
			while(input.good())
			{
				input >> token;

				if(!selection_expression.read(token, input))
				{
					throw std::runtime_error(std::string("Invalid token '")+token+"'.");
				}

				if(input.fail() || token.empty())
				{
					throw std::runtime_error(std::string("Invalid command."));
				}

				input >> std::ws;
			}
		}

		const std::set<std::size_t> ids=selection_manager_.select_atoms(selection_expression.expression, selection_expression.full_residues);
		if(ids.size()<4)
		{
			throw std::runtime_error(std::string("Less than 4 atoms selected."));
		}

		std::vector<Atom> atoms;
		atoms.reserve(ids.size());
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			atoms.push_back(atoms_.at(*it));
		}

		const SummaryOfAtoms old_summary=collect_summary_of_atoms();

		reset_atoms(atoms);

		output << "Restricted atoms from (";
		print_summary_of_atoms(old_summary, output);
		output << ") to (";
		print_summary_of_atoms(collect_summary_of_atoms(), output);
		output << ")\n";
	}

	void command_query_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		SelectionExpressionParameters selection_expression;
		std::string name;
		bool print=false;

		{
			std::string token;
			while(input.good())
			{
				input >> token;

				if(token=="name")
				{
					input >> name;
				}
				else if(token=="print")
				{
					print=true;
				}
				else if(!selection_expression.read(token, input))
				{
					throw std::runtime_error(std::string("Invalid token '")+token+"'.");
				}

				if(input.fail() || token.empty())
				{
					throw std::runtime_error(std::string("Invalid command."));
				}

				input >> std::ws;
			}
		}

		const std::set<std::size_t> ids=selection_manager_.select_atoms(selection_expression.expression, selection_expression.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		if(print)
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Atom& atom=atoms_[*it];
				output << atom << "\n";
			}
		}

		{
			output << "Summary of atoms: ";
			print_summary_of_atoms(collect_summary_of_atoms(ids), output);
			output << "\n";
		}

		if(!name.empty())
		{
			selection_manager_.set_atoms_selection(name, ids);
			output << "Set selection of atoms named '" << name << "'\n";
		}
	}

	void command_construct_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		ConstructionOfContacts::construct_bundle_of_contact_information construct_bundle_of_contact_information;
		construct_bundle_of_contact_information.calculate_volumes=true;

		ConstructionOfContacts::enhance_contacts enhance_contacts;
		enhance_contacts.tag_centrality=true;
		enhance_contacts.tag_peripherial=true;

		bool render=false;
		std::string rendering_selection_expression="{min-seq-sep 1}";

		{
			std::string token;
			while(input.good())
			{
				input >> token;

				if(token=="probe")
				{
					input >> construct_bundle_of_contact_information.probe;
					enhance_contacts.probe=construct_bundle_of_contact_information.probe;
				}
				else if(token=="render")
				{
					render=true;
				}
				else if(token=="render-sel")
				{
					render=true;
					read_string_considering_quotes(input, rendering_selection_expression);
				}
				else
				{
					throw std::runtime_error(std::string("Invalid token '")+token+"'.");
				}

				if(input.fail() || token.empty())
				{
					throw std::runtime_error(std::string("Invalid command."));
				}

				input >> std::ws;
			}
		}

		ConstructionOfContacts::BundleOfTriangulationInformation bundle_of_triangulation_information;
		ConstructionOfContacts::BundleOfContactInformation bundle_of_contact_information;

		if(construct_bundle_of_contact_information(common::ConstructionOfAtomicBalls::collect_plain_balls_from_atomic_balls<apollota::SimpleSphere>(atoms_), bundle_of_triangulation_information, bundle_of_contact_information))
		{
			reset_contacts(bundle_of_contact_information.contacts);

			for(std::size_t i=0;i<bundle_of_contact_information.volumes.size() && i<atoms_.size();i++)
			{
				atoms_[i].value.props.adjuncts["volume"]=bundle_of_contact_information.volumes[i];
			}

			std::set<std::size_t> draw_ids;
			if(render)
			{
				draw_ids=selection_manager_.select_contacts(rendering_selection_expression, false);
			}

			enhance_contacts(bundle_of_triangulation_information, draw_ids, contacts_);

			output << "Constructed contacts (";
			print_summary_of_contacts(collect_summary_of_contacts(), output);
			output << ")\n";
		}
		else
		{
			throw std::runtime_error(std::string("Failed to construct contacts."));
		}
	}

	void command_query_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_contacts_availability();

		SelectionExpressionParameters selection_expression;
		std::string name;
		bool print=false;

		{
			std::string token;
			while(input.good())
			{
				input >> token;

				if(token=="name")
				{
					input >> name;
				}
				else if(token=="print")
				{
					print=true;
				}
				else if(!selection_expression.read(token, input))
				{
					throw std::runtime_error(std::string("Invalid token '")+token+"'.");
				}

				if(input.fail() || token.empty())
				{
					throw std::runtime_error(std::string("Invalid command."));
				}

				input >> std::ws;
			}
		}

		const std::set<std::size_t> ids=selection_manager_.select_contacts(selection_expression.expression, selection_expression.full_residues);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No contacts selected."));
		}

		if(print)
		{
			enabled_output_of_ContactValue_graphics()=false;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const Contact& contact=contacts_[*it];
				if(contact.solvent())
				{
					output << atoms_[contact.ids[0]].crad << " " << ChainResidueAtomDescriptor::solvent();
				}
				else
				{
					output << atoms_[contact.ids[0]].crad << " " << atoms_[contact.ids[1]].crad;
				}
				output  << " " << contact.value << "\n";
			}
			enabled_output_of_ContactValue_graphics()=true;
		}

		{
			output << "Summary of contacts: ";
			print_summary_of_contacts(collect_summary_of_contacts(ids), output);
			output << "\n";
		}

		if(!name.empty())
		{
			selection_manager_.set_contacts_selection(name, ids);
			output << "Set selection of contacts named '" << name << "'\n";
		}
	}

	void list_selections_of_atoms(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void list_selections_of_contacts(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void clear_selections_of_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void clear_selections_of_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	std::vector<Atom> atoms_;
	std::vector<Contact> contacts_;
	std::vector<DisplayState> atoms_display_states_;
	std::vector<DisplayState> contacts_display_states_;
	SelectionManagerForAtomsAndContacts selection_manager_;
};

}

#endif /* COMMON_MANIPULATION_MANAGER_FOR_ATOMS_AND_CONTACTS_H_ */
