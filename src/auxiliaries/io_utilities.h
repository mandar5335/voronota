#ifndef AUXILIARIES_IO_UTILITIES__H_
#define AUXILIARIES_IO_UTILITIES__H_

#include <iostream>
#include <sstream>
#include <list>
#include <set>
#include <map>
#include <stdexcept>

namespace auxiliaries
{

template<typename LineReader, typename Container>
inline void read_lines_to_container(
		std::istream& input,
		const std::string& comments_marker,
		LineReader line_reader,
		Container& container,
		std::list< std::pair<std::size_t, std::string> >* comments_list_ptr=0)
{
	std::size_t line_num=0;
	while(input.good())
	{
		std::string line;
		std::getline(input, line);
		if(!comments_marker.empty())
		{
			const std::size_t comments_pos=line.find(comments_marker, 0);
			if(comments_pos!=std::string::npos)
			{
				if(comments_list_ptr!=0)
				{
					comments_list_ptr->push_back(std::make_pair(line_num, line.substr(comments_pos)));
				}
				line=line.substr(0, comments_pos);
			}
		}
		if(!line.empty())
		{
			std::istringstream line_input(line);
			if(!line_reader(line_input, container))
			{
				throw std::runtime_error(std::string("Failed to read line '")+line+"'.");
			}
			line_num++;
		}
	}
}

template<typename T>
inline std::set<T> read_set_from_string(const std::string& input_str, const std::string& separators)
{
	std::set<T> result;
	if(input_str.find_first_not_of(separators)!=std::string::npos)
	{
		std::string str=input_str;
		for(std::size_t i=0;i<str.size();i++)
		{
			if(separators.find(str[i])!=std::string::npos)
			{
				str[i]=' ';
			}
		}
		std::istringstream input(str);
		while(input.good())
		{
			std::string token;
			input >> token;
			if(!token.empty())
			{
				result.insert(token);
			}
		}
	}
	return result;
}

template<typename T>
inline std::string print_set_to_string(const std::set<T>& set, const std::string& sep)
{
	std::ostringstream output;
	for(typename std::set<T>::const_iterator it=set.begin();it!=set.end();++it)
	{
		output << (it==set.begin() ? std::string() : sep) << (*it);
	}
	return output.str();
}

}

#endif /* AUXILIARIES_IO_UTILITIES__H_ */
