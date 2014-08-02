#ifndef AUXILIARIES_OPENGL_PRINTER_H_
#define AUXILIARIES_OPENGL_PRINTER_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

namespace auxiliaries
{

class OpenGLPrinter
{
public:
	OpenGLPrinter()
	{
	}

	void add(const std::string& str)
	{
		string_stream_ << " " << str;
	}

	void add_alpha(const double alpha)
	{
		string_stream_ << object_typer_.alpha << " " << alpha << " ";
	}

	void add_color(const unsigned int rgb)
	{
		string_stream_ << object_typer_.color << " ";
		write_color_to_stream(Color(rgb), string_stream_);
	}

	template<typename SphereType>
	void add_sphere(const SphereType& sphere)
	{
		string_stream_ << object_typer_.sphere << " ";
		write_point_to_stream(sphere, string_stream_);
		string_stream_.precision(3);
		string_stream_ << std::fixed << sphere.r << " ";
	}

	template<typename PointType>
	void add_triangle_strip(const std::vector<PointType>& vertices, const std::vector<PointType>& normals)
	{
		if(!vertices.empty() && vertices.size()==normals.size())
		{
			string_stream_ << object_typer_.tstrip << " ";
			write_points_vector_to_stream(vertices, string_stream_);
			write_points_vector_to_stream(normals, string_stream_);
		}
	}

	template<typename PointType>
	void add_triangle_fan(const std::vector<PointType>& vertices, const std::vector<PointType>& normals)
	{
		if(!vertices.empty() && vertices.size()==normals.size())
		{
			string_stream_ << object_typer_.tfan << " ";
			write_points_vector_to_stream(vertices, string_stream_);
			write_points_vector_to_stream(normals, string_stream_);
		}
	}

	template<typename PointType>
	void add_triangle_fan(const PointType& center, const std::vector<PointType>& vertices, const PointType& normal)
	{
		if(!vertices.empty())
		{
			string_stream_ << object_typer_.tfanc << " ";
			write_point_to_stream(center, string_stream_);
			write_point_to_stream(normal, string_stream_);
			write_points_vector_to_stream(vertices, string_stream_);
		}
	}

	void add_label(const std::string& label)
	{
		string_stream_ << object_typer_.label << " " << label << " ";
	}

	std::string str() const
	{
		return string_stream_.str();
	}

	void print_pymol_script(const std::string& obj_name, const bool two_sided_lighting, std::ostream& output)
	{
		std::istringstream input(str());
		if(!input.good())
		{
			return;
		}
		const std::string sep=", ";
		output << "from pymol.cgo import *\n";
		output << "from pymol import cmd\n";
		int sublist_id=0;
		while(input.good())
		{
			std::size_t sublist_size=0;
			output << obj_name << "_" << sublist_id << " = [";
			while(input.good() && sublist_size<10000)
			{
				std::string type_str;
				input >> type_str;
				const ObjectTypeMarker type(type_str, object_typer_);
				if(type.alpha)
				{
					double alpha=1.0;
					input >> alpha;
					output << "ALPHA, " << alpha << sep;
					sublist_size+=2;
				}
				else if(type.color)
				{
					write_color_to_stream(read_color_from_stream(input), true, "COLOR, ", sep, sep, output);
					sublist_size+=5;
				}
				else if(type.tstrip || type.tfan || type.tfanc)
				{
					std::vector<PlainPoint> vertices;
					std::vector<PlainPoint> normals;
					if(read_strip_or_fan_from_stream(type.tstrip, type.tfan, type.tfanc, input, vertices, normals))
					{
						output << (type.tstrip ? "BEGIN, TRIANGLE_STRIP, " : "BEGIN, TRIANGLE_FAN, ");
						for(std::size_t i=0;i<vertices.size();i++)
						{
							write_point_to_stream(normals[i], "NORMAL, ", sep, sep, output);
							write_point_to_stream(vertices[i], "VERTEX, ", sep, sep, output);
						}
						output << "END, ";
					}
					sublist_size+=vertices.size()*4+normals.size()*4+3;
				}
				else if(type.sphere)
				{
					const PlainPoint c=read_point_from_stream(input);
					double r;
					input >> r;
					write_point_to_stream(c, "SPHERE, ", sep, sep, output);
					output << r << sep;
					sublist_size+=5;
				}
			}
			output << "]\n";
			sublist_id++;
		}
		output << obj_name << " = " << obj_name << "_0\n";
		for(int i=1;i<sublist_id;i++)
		{
			output << obj_name << ".extend(" << obj_name << "_" << i << ")\n";
		}
		output << "cmd.load_cgo(" << obj_name << ", '" << obj_name << "')\n";
		output << "cmd.set('two_sided_lighting', '" << (two_sided_lighting ? "on" : "off") << "')\n";
	}

