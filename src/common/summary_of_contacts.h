#ifndef COMMON_SUMMARY_OF_CONTACTS_H_
#define COMMON_SUMMARY_OF_CONTACTS_H_

#include "construction_of_atomic_balls.h"
#include "construction_of_contacts.h"

namespace common
{

class SummaryOfContacts
{
public:
	typedef ConstructionOfAtomicBalls::AtomicBall Atom;
	typedef ConstructionOfContacts::Contact Contact;
	typedef ChainResidueAtomDescriptor CRAD;
	typedef ChainResidueAtomDescriptorsPair CRADsPair;

	class Conditions
	{
	public:
		std::set<std::string> tags;

		Conditions()
		{
		}

		explicit Conditions(const std::set<std::string>& tags) : tags(tags)
		{
		}

		bool operator==(const Conditions& v) const
		{
			return (tags==v.tags);
		}

		bool operator!=(const Conditions& v) const
		{
			return (!((*this)==v));
		}

		bool operator<(const Conditions& v) const
		{
			return (tags<v.tags);
		}

		friend std::ostream& operator<<(std::ostream& output, const Conditions& value)
		{
			if(value.tags.empty())
			{
				output << ".";
			}
			else
			{
				auxiliaries::IOUtilities(';').write_set(value.tags, output);
			}
			return output;
		}

		friend std::istream& operator>>(std::istream& input, Conditions& value)
		{
			std::string tags_str;
			input >> tags_str;
			if(!input.fail())
			{
				std::set<std::string> tags;
				if(!tags_str.empty() && tags_str[0]!='.')
				{
					auxiliaries::IOUtilities(';').read_string_lines_to_set(tags_str, tags);
				}
				value.tags=tags;
			}
			return input;
		}
	};

	class FullKey
	{
	public:
		CRADsPair crads;
		Conditions conditions;

		FullKey()
		{
		}

		FullKey(const CRADsPair& crads, const Conditions& conditions) :
			crads(crads),
			conditions(conditions)
		{
		}

		bool operator==(const FullKey& v) const
		{
			return (crads==v.crads && conditions==v.conditions);
		}

		bool operator!=(const FullKey& v) const
		{
			return (!((*this)==v));
		}

		bool operator<(const FullKey& v) const
		{
			if(crads<v.crads)
			{
				return true;
			}
			else if(crads==v.crads)
			{
				return (conditions<v.conditions);
			}
			return false;
		}

		friend std::ostream& operator<<(std::ostream& output, const FullKey& value)
		{
			output << value.crads << " " << value.conditions;
			return output;
		}

		friend std::istream& operator>>(std::istream& input, FullKey& value)
		{
			input >> value.crads >> value.conditions;
			return input;
		}
	};

	typedef std::map<FullKey, double> FullMap;
	typedef std::map<CRAD, double> AtomTypeMap;
	typedef std::map<Conditions, double> ConditionsMap;

	struct DerivedSumsOfAreas
	{
		double solvent;
		double nonsolvent;
		AtomTypeMap atom_type_map;
		ConditionsMap conditions_map;

		DerivedSumsOfAreas() :
			solvent(0.0),
			nonsolvent(0.0)
		{
		}
	};

	struct DerivedObservedProbabilities
	{
		double full_sum;
		double atom_type_sum;
		double conditions_sum;
		FullMap full_map;
		AtomTypeMap atom_type_map;
		ConditionsMap conditions_map;

		DerivedObservedProbabilities() :
			full_sum(0.0),
			atom_type_sum(0.0),
			conditions_sum(0.0)
		{
		}
	};

	struct DerivedPotential
	{
		struct ExpectedProbabilities
		{
			double full_sum;
			double conditions_sum;
			FullMap full_map;
			ConditionsMap conditions_map;

			ExpectedProbabilities() :
				full_sum(0.0),
				conditions_sum(0.0)
			{
			}
		};

		double min_value;
		double max_value;
		ExpectedProbabilities expected_probabilities;
		FullMap potential_map;

		DerivedPotential() :
			min_value(0.0),
			max_value(0.0)
		{
		}
	};

