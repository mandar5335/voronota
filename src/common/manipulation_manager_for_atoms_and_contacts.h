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
				read_atoms(input, output);
			}
			else if(token=="restrict-atoms")
			{
				restrict_atoms(input, output);
			}
			else if(token=="write-atoms")
			{
				write_atoms(input, output);
			}
			else if(token=="read-atoms-and-contacts")
			{
				read_atoms_and_contacts(input, output);
			}
			else if(token=="construct-contacts")
			{
				construct_contacts(input, output);
			}
			else if(token=="write-atoms-and-contacts")
			{
				write_atoms_and_contacts(input, output);
			}
			else if(token=="select-atoms")
			{
				select_atoms(input, output);
			}
			else if(token=="select-contacts")
			{
				select_contacts(input, output);
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
			else if(token=="print-atoms")
			{
				print_atoms(input, output);
			}
			else if(token=="print-contacts")
			{
				print_contacts(input, output);
			}
			else if(token=="show-atoms")
			{
				show_atoms(input, output);
			}
			else if(token=="show-contacts")
			{
				show_contacts(input, output);
			}
			else if(token=="hide-atoms")
			{
				hide_atoms(input, output);
			}
			else if(token=="hide-contacts")
			{
				hide_contacts(input, output);
			}
			else if(token=="color-atoms")
			{
				color_atoms(input, output);
			}
			else if(token=="color-contacts")
			{
				color_contacts(input, output);
			}
			else
			{
				throw std::runtime_error(std::string("Unrecognized command."));
			}
		}
	}

private:
	static void read_string_considering_quotes(std::istream& input, std::string& output)
	{
		output.clear();
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

	void read_atoms(std::istringstream& input, std::ostream& output)
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
				output << "Read " << atoms_.size() << " atoms from file '" << atoms_file << "'.\n";
			}
		}
		else
		{
			throw std::runtime_error(std::string("Failed to read atoms from file."));
		}
	}

	void restrict_atoms(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		std::string restriction_expression="{}";
		bool full_residues=false;

		{
			std::string token;
			while(input.good())
			{
				input >> token;

				if(token=="sel")
				{
					read_string_considering_quotes(input, restriction_expression);
				}
				else if(token=="full-residues")
				{
					full_residues=true;
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

		const std::set<std::size_t> ids=selection_manager_.select_atoms(restriction_expression, full_residues);
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

		reset_atoms(atoms);
		output << "Restricted from " << atoms.size() << " to " << atoms_.size() << " atoms." << std::endl;
	}

	void write_atoms(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void read_atoms_and_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void construct_contacts(std::istringstream& input, std::ostream& output)
	{
		assert_atoms_availability();

		ConstructionOfContacts::construct_bundle_of_contact_information construct_bundle_of_contact_information;
		construct_bundle_of_contact_information.calculate_volumes=true;

		ConstructionOfContacts::enhance_contacts enhance_contacts;
		enhance_contacts.tag_centrality=true;
		enhance_contacts.tag_peripherial=true;

		bool render=false;
		std::string rendering_expression="{min-seq-sep 1}";

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
					read_string_considering_quotes(input, rendering_expression);
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
				draw_ids=selection_manager_.select_contacts(rendering_expression, false);
			}

			enhance_contacts(bundle_of_triangulation_information, draw_ids, bundle_of_contact_information.contacts);

			output << "Constructed " << contacts_.size() << " contacts, " << draw_ids.size() << " of them with graphics." << std::endl;
		}
		else
		{
			throw std::runtime_error(std::string("Failed to construct contacts."));
		}
	}

	void write_atoms_and_contacts(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void select_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void select_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
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

	void print_atoms(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void print_contacts(std::istringstream& input, std::ostream& output) const
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void show_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void show_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void hide_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void hide_contacts(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void color_atoms(std::istringstream& input, std::ostream& output)
	{
		throw std::runtime_error(std::string("Command not implemented.")); input.good(); output.good(); //TODO implement
	}

	void color_contacts(std::istringstream& input, std::ostream& output)
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