	void print_jmol_script(const std::string& obj_name, std::ostream& output)
	{
		std::istringstream input(str());
		if(!input.good())
		{
			return;
		}
		double alpha=1.0;
		Color color(0xFFFFFF);
		std::vector<PlainPoint> global_vertices;
		std::vector<PlainTriple> global_triples;
		while(input.good())
		{
			std::string type_str;
			input >> type_str;
			const ObjectTypeMarker type(type_str, object_typer_);
			if(type.alpha || type.color)
			{
				print_jmol_polygon(global_vertices, global_triples, color, alpha, obj_name, output);
				if(type.alpha)
				{
					input >> alpha;
					alpha=(1.0-alpha);
				}
				else if(type.color)
				{
					color=read_color_from_stream(input);
				}
			}
			else if(type.tstrip || type.tfan || type.tfanc)
			{
				std::vector<PlainPoint> vertices;
				std::vector<PlainPoint> normals;
				if(read_strip_or_fan_from_stream(type.tstrip, type.tfan, type.tfanc, input, vertices, normals))
				{
					const std::size_t offset=global_vertices.size();
					global_vertices.insert(global_vertices.end(), vertices.begin(), vertices.end());
					for(std::size_t i=0;(i+2)<vertices.size();i++)
					{
						global_triples.push_back(PlainTriple(offset+(type.tstrip ? i : 0), offset+(i+1), offset+(i+2)));
					}
				}
			}
		}
		print_jmol_polygon(global_vertices, global_triples, color, alpha, obj_name, output);
	}

	void print_scenejs_script(const std::string& obj_name, const bool fit, std::ostream& output)
	{
		std::istringstream input(str());
		if(!input.good())
		{
			return;
		}
		std::ostringstream body_output;
		Color color(0xFFFFFF);
		std::string label=obj_name;
		std::vector<PlainPoint> global_vertices;
		std::vector<PlainPoint> global_normals;
		std::vector<PlainTriple> global_triples;
		BoundingBox bounding_box;
		while(input.good())
		{
			std::string type_str;
			input >> type_str;
			const ObjectTypeMarker type(type_str, object_typer_);
			if(type.color || type.label)
			{
				print_scenejs_polygon(global_vertices, global_normals, global_triples, color, label, body_output);
				if(type.color)
				{
					color=read_color_from_stream(input);
				}
				else if(type.label)
				{
					input >> label;
				}
			}
			else if(type.tstrip || type.tfan || type.tfanc)
			{
				std::vector<PlainPoint> vertices;
				std::vector<PlainPoint> normals;
				if(read_strip_or_fan_from_stream(type.tstrip, type.tfan, type.tfanc, input, vertices, normals) && vertices.size()==normals.size())
				{
					const std::size_t offset=global_vertices.size();
					global_vertices.insert(global_vertices.end(), vertices.begin(), vertices.end());
					global_normals.insert(global_normals.end(), normals.begin(), normals.end());
					for(std::size_t i=0;(i+2)<vertices.size();i++)
					{
						global_triples.push_back(PlainTriple(offset+(type.tstrip ? i : 0), offset+(i+1), offset+(i+2)));
					}
					for(std::size_t i=0;i<vertices.size();i++)
					{
						bounding_box.update(vertices[i]);
					}
				}
			}
		}
		print_scenejs_polygon(global_vertices, global_normals, global_triples, color, label, body_output);
		{
			const std::pair<PlainPoint, double> transformation=(fit ? bounding_box.calc_normal_transformation() : std::make_pair(PlainPoint(), 1.0));
			output.precision(3);
			output << "var " << obj_name << "={type:\"scale\",x:" << std::fixed << transformation.second << ",y:" << transformation.second << ",z:" << transformation.second << ",\n";
			{
				output << "nodes:[{type:\"translate\",x:" << std::fixed << transformation.first.x << ",y:" << transformation.first.y << ",z:" << transformation.first.z << ",\n";
				{
					output << "nodes:[\n";
					output << body_output.str();
					output << "]\n";
				}
				output << "}]\n";
			}
			output << "};\n";
		}
	}

private:
	struct ObjectTyper
	{
		std::string alpha;
		std::string color;
		std::string label;
		std::string tstrip;
		std::string tfan;
		std::string tfanc;
		std::string sphere;

