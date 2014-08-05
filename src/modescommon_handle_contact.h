#ifndef MODESCOMMON_HANDLE_CONTACT_H_
#define MODESCOMMON_HANDLE_CONTACT_H_

#include "auxiliaries/chain_residue_atom_comment.h"
#include "auxiliaries/io_utilities.h"

namespace modescommon
{

namespace contact
{

typedef auxiliaries::ChainResidueAtomComment Comment;

struct ContactValue
{
	double area;
	double dist;
	std::string graphics;

	ContactValue() : area(0.0), dist(0.0)
	{
	}

	void add(const ContactValue& v)
	{
		area+=v.area;
		dist=(dist<=0.0 ? v.dist : std::min(dist, v.dist));
		graphics+=v.graphics;
	}
};

inline void print_contact_record(const std::pair<Comment, Comment>& comments, const ContactValue& value, const bool preserve_graphics, std::ostream& output)
{
	output << comments.first.str() << " " << comments.second.str() << " " << value.area << " " << value.dist;
	if(preserve_graphics && !value.graphics.empty())
	{
		if(value.graphics[0]!=' ')
		{
			output << " ";
		}
		output << value.graphics;
	}
	output << "\n";
}

template<typename T>
inline T refine_pair(const T& p, const bool reverse)
{
	if(reverse)
	{
		return T(p.second, p.first);
	}
	else
	{
		return p;
	}
}

inline bool add_contacts_record_from_stream_to_map(std::istream& input, std::map< std::pair<Comment, Comment>, ContactValue >& map_of_records)
{
	std::pair<std::string, std::string> comment_strings;
	ContactValue value;
	input >> comment_strings.first >> comment_strings.second >> value.area >> value.dist;
	if(input.good())
	{
		std::getline(input, value.graphics);
	}
	if(!input.fail() && !comment_strings.first.empty() && !comment_strings.second.empty())
	{
		const std::pair<Comment, Comment> comments(Comment::from_str(comment_strings.first), Comment::from_str(comment_strings.second));
		if(comments.first.valid() && comments.second.valid())
		{
			map_of_records[refine_pair(comments, comments.second<comments.first)]=value;
			return true;
		}
	}
	return false;
}

inline bool add_contacts_name_pair_from_stream_to_set(std::istream& input, std::set< std::pair<Comment, Comment> >& set_of_name_pairs)
{
	std::pair<std::string, std::string> comment_strings;
	input >> comment_strings.first >> comment_strings.second;
	if(!input.fail() && !comment_strings.first.empty() && !comment_strings.second.empty())
	{
		const std::pair<Comment, Comment> comments(Comment::from_str(comment_strings.first), Comment::from_str(comment_strings.second));
		if(comments.first.valid() && comments.second.valid())
		{
			set_of_name_pairs.insert(refine_pair(comments, comments.second<comments.first));
			return true;
		}
	}
	return false;
}

}

}

#endif /* MODESCOMMON_HANDLE_CONTACT_H_ */
