#ifndef SCRIPTING_OPERATORS_SPECTRUM_CONTACTS_H_
#define SCRIPTING_OPERATORS_SPECTRUM_CONTACTS_H_

#include "common.h"

namespace scripting
{

namespace operators
{

class SpectrumContacts
{
public:
	struct Result
	{
		SummaryOfContacts contacts_summary;
		double min_value;
		double max_value;
		int number_of_values;
		double mean_of_values;
		double sd_of_values;

		const Result& write(HeterogeneousStorage& heterostorage) const
		{
			VariantSerialization::write(contacts_summary, heterostorage.variant_object.object("contacts_summary"));
			VariantObject& info=heterostorage.variant_object.object("spectrum_summary");
			info.value("min_value")=min_value;
			info.value("max_value")=max_value;
			info.value("number_of_values")=number_of_values;
			info.value("mean_of_values")=mean_of_values;
			info.value("sd_of_values")=sd_of_values;
			return (*this);
		}
	};

	SelectionManager::Query parameters_for_selecting;
	std::vector<std::string> representation_names;
	std::string adjunct;
	std::string by;
	std::string scheme;
	bool as_z_scores;
	bool min_val_present;
	double min_val;
	bool max_val_present;
	double max_val;
	bool only_summarize;

	SpectrumContacts() : as_z_scores(false), min_val_present(false), min_val(0.0), max_val_present(false), max_val(1.0), only_summarize(false)
	{
	}

	SpectrumContacts& init(CommandInput& input)
	{
		parameters_for_selecting=Utilities::read_generic_selecting_query(input);
		representation_names=input.get_value_vector_or_default<std::string>("rep", std::vector<std::string>());
		adjunct=input.get_value_or_default<std::string>("adjunct", "");
		by=adjunct.empty() ? input.get_value<std::string>("by") : std::string("adjunct");
		scheme=input.get_value_or_default<std::string>("scheme", "reverse-rainbow");
		as_z_scores=input.get_flag("as-z-scores");
		min_val_present=input.is_option("min-val");
		min_val=input.get_value_or_default<double>("min-val", (as_z_scores ? -2.0 : 0.0));
		max_val_present=input.is_option("max-val");
		max_val=input.get_value_or_default<double>("max-val", (as_z_scores ? 2.0 : 1.0));
		only_summarize=input.get_flag("only-summarize");
		return (*this);
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_contacts_availability();
		data_manager.assert_contacts_representations_availability();

		const std::set<std::size_t> representation_ids=data_manager.contacts_representation_descriptor().ids_by_names(representation_names);

		if(by!="area" && by!="adjunct" && by!="dist-centers" && by!="dist-balls" && by!="seq-sep")
		{
			throw std::runtime_error(std::string("Invalid 'by' value '")+by+"'.");
		}

		if(by=="adjunct" && adjunct.empty())
		{
			throw std::runtime_error(std::string("No adjunct name provided."));
		}

		if(by!="adjunct" && !adjunct.empty())
		{
			throw std::runtime_error(std::string("Adjunct name provided when coloring not by adjunct."));
		}

		if(!auxiliaries::ColorUtilities::color_valid(auxiliaries::ColorUtilities::color_from_gradient(scheme, 0.5)))
		{
			throw std::runtime_error(std::string("Invalid 'scheme' value '")+scheme+"'.");
		}

		if(min_val_present && max_val_present && max_val<=min_val)
		{
			throw std::runtime_error(std::string("Minimum and maximum values do not define range."));
		}

		const std::set<std::size_t> ids=data_manager.filter_contacts_drawable_implemented_ids(
				representation_ids,
				data_manager.selection_manager().select_contacts(parameters_for_selecting),
				false);

		if(ids.empty())
		{
			throw std::runtime_error(std::string("No drawable contacts selected."));
		}

		std::map<std::size_t, double> map_of_ids_values;

		if(by=="adjunct")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::map<std::string, double>& adjuncts=data_manager.contacts()[*it].value.props.adjuncts;
				std::map<std::string, double>::const_iterator jt=adjuncts.find(adjunct);
				if(jt!=adjuncts.end())
				{
					map_of_ids_values[*it]=jt->second;
				}
			}
		}
		else if(by=="area")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=data_manager.contacts()[*it].value.area;
			}
		}
		else if(by=="dist-centers")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				map_of_ids_values[*it]=data_manager.contacts()[*it].value.dist;
			}
		}
		else if(by=="dist-balls")
		{
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id0=data_manager.contacts()[*it].ids[0];
				const std::size_t id1=data_manager.contacts()[*it].ids[1];
				if(data_manager.contacts()[*it].solvent())
				{
					map_of_ids_values[*it]=(data_manager.contacts()[*it].value.dist-data_manager.atoms()[id0].value.r)/3.0*2.0;
				}
				else
				{
					map_of_ids_values[*it]=apollota::minimal_distance_from_sphere_to_sphere(data_manager.atoms()[id0].value, data_manager.atoms()[id1].value);
				}
			}
		}
		else if(by=="seq-sep")
		{
			double max_seq_sep=0.0;
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id0=data_manager.contacts()[*it].ids[0];
				const std::size_t id1=data_manager.contacts()[*it].ids[1];
				if(data_manager.atoms()[id0].crad.chainID==data_manager.atoms()[id1].crad.chainID)
				{
					const double seq_sep=fabs(static_cast<double>(data_manager.atoms()[id0].crad.resSeq-data_manager.atoms()[id1].crad.resSeq));
					map_of_ids_values[*it]=seq_sep;
					max_seq_sep=((max_seq_sep<seq_sep) ? seq_sep : max_seq_sep);
				}
			}
			for(std::set<std::size_t>::const_iterator it=ids.begin();it!=ids.end();++it)
			{
				const std::size_t id0=data_manager.contacts()[*it].ids[0];
				const std::size_t id1=data_manager.contacts()[*it].ids[1];
				if(data_manager.atoms()[id0].crad.chainID!=data_manager.atoms()[id1].crad.chainID)
				{
					map_of_ids_values[*it]=max_seq_sep+1.0;
				}
			}
		}

		if(map_of_ids_values.empty())
		{
			throw std::runtime_error(std::string("Nothing colorable."));
		}

		double min_val_actual=0.0;
		double max_val_actual=0.0;
		int num_of_vals=0;
		double mean_of_values=0.0;
		double sd_of_values=0.0;

		Utilities::calculate_spectrum_info(
				as_z_scores,
				min_val_present,
				min_val,
				max_val_present,
				max_val,
				min_val_actual,
				max_val_actual,
				num_of_vals,
				mean_of_values,
				sd_of_values,
				map_of_ids_values);

		if(!only_summarize)
		{
			DataManager::DisplayStateUpdater dsu;
			dsu.visual_ids=representation_ids;

			for(std::map<std::size_t, double>::const_iterator it=map_of_ids_values.begin();it!=map_of_ids_values.end();++it)
			{
				dsu.color=auxiliaries::ColorUtilities::color_from_gradient(scheme, it->second);
				data_manager.update_contacts_display_state(dsu, it->first);
			}
		}

		Result result;
		result.contacts_summary=SummaryOfContacts(data_manager.contacts(), ids);
		result.min_value=min_val_actual;
		result.max_value=max_val_actual;
		result.number_of_values=num_of_vals;
		result.mean_of_values=mean_of_values;
		result.sd_of_values=sd_of_values;

		return result;
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_SPECTRUM_CONTACTS_H_ */