		ObjectTyper() :
			alpha("_alpha"),
			color("_color"),
			label("_label"),
			tstrip("_tstrip"),
			tfan("_tfan"),
			tfanc("_tfanc"),
			sphere("_sphere")
		{
		}
	};

	struct ObjectTypeMarker
	{
		bool alpha;
		bool color;
		bool label;
		bool tstrip;
		bool tfan;
		bool tfanc;
		bool sphere;

		ObjectTypeMarker(const std::string& type_str, const ObjectTyper& object_typer) :
			alpha(type_str==object_typer.alpha),
			color(type_str==object_typer.color),
			label(type_str==object_typer.label),
			tstrip(type_str==object_typer.tstrip),
			tfan(type_str==object_typer.tfan),
			tfanc(type_str==object_typer.tfanc),
			sphere(type_str==object_typer.sphere)
		{
		}
	};

	struct PlainPoint
	{
		double x;
		double y;
		double z;

		PlainPoint() : x(0), y(0), z(0)
		{
		}

		PlainPoint(const double x, const double y, const double z) : x(x), y(y), z(z)
		{
		}
	};

	struct PlainTriple
	{
		std::size_t a;
		std::size_t b;
		std::size_t c;

		PlainTriple() : a(0), b(0), c(0)
		{
		}

		PlainTriple(const std::size_t a, const std::size_t b, const std::size_t c) : a(a), b(b), c(c)
		{
		}
	};

	struct Color
	{
		unsigned int r;
		unsigned int g;
		unsigned int b;

		Color() : r(0), g(0), b(0)
		{
		}

		Color(const unsigned int rgb) : r((rgb&0xFF0000) >> 16), g((rgb&0x00FF00) >> 8), b(rgb&0x0000FF)
		{
		}
	};

	class BoundingBox
	{
	public:
		BoundingBox() : modified_(false)
		{
			const double max_double=std::numeric_limits<double>::max();
			const double min_double=-max_double;
			low_=PlainPoint(max_double, max_double, max_double);
			high_=PlainPoint(min_double, min_double, min_double);
		}

		template<typename PointType>
		void update(const PointType& p)
		{
			low_.x=std::min(low_.x, p.x);
			low_.y=std::min(low_.y, p.y);
			low_.z=std::min(low_.z, p.z);
			high_.x=std::max(high_.x, p.x);
			high_.y=std::max(high_.y, p.y);
			high_.z=std::max(high_.z, p.z);
			modified_=true;
		}

		std::pair<PlainPoint, double> calc_normal_transformation() const
		{
			if(modified_)
			{
				const PlainPoint translation(0.0-((low_.x+high_.x)*0.5), 0.0-((low_.y+high_.y)*0.5), 0.0-((low_.z+high_.z)*0.5));
				const double max_side=std::max(high_.x-low_.x, std::max(high_.y-low_.y, high_.z-low_.z));
				return std::make_pair(translation, (max_side>0.0 ? 2.0/max_side : 1.0));
			}
			else
			{
				return std::make_pair(PlainPoint(), 1.0);
			}
		}

	private:
		bool modified_;
		PlainPoint low_;
		PlainPoint high_;
	};

	template<typename PointType>
	static void write_point_to_stream(const PointType& p, const std::string& start, const std::string& sep, const std::string& end, std::ostream& output)
	{
		output << start;
		output.precision(3);
		output << std::fixed << p.x << sep << p.y << sep << p.z;
		output << end;
	}

	template<typename PointType>
	static void write_point_to_stream(const PointType& p, std::ostream& output)
	{
		write_point_to_stream(p, "", " ", " ", output);
	}

	static PlainPoint read_point_from_stream(std::istream& input)
	{
		PlainPoint p;
		input >> p.x >> p.y >> p.z;
		return p;
	}

	template<typename PointType>
	static void write_points_vector_to_stream(const std::vector<PointType>& v, std::ostream& output)
	{
		output << v.size() << " ";
		for(std::size_t i=0;i<v.size();i++)
		{
			write_point_to_stream(v[i], output);
		}
	}

	static std::vector<PlainPoint> read_points_vector_from_stream(std::istream& input)
	{
		std::size_t n=0;
		input >> n;
		std::vector<PlainPoint> v(n);
		for(std::size_t i=0;i<v.size();i++)
		{
			v[i]=read_point_from_stream(input);
		}
		return v;
	}

	static void write_color_to_stream(const Color& c, const bool normalized, const std::string& start, const std::string& sep, const std::string& end, std::ostream& output)
	{
		output << start;
		if(normalized)
		{
			output.precision(3);
			output << std::fixed << (static_cast<double>(c.r)/255.0) << sep << (static_cast<double>(c.g)/255.0) << sep << (static_cast<double>(c.b)/255.0);
		}
		else
		{
			output << c.r << sep << c.g << sep << c.b;
		}
		output << end;
	}

