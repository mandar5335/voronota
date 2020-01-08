#ifndef SCRIPTING_OPERATORS_EXPORT_ATOMS_H_
#define SCRIPTING_OPERATORS_EXPORT_ATOMS_H_

#include "../operators_common.h"
#include "../../common/writing_atomic_balls_in_pdb_format.h"

namespace scripting
{

namespace operators
{

class ExportAtoms : public OperatorBase<ExportAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::string file;
		std::string dump;
		SummaryOfAtoms atoms_summary;

		void store(HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("file")=file;
			if(!dump.empty())
			{
				heterostorage.variant_object.value("dump")=dump;
			}
			VariantSerialization::write(atoms_summary, heterostorage.variant_object.object("atoms_summary"));
		}
	};

    std::string file;
    SelectionManager::Query parameters_for_selecting;
    bool as_pdb;
	std::string pdb_b_factor_name;
	bool pdb_ter;

	ExportAtoms() : file(""), as_pdb(false), pdb_b_factor_name("tf"), pdb_ter(false)
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		assert_file_name_input(file, false);
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		as_pdb=input.get_flag("as-pdb");
		pdb_b_factor_name=input.get_value_or_default<std::string>("pdb-b-factor", "tf");
		pdb_ter=input.get_flag("pdb-ter");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		Utilities::document_read_generic_selecting_query(doc);
		doc.set_option_decription(CDOD("as-pdb", CDOD::DATATYPE_BOOL, "flag to output in PDB format"));
		doc.set_option_decription(CDOD("pdb-b-factor", CDOD::DATATYPE_STRING, "name of adjunct values to write as b-factors in PDB output", "tf"));
		doc.set_option_decription(CDOD("pdb-ter", CDOD::DATATYPE_BOOL, "flag to include TER lines in PDB output"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		assert_file_name_input(file, false);

		const std::set<std::size_t> ids=data_manager.selection_manager().select_atoms(parameters_for_selecting);
		if(ids.empty())
		{
			throw std::runtime_error(std::string("No atoms selected."));
		}

		OutputSelector output_selector(file);
		std::ostream& output=output_selector.stream();
		assert_io_stream(file, output);

		const std::vector<Atom> atoms=slice_vector_by_ids(data_manager.atoms(), ids);

		if(as_pdb)
		{
			common::WritingAtomicBallsInPDBFormat::write_atomic_balls(atoms, pdb_b_factor_name, pdb_ter, output);
		}
		else
		{
			auxiliaries::IOUtilities().write_set(atoms, output);
		}

		Result result;
		result.file=file;
		if(output_selector.location_type()==OutputSelector::TEMPORARY_MEMORY)
		{
			result.dump=output_selector.str();
		}
		result.atoms_summary=SummaryOfAtoms(atoms);

		return result;
	}

private:
	template<typename T>
	static T slice_vector_by_ids(const T& full_vector, const std::set<std::size_t>& ids)
	{
		T sliced_vector;
		sliced_vector.reserve(ids.size());
		for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
		{
			sliced_vector.push_back(full_vector.at(*it));
		}
		return sliced_vector;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_EXPORT_ATOMS_H_ */

