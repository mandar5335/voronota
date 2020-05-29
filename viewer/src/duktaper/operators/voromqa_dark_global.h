#ifndef VIEWER_OPERATORS_VOROMQA_DARK_GLOBAL_H_
#define VIEWER_OPERATORS_VOROMQA_DARK_GLOBAL_H_

#include "../../../../src/scripting/operators_all.h"

#include "../stocked_data_resources.h"

#include "nnport_predict.h"

namespace voronota
{

namespace duktaper
{

namespace operators
{

class VoroMQADarkGlobal : public scripting::operators::OperatorBase<VoroMQADarkGlobal>
{
public:
	struct Result : public scripting::operators::OperatorResultBase<Result>
	{
		double global_score;
		int number_of_residues;

		Result() : global_score(0.0), number_of_residues(0)
		{
		}

		void store(scripting::HeterogeneousStorage& heterostorage) const
		{
			heterostorage.variant_object.value("global_score")=global_score;
			heterostorage.variant_object.value("number_of_residues")=number_of_residues;
		}
	};

	std::string global_adj_prefix;

	VoroMQADarkGlobal()
	{
	}

	void initialize(scripting::CommandInput& input)
	{
		global_adj_prefix=input.get_value_or_default<std::string>("global-adj-prefix", "voromqa_dark_global");
	}

	void document(scripting::CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("global-adj-prefix", CDOD::DATATYPE_STRING, "prefix for output global adjuncts", "voromqa_dark_global"));
	}

	Result run(scripting::DataManager& data_manager) const
	{
		scripting::VirtualFileStorage::TemporaryFile tmp_profile;
		scripting::VirtualFileStorage::TemporaryFile tmp_scores;

		{
			std::ostringstream args;
			args << "-file " << tmp_profile.filename();
			scripting::operators::GenerateResidueVoroMQAEnergyProfile().init(args.str()).run(data_manager);
		}

		{
			const std::string filename_for_voromqa_dark_nnport_input_header=scripting::VirtualFileStorage::validate_filename("voromqa_dark_nnport_input_header");
			if(!scripting::VirtualFileStorage::file_exists(filename_for_voromqa_dark_nnport_input_header))
			{
				voronota::scripting::VirtualFileStorage::set_file(filename_for_voromqa_dark_nnport_input_header, resources::data_voromqa_dark_nnport_input_header(), true);
			}

			const std::string filename_for_voromqa_dark_nnport_input_statistics=scripting::VirtualFileStorage::validate_filename("voromqa_dark_nnport_input_statistics");
			if(!scripting::VirtualFileStorage::file_exists(filename_for_voromqa_dark_nnport_input_statistics))
			{
				voronota::scripting::VirtualFileStorage::set_file(filename_for_voromqa_dark_nnport_input_statistics, resources::data_voromqa_dark_nnport_input_statistics(), true);
			}

			const std::string filename_for_voromqa_dark_nnport_input_fdeep_model_json=scripting::VirtualFileStorage::validate_filename("voromqa_dark_nnport_input_fdeep_model_json");
			if(!scripting::VirtualFileStorage::file_exists(filename_for_voromqa_dark_nnport_input_fdeep_model_json))
			{
				voronota::scripting::VirtualFileStorage::set_file(filename_for_voromqa_dark_nnport_input_fdeep_model_json, resources::data_voromqa_dark_nnport_input_fdeep_model_json(), true);
			}

			std::ostringstream args;
			args << "-input-value-column-names-file " << filename_for_voromqa_dark_nnport_input_header << " ";
			args << "-input-statistics-file " << filename_for_voromqa_dark_nnport_input_statistics << " ";
			args << "-input-model-files " << filename_for_voromqa_dark_nnport_input_fdeep_model_json << " ";
			args << "-output-value-column-names vd1 vd2 vd3 vd4 vd5 vd6 ";
			args << "-input-data-file " << tmp_profile.filename() << " ";
			args << "-output-data-file " << tmp_scores.filename() << " ";
			operators::NNPortPredict().init(args.str()).run(0);
		}

		{
			std::ostringstream args;
			args << "-file " << tmp_scores.filename();
			scripting::operators::ImportAdjunctsOfAtoms().init(args.str()).run(data_manager);
		}

		const scripting::operators::SpectrumAtoms::Result spectrum_result=scripting::operators::SpectrumAtoms().init("-use [-aname CA] -adjunct vd1 -only-summarize").run(data_manager);

		Result result;
		result.global_score=spectrum_result.mean_of_values;
		result.number_of_residues=spectrum_result.number_of_values;

		if(!global_adj_prefix.empty())
		{
			data_manager.global_numeric_adjuncts_mutable()[global_adj_prefix+"_quality_score"]=result.global_score;
		}

		return result;
	}
};

}

}

}

#endif /* VIEWER_OPERATORS_VOROMQA_DARK_GLOBAL_H_ */