	static void write_color_to_stream(const Color& c, std::ostream& output)
	{
		write_color_to_stream(c, false, "", " ", " ", output);
	}

	static Color read_color_from_stream(std::istream& input)
	{
		Color c;
		input >> c.r >> c.g >> c.b;
		return c;
	}

	static bool read_strip_or_fan_from_stream(
			const bool tstrip,
			const bool tfan,
			const bool tfanc,
			std::istream& input,
			std::vector<PlainPoint>& vertices,
			std::vector<PlainPoint>& normals)
	{
		vertices.clear();
		normals.clear();
		if(tstrip || tfan)
		{
			vertices=read_points_vector_from_stream(input);
			normals=read_points_vector_from_stream(input);
		}
		else if(tfanc)
		{
			const PlainPoint center=read_point_from_stream(input);
			const PlainPoint normal=read_point_from_stream(input);
			const std::vector<PlainPoint> outer_vertices=read_points_vector_from_stream(input);
			if(!outer_vertices.empty())
			{
				vertices.push_back(center);
				vertices.insert(vertices.end(), outer_vertices.begin(), outer_vertices.end());
				vertices.push_back(outer_vertices.front());
				normals.resize(vertices.size(), normal);
			}
		}
		return (vertices.size()>=3 && vertices.size()==normals.size());
	}

	static void print_jmol_polygon(
			std::vector<PlainPoint>& vertices,
			std::vector<PlainTriple>& triples,
			const Color& color,
			const double alpha,
			const std::string& id,
			std::ostream& output)
	{
		static int use_num=0;
		if(!(vertices.empty() || triples.empty()))
		{
			output << "draw " << id << use_num << " ";
			output << "POLYGON " << vertices.size() << " ";
			for(std::size_t i=0;i<vertices.size();i++)
			{
				write_point_to_stream(vertices[i], "{", " ", "} ", output);
			}
			output  << triples.size();
			for(std::size_t i=0;i<triples.size();i++)
			{
				const PlainTriple& t=triples[i];
				output << " [" << t.a << " " << t.b << " " << t.c << " 0]";
			}
			output << "\n";
			output << "color $" << id << use_num;
			if(alpha>0.0)
			{
				output << " TRANSLUCENT " << alpha;
			}
			write_color_to_stream(color, false, " [", ",", "]\n", output);
			use_num++;
		}
		vertices.clear();
		triples.clear();
	}

	static void print_scenejs_polygon(
			std::vector<PlainPoint>& vertices,
			std::vector<PlainPoint>& normals,
			std::vector<PlainTriple>& triples,
			const Color& color,
			const std::string& id,
			std::ostream& output)
	{
		if(!(vertices.empty() || normals.size()!=vertices.size() || triples.empty()))
		{
			output << "{type:\"name\",name:\"" << id << "\",\n";
			{
				output << "nodes:[{type:\"material\",\n";
				output.precision(3);
				output << "color:{r:" << std::fixed << (static_cast<double>(color.r)/255.0) << ",g:" << (static_cast<double>(color.g)/255.0) << ",b:" << (static_cast<double>(color.b)/255.0) << "},\n";
				{
					output << "nodes:[{type:\"geometry\",primitive:\"triangles\",\n";
					{
						output << "positions:[\n";
						for(std::size_t i=0;i<vertices.size();i++)
						{
							write_point_to_stream(vertices[i], (i==0 ? "" : ",\n"), ",", "", output);
						}
						output << "\n],\n";
					}
					{
						output << "normals:[\n";
						for(std::size_t i=0;i<normals.size();i++)
						{
							write_point_to_stream(normals[i], (i==0 ? "" : ",\n"), ",", "", output);
						}
						output << "\n],\n";
					}
					{
						output << "indices:[\n";
						for(std::size_t i=0;i<triples.size();i++)
						{
							const PlainTriple& t=triples[i];
							output << (i==0  ? "" : ",\n") << t.a << "," << t.b << "," << t.c;
						}
						output << "\n],\n";
					}
					output << "}]\n";
				}
				output << "}]\n";
			}
			output << "},\n";
		}
		vertices.clear();
		normals.clear();
		triples.clear();
	}

	ObjectTyper object_typer_;
	std::ostringstream string_stream_;
};

}

#endif /* AUXILIARIES_OPENGL_PRINTER_H_ */