	static CRAD generalize_crad(const CRAD& input_crad)
	{
		CRAD crad=input_crad.without_numbering();

		if(crad.name=="OXT")
		{
			crad.name="O";
		}

		if(crad.name=="H1" || crad.name=="H2" || crad.name=="H3")
		{
			crad.name="H";
		}

		if(crad.resName=="MSE")
		{
			crad.resName="MET";
			if(crad.name=="SE")
			{
				crad.name="SD";
			}
		}

		if(crad.resName=="SEC")
		{
			crad.resName="CYS";
			if(crad.name=="SE")
			{
				crad.name="SG";
			}
		}

		if(crad.resName=="ALA")
		{
			if(crad.name=="HB1" || crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB1";
			}
		}

		if(crad.resName=="ARG")
		{
			if(crad.name=="NH1" || crad.name=="NH2")
			{
				crad.name="NH1";
			}
			else if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HG2" || crad.name=="HG3")
			{
				crad.name="HG2";
			}
			else if(crad.name=="HD2" || crad.name=="HD3")
			{
				crad.name="HD2";
			}
			else if(crad.name=="HH11" || crad.name=="HH12" || crad.name=="HH21" || crad.name=="HH22")
			{
				crad.name="HH11";
			}
		}

		if(crad.resName=="ASP")
		{
			if(crad.name=="OD1" || crad.name=="OD2")
			{
				crad.name="OD1";
			}
			else if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
		}

		if(crad.resName=="ASN")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HD21" || crad.name=="HD22")
			{
				crad.name="HD21";
			}
		}

		if(crad.resName=="CYS")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
		}

		if(crad.resName=="GLU")
		{
			if(crad.name=="OE1" || crad.name=="OE2")
			{
				crad.name="OE1";
			}
			else if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HG2" || crad.name=="HG3")
			{
				crad.name="HG2";
			}
		}

