#ifndef SCRIPTING_OPERATORS_ALL_H_
#define SCRIPTING_OPERATORS_ALL_H_

#include "operators/add_figure.h"
#include "operators/add_figure_of_triangulation.h"
#include "operators/cad_score_many.h"
#include "operators/cad_score.h"
#include "operators/calculate_betweenness.h"
#include "operators/calculate_burial_depth.h"
#include "operators/cat_files.h"
#include "operators/center_atoms.h"
#include "operators/check_distance_constraint.h"
#include "operators/clash_score.h"
#include "operators/color_atoms.h"
#include "operators/color_contacts.h"
#include "operators/color_figures.h"
#include "operators/construct_contacts.h"
#include "operators/construct_triangulation.h"
#include "operators/copy_object.h"
#include "operators/delete_adjuncts_of_atoms.h"
#include "operators/delete_adjuncts_of_contacts.h"
#include "operators/delete_figures.h"
#include "operators/delete_objects.h"
#include "operators/delete_selections_of_atoms.h"
#include "operators/delete_selections_of_contacts.h"
#include "operators/delete_tags_of_atoms.h"
#include "operators/delete_tags_of_contacts.h"
#include "operators/delete_virtual_files.h"
#include "operators/describe_exposure.h"
#include "operators/download_virtual_file.h"
#include "operators/echo.h"
#include "operators/exit.h"
#include "operators/explain_command.h"
#include "operators/export_adjuncts_of_atoms_as_casp_qa_line.h"
#include "operators/export_adjuncts_of_atoms.h"
#include "operators/export_atoms_and_contacts.h"
#include "operators/export_atoms_as_pymol_cgo.h"
#include "operators/export_atoms.h"
#include "operators/export_cartoon_as_pymol_cgo.h"
#include "operators/export_contacts_as_pymol_cgo.h"
#include "operators/export_contacts.h"
#include "operators/export_figures_as_pymol_cgo.h"
#include "operators/export_global_adjuncts.h"
#include "operators/export_selection_of_atoms.h"
#include "operators/export_selection_of_contacts.h"
#include "operators/export_triangulation.h"
#include "operators/export_triangulation_voxels.h"
#include "operators/find_connected_components.h"
#include "operators/generate_residue_voromqa_energy_profile.h"
#include "operators/import_adjuncts_of_atoms.h"
#include "operators/import_contacts.h"
#include "operators/import.h"
#include "operators/import_selection_of_atoms.h"
#include "operators/import_selection_of_contacts.h"
#include "operators/list_commands.h"
#include "operators/list_figures.h"
#include "operators/list_objects.h"
#include "operators/list_selections_of_atoms.h"
#include "operators/list_selections_of_contacts.h"
#include "operators/list_virtual_files.h"
#include "operators/make_drawable_contacts.h"
#include "operators/make_undrawable_contacts.h"
#include "operators/mark_atoms.h"
#include "operators/mark_contacts.h"
#include "operators/mock.h"
#include "operators/move_atoms.h"
#include "operators/order_atoms_by_residue_id.h"
#include "operators/pick_objects.h"
#include "operators/print_atoms.h"
#include "operators/print_contacts.h"
#include "operators/print_figures.h"
#include "operators/print_global_adjuncts.h"
#include "operators/print_sequence.h"
#include "operators/print_time.h"
#include "operators/print_triangulation.h"
#include "operators/print_virtual_file.h"
#include "operators/rename_global_adjunct.h"
#include "operators/rename_object.h"
#include "operators/rename_selection_of_atoms.h"
#include "operators/rename_selection_of_contacts.h"
#include "operators/reset_time.h"
#include "operators/restrict_atoms_and_renumber_residues_by_adjunct.h"
#include "operators/restrict_atoms.h"
#include "operators/select_atoms_by_triangulation_query.h"
#include "operators/select_atoms.h"
#include "operators/select_contacts.h"
#include "operators/set_adjunct_of_atoms_by_contact_adjuncts.h"
#include "operators/set_adjunct_of_atoms_by_contact_areas.h"
#include "operators/set_adjunct_of_atoms_by_expression.h"
#include "operators/set_adjunct_of_atoms_by_residue_pooling.h"
#include "operators/set_adjunct_of_atoms_by_sequence_alignment.h"
#include "operators/set_adjunct_of_atoms.h"
#include "operators/set_adjunct_of_contacts.h"
#include "operators/set_alias.h"
#include "operators/set_atom_serials.h"
#include "operators/set_chain_name.h"
#include "operators/set_chain_names_by_guessing.h"
#include "operators/set_tag_of_atoms_by_secondary_structure.h"
#include "operators/set_tag_of_atoms.h"
#include "operators/set_tag_of_contacts.h"
#include "operators/setup_loading.h"
#include "operators/setup_voromqa.h"
#include "operators/show_atoms.h"
#include "operators/show_contacts.h"
#include "operators/show_figures.h"
#include "operators/show_objects.h"
#include "operators/source.h"
#include "operators/spectrum_atoms.h"
#include "operators/spectrum_contacts.h"
#include "operators/split_pdb_file.h"
#include "operators/summarize_linear_structure.h"
#include "operators/tournament_sort.h"
#include "operators/unset_aliases.h"
#include "operators/upload_virtual_file.h"
#include "operators/voromqa_frustration.h"
#include "operators/voromqa_global.h"
#include "operators/voromqa_interface_frustration.h"
#include "operators/voromqa_local.h"
#include "operators/voromqa_membrane_place.h"
#include "operators/zoom_by_atoms.h"
#include "operators/zoom_by_contacts.h"
#include "operators/zoom_by_objects.h"

#endif /* SCRIPTING_OPERATORS_ALL_H_ */