		if(crad.resName=="GLN")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HG2" || crad.name=="HG3")
			{
				crad.name="HG2";
			}
			else if(crad.name=="HE21" || crad.name=="HE22")
			{
				crad.name="HE21";
			}
		}

		if(crad.resName=="GLY")
		{
			if(crad.name=="HA2" || crad.name=="HA3")
			{
				crad.name="HA2";
			}
		}

		if(crad.resName=="HIS")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
		}

		if(crad.resName=="ILE")
		{
			if(crad.name=="HG12" || crad.name=="HG13")
			{
				crad.name="HG12";
			}
			else if(crad.name=="HG21" || crad.name=="HG22" || crad.name=="HG23")
			{
				crad.name="HG21";
			}
			else if(crad.name=="HD11" || crad.name=="HD12" || crad.name=="HD13")
			{
				crad.name="HD11";
			}
		}

		if(crad.resName=="LEU")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HD11" || crad.name=="HD12" || crad.name=="HD13")
			{
				crad.name="HD11";
			}
			else if(crad.name=="HD21" || crad.name=="HD22" || crad.name=="HD23")
			{
				crad.name="HD21";
			}
		}

		if(crad.resName=="LYS")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HG2" || crad.name=="HG3")
			{
				crad.name="HG2";
			}
			else if(crad.name=="HD2" || crad.name=="HD3")
			{
				crad.name="HD2";
			}
			else if(crad.name=="HE2" || crad.name=="HE3")
			{
				crad.name="HE2";
			}
			else if(crad.name=="HZ1" || crad.name=="HZ2" || crad.name=="HZ3")
			{
				crad.name="HZ1";
			}
		}

		if(crad.resName=="MET")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HG2" || crad.name=="HG3")
			{
				crad.name="HG2";
			}
			else if(crad.name=="HE1" || crad.name=="HE2" || crad.name=="HE3")
			{
				crad.name="HE1";
			}
		}

		if(crad.resName=="PHE")
		{
			if(crad.name=="CD1" || crad.name=="CD2")
			{
				crad.name="CD1";
			}
			else if(crad.name=="CE1" || crad.name=="CE2")
			{
				crad.name="CE1";
			}
			else if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HD1" || crad.name=="HD2")
			{
				crad.name="HD1";
			}
			else if(crad.name=="HE1" || crad.name=="HE2")
			{
				crad.name="HE1";
			}
		}

		if(crad.resName=="PRO")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HG2" || crad.name=="HG3")
			{
				crad.name="HG2";
			}
			else if(crad.name=="HD2" || crad.name=="HD3")
			{
				crad.name="HD2";
			}
		}

		if(crad.resName=="SER")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
		}

		if(crad.resName=="THR")
		{
			if(crad.name=="HG21" || crad.name=="HG22" || crad.name=="HG23")
			{
				crad.name="HG21";
			}
		}

		if(crad.resName=="TRP")
		{
			if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
		}

		if(crad.resName=="TYR")
		{
			if(crad.name=="CD1" || crad.name=="CD2")
			{
				crad.name="CD1";
			}
			else if(crad.name=="CE1" || crad.name=="CE2")
			{
				crad.name="CE1";
			}
			else if(crad.name=="HB2" || crad.name=="HB3")
			{
				crad.name="HB2";
			}
			else if(crad.name=="HD1" || crad.name=="HD2")
			{
				crad.name="HD1";
			}
			else if(crad.name=="HE1" || crad.name=="HE2")
			{
				crad.name="HE1";
			}
		}

		if(crad.resName=="VAL")
		{
			if(crad.name=="HG11" || crad.name=="HG12" || crad.name=="HG13")
			{
				crad.name="HG11";
			}
			else if(crad.name=="HG21" || crad.name=="HG22" || crad.name=="HG23")
			{
				crad.name="HG21";
			}
		}

		return crad;
	}

	SummaryOfContacts() :
		ignored_seq_sep_max_(1),
		far_seq_sep_min_(6)
	{
		auxiliaries::IOUtilities(';').read_string_lines_to_set("LEU;ALA;GLY;VAL;GLU;SER;LYS;ILE;ASP;THR;ARG;PRO;ASN;PHE;GLN;TYR;HIS;MET;TRP;CYS;MSE", allowed_residue_names_);
	}

	const FullMap& get_full_map_of_areas() const
	{
		return full_map_of_areas_;
	}

	void read(std::istream& input)
	{
		input >> std::ws;
		while(input.good())
		{
			FullKey key;
			double area=0.0;
			input >> key >> area;
			if(!input.fail() && area>0.0)
			{
				add(key.crads.a, key.crads.b, key.conditions, area, 1.0);
			}
			input >> std::ws;
		}
	}

	void write(std::ostream& output) const
	{
		auxiliaries::IOUtilities().write_map(full_map_of_areas_, output);
	}

	void add(const CRAD& crad_a, const CRAD& crad_b, const Conditions& conditions, const double area, const double multiplier)
	{
		if(crad_a==CRAD::solvent() && crad_b==CRAD::solvent())
		{
			return;
		}

		if(crad_a==CRAD::any() || crad_b==CRAD::any())
		{
			return;
		}

		if(crad_a!=CRAD::solvent() && allowed_residue_names_.count(crad_a.resName)==0)
		{
			return;
		}

		if(crad_b!=CRAD::solvent() && allowed_residue_names_.count(crad_b.resName)==0)
		{
			return;
		}

		if(CRAD::match_with_sequence_separation_interval(crad_a, crad_b, 0, ignored_seq_sep_max_, false))
		{
			return;
		}

		FullKey key(ChainResidueAtomDescriptorsPair(generalize_crad(crad_a), generalize_crad(crad_b)), conditions);

		if(crad_b==CRAD::solvent() || crad_a==CRAD::solvent())
		{
			key.conditions.tags.insert("solvent");
		}
		else
		{
			key.conditions.tags.erase("solvent");
		}

		if(key.conditions.tags.count("far")==0 && key.conditions.tags.count("near")==0 && key.conditions.tags.count("solvent")==0)
		{
			if(CRAD::match_with_sequence_separation_interval(crad_a, crad_b, far_seq_sep_min_, CRAD::null_num(), true))
			{
				key.conditions.tags.insert("far");
			}
			else
			{
				key.conditions.tags.insert("near");
			}
		}

		double& accumulated_value=full_map_of_areas_[key];
		accumulated_value+=area*multiplier;
		if(accumulated_value<=0.0)
		{
			full_map_of_areas_.erase(key);
		}
	}

	void add(const std::vector<Atom>& atoms, const std::vector<Contact>& contacts, const std::size_t contact_id, const double multiplier)
	{
		if(contact_id>=contacts.size())
		{
			return;
		}

		const Contact& contact=contacts[contact_id];

		if(contact.ids[0]>=atoms.size() || contact.ids[1]>=atoms.size())
		{
			return;
		}

		if(contact.solvent())
		{
			add(atoms[contact.ids[0]].crad, CRAD::solvent(), Conditions(contact.value.props.tags), contact.value.area, multiplier);
		}
		else
		{
			add(atoms[contact.ids[0]].crad, atoms[contact.ids[1]].crad, Conditions(contact.value.props.tags), contact.value.area, multiplier);
		}
	}

	void add(const std::vector<Atom>& atoms, const std::vector<Contact>& contacts, const double multiplier)
	{
		for(std::size_t i=0;i<contacts.size();i++)
		{
			add(atoms, contacts, i, multiplier);
		}
	}

	DerivedSumsOfAreas derive_sums_of_areas() const
	{
		DerivedSumsOfAreas sums_of_areas;
		for(FullMap::const_iterator it=full_map_of_areas_.begin();it!=full_map_of_areas_.end();++it)
		{
			const FullKey& key=it->first;
			const double area=it->second;
			if(key.conditions.tags.count("solvent")>0)
			{
				sums_of_areas.solvent+=area;
			}
			else
			{
				sums_of_areas.nonsolvent+=area;
			}

			if(key.crads.a!=CRAD::solvent())
			{
				sums_of_areas.atom_type_map[key.crads.a]+=area;
			}
			if(key.crads.b!=CRAD::solvent())
			{
				sums_of_areas.atom_type_map[key.crads.b]+=area;
			}

			sums_of_areas.conditions_map[key.conditions]+=area;
		}
		return sums_of_areas;
	}

	DerivedObservedProbabilities derive_observed_probabilities() const
	{
		const DerivedSumsOfAreas sums_of_areas=derive_sums_of_areas();
		DerivedObservedProbabilities observed_probabilities;
		for(FullMap::const_iterator it=full_map_of_areas_.begin();it!=full_map_of_areas_.end();++it)
		{
			const double value=(it->second)/(sums_of_areas.solvent+sums_of_areas.nonsolvent);
			observed_probabilities.full_sum+=value;
			observed_probabilities.full_map[it->first]=value;
		}
		for(AtomTypeMap::const_iterator it=sums_of_areas.atom_type_map.begin();it!=sums_of_areas.atom_type_map.end();++it)
		{
			const double value=(it->second)/(sums_of_areas.solvent+(sums_of_areas.nonsolvent*2.0));
			observed_probabilities.atom_type_sum+=value;
			observed_probabilities.atom_type_map[it->first]=value;
		}
		for(ConditionsMap::const_iterator it=sums_of_areas.conditions_map.begin();it!=sums_of_areas.conditions_map.end();++it)
		{
			const double value=(it->second)/(sums_of_areas.solvent+sums_of_areas.nonsolvent);
			observed_probabilities.conditions_sum+=value;
			observed_probabilities.conditions_map[it->first]=value;
		}
		return observed_probabilities;
	}

	DerivedPotential derive_potential(const ConditionsMap& custom_expected_probabilities_conditions_map) const
	{
		DerivedObservedProbabilities observed_probabilities=derive_observed_probabilities();

		DerivedPotential potential;

		if(custom_expected_probabilities_conditions_map.empty())
		{
			potential.expected_probabilities.conditions_map=observed_probabilities.conditions_map;
		}
		else
		{
			potential.expected_probabilities.conditions_map=custom_expected_probabilities_conditions_map;
		}

		for(FullMap::const_iterator it=observed_probabilities.full_map.begin();it!=observed_probabilities.full_map.end();++it)
		{
			const FullKey& key=it->first;

			double p_exp=0.0;

			if(key.conditions.tags.count("solvent")>0)
			{
				p_exp=observed_probabilities.atom_type_map[key.crads.a!=CRAD::solvent() ? key.crads.a : key.crads.b]*potential.expected_probabilities.conditions_map[key.conditions];
			}
			else
			{
				p_exp=observed_probabilities.atom_type_map[key.crads.a]*observed_probabilities.atom_type_map[key.crads.b]*potential.expected_probabilities.conditions_map[key.conditions];
				if(key.crads.a!=key.crads.b)
				{
					p_exp=p_exp*2.0;
				}
			}

			if(p_exp>0.0)
			{
				potential.expected_probabilities.full_sum+=p_exp;
				potential.expected_probabilities.full_map[key]=p_exp;

				const double p_obs=it->second;
				if(p_obs>0.0)
				{
					const double value=log(p_exp/p_obs);
					if(value<potential.min_value || potential.potential_map.empty())
					{
						potential.min_value=value;
					}
					if(value>potential.max_value || potential.potential_map.empty())
					{
						potential.max_value=value;
					}
					potential.potential_map[key]=value;
				}
			}
		}

		for(ConditionsMap::const_iterator it=potential.expected_probabilities.conditions_map.begin();it!=potential.expected_probabilities.conditions_map.end();++it)
		{
			potential.expected_probabilities.conditions_sum+=(it->second);
		}

		return potential;
	}

	DerivedPotential derive_potential() const
	{
		return derive_potential(ConditionsMap());
	}

private:
	int ignored_seq_sep_max_;
	int far_seq_sep_min_;
	std::set<std::string> allowed_residue_names_;
	FullMap full_map_of_areas_;
};

}

#endif /* COMMON_SUMMARY_OF_CONTACTS_H_ */
