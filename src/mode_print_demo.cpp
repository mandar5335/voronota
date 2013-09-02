#include <iostream>
#include <deque>

#include "apollota/triangulation.h"
#include "apollota/opengl_printer.h"

#include "modes_commons.h"

namespace
{

void print_demo_bsh(const auxiliaries::ProgramOptionsHandler& poh)
{
	const double init_radius=poh.argument<double>("--init-radius", 3.5);
	if(init_radius<=1.0)
	{
		throw std::runtime_error("Bounding spheres hierarchy initial radius should be greater than 1.");
	}

	const unsigned int max_level=poh.argument<unsigned int>("--max-level", 0);

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	if(spheres.size()<4)
	{
		throw std::runtime_error("Less than 4 balls provided to stdin.");
	}

	apollota::BoundingSpheresHierarchy bsh(spheres, init_radius, 1);

	if(bsh.levels()>0)
	{
		apollota::OpenGLPrinter::print_setup(std::cout);

		{
			std::ostringstream obj_name;
			obj_name << "obj_as";
			std::ostringstream cgo_name;
			cgo_name << "cgo_as";
			apollota::OpenGLPrinter opengl_printer(std::cout, obj_name.str(), cgo_name.str());
			for(std::size_t i=0;i<spheres.size();i++)
			{
				opengl_printer.print_color(0x36BBCE);
				opengl_printer.print_sphere(spheres[i]);
			}
		}

		for(std::size_t l=0;l<bsh.levels() && l<=max_level;l++)
		{
			const std::vector<apollota::SimpleSphere> bs=bsh.collect_bounding_spheres(l);
			std::ostringstream obj_name;
			obj_name << "obj_bs" << l;
			std::ostringstream cgo_name;
			cgo_name << "cgo_bs" << l;
			apollota::OpenGLPrinter opengl_printer(std::cout, obj_name.str(), cgo_name.str());
			for(std::size_t i=0;i<bs.size();i++)
			{
				opengl_printer.print_color(0x37DE6A);
				opengl_printer.print_sphere(bs[i]);
			}
		}

		std::cout << "cmd.set('bg_rgb', [1,1,1])\n\n";
		std::cout << "cmd.set('ambient', 0.3)\n\n";
	}
}

void print_demo_face()
{
	std::vector<apollota::SimpleSphere> generators;
	generators.push_back(apollota::SimpleSphere(2, 1, 0, 0.7));
	generators.push_back(apollota::SimpleSphere(-1, 2, 0, 1.0));
	generators.push_back(apollota::SimpleSphere(-1, -1, 0, 1.3));

	apollota::OpenGLPrinter::print_setup(std::cout);

	apollota::OpenGLPrinter opengl_printer_curve(std::cout, "obj_curve", "cgo_curve");
	apollota::OpenGLPrinter opengl_printer_generators(std::cout, "obj_generators", "cgo_generators");
	apollota::OpenGLPrinter opengl_printer_m_surface(std::cout, "obj_m_surface", "cgo_m_surface");
	apollota::OpenGLPrinter opengl_printer_m_trans_surface(std::cout, "obj_m_trans_surface", "cgo_m_trans_surface");
	apollota::OpenGLPrinter opengl_printer_tangent_planes(std::cout, "obj_tangent_planes", "cgo_tangent_planes");
	apollota::OpenGLPrinter opengl_printer_tangent_spheres(std::cout, "obj_tangent_spheres", "cgo_tangent_spheres");
	apollota::OpenGLPrinter opengl_printer_m_contour(std::cout, "obj_m_contour", "cgo_m_contour");
	apollota::OpenGLPrinter opengl_printer_m_touch_points(std::cout, "obj_m_touch_points", "cgo_m_touch_points");
	apollota::OpenGLPrinter opengl_printer_m_touch_curves(std::cout, "obj_m_touch_curves", "cgo_m_touch_curves");
	apollota::OpenGLPrinter opengl_printer_m_triangles(std::cout, "obj_m_triangles", "cgo_m_triangles");

	for(std::size_t i=0;i<generators.size();i++)
	{
		opengl_printer_generators.print_color(0x36BBCE);
		opengl_printer_generators.print_sphere(apollota::SimpleSphere(generators[i], generators[i].r-0.01));
	}

	std::vector< std::pair<apollota::SimplePoint, apollota::SimplePoint> > tangent_planes=apollota::TangentPlaneOfThreeSpheres::calculate(generators[0], generators[1], generators[2]);
	const std::vector<apollota::SimpleSphere> min_tangents=apollota::TangentSphereOfThreeSpheres::calculate(generators[0], generators[1], generators[2]);
	if(tangent_planes.size()==2 && min_tangents.size()==1)
	{
		if(apollota::halfspace_of_point(generators[0], apollota::plane_normal_from_three_points<apollota::SimplePoint>(generators[0], generators[1], generators[2]), tangent_planes[0].first+tangent_planes[0].second)>0)
		{
			std::swap(tangent_planes[0], tangent_planes[1]);
		}

		const apollota::SimpleSphere min_tangent=min_tangents.front();
		opengl_printer_curve.print_color(0xA61700);
		opengl_printer_curve.print_sphere(apollota::SimpleSphere(min_tangent, 0.1));

		std::deque<apollota::SimplePoint> curve;
		std::deque<double> radii;
		{
			const double r_mult=1.01;
			const double r_max=14.0;
			for(double r=min_tangent.r;r<r_max;r=r*r_mult)
			{
				const std::vector<apollota::SimpleSphere> tangent_spheres=apollota::TangentSphereOfThreeSpheres::calculate(generators[0], generators[1], generators[2], r);
				if(tangent_spheres.size()==2)
				{
					for(std::size_t i=0;i<tangent_spheres.size();i++)
					{
						if(apollota::halfspace_of_point(generators[0], apollota::plane_normal_from_three_points<apollota::SimplePoint>(generators[0], generators[1], generators[2]), tangent_spheres[i])<0)
						{
							curve.push_front(apollota::SimplePoint(tangent_spheres[i]));
							radii.push_front(r);
						}
						else
						{
							curve.push_back(apollota::SimplePoint(tangent_spheres[i]));
							radii.push_back(r);
						}
					}
				}
			}
		}

		opengl_printer_curve.print_line_strip(std::vector<apollota::SimplePoint>(curve.begin(), curve.end()));

		std::vector< std::vector<apollota::SimplePoint> > circles_touches;
		std::vector< std::vector<apollota::SimplePoint> > circles_vertices;
		std::vector< std::vector<apollota::SimplePoint> > circles_normals;
		for(std::size_t i=0;i<curve.size();i++)
		{
			std::vector<apollota::SimplePoint> touches;
			if(i==0 || i+1==curve.size())
			{
				for(std::size_t j=0;j<generators.size();j++)
				{
					touches.push_back(apollota::SimplePoint(generators[j])+(tangent_planes[i==0 ? 0 : 1].second*generators[j].r));
				}
			}
			else
			{
				const apollota::SimpleSphere tangent(curve[i], radii[i]);
				for(std::size_t j=0;j<generators.size();j++)
				{
					apollota::SimplePoint a(tangent);
					apollota::SimplePoint b(generators[j]);
					apollota::SimplePoint ab=((b-a).unit())*tangent.r;
					touches.push_back(a+ab);
				}
			}
			const std::vector<apollota::SimpleSphere> circles=apollota::TangentSphereOfThreeSpheres::calculate(apollota::SimpleSphere(touches[0], 0.0), apollota::SimpleSphere(touches[1], 0.0), apollota::SimpleSphere(touches[2], 0.0));
			if(circles.size()==1)
			{
				const apollota::SimpleSphere& circle=circles.front();
				const apollota::SimplePoint orientation=apollota::plane_normal_from_three_points<apollota::SimplePoint>(touches[0], touches[1], touches[2]);
				const apollota::SimplePoint first_point(apollota::SimplePoint(circle)-touches[0]);
				apollota::Rotation rotation(apollota::custom_point_from_object<apollota::SimplePoint>(orientation), 0);
				const double angle_step=10;
				std::vector<apollota::SimplePoint> circle_vertices;
				for(rotation.angle=0;rotation.angle<=360;rotation.angle+=angle_step)
				{
					circle_vertices.push_back(apollota::SimplePoint(circle)+rotation.rotate<apollota::SimplePoint>(first_point));
				}
				std::vector<apollota::SimplePoint> circle_normals;
				for(std::size_t j=0;j<circle_vertices.size();j++)
				{
					circle_normals.push_back((circle_vertices[j]-apollota::SimplePoint(circle)).unit());
				}
				circles_touches.push_back(touches);
				circles_vertices.push_back(circle_vertices);
				circles_normals.push_back(circle_normals);
			}
		}

		opengl_printer_m_surface.print_color(0xFF5A40);
		opengl_printer_m_trans_surface.print_color(0xFF5A40);
		opengl_printer_m_trans_surface.print_alpha(0.5);
		for(std::size_t i=0;i+1<circles_vertices.size();i++)
		{
			std::size_t j=i+1;
			std::vector<apollota::SimplePoint> vertices;
			std::vector<apollota::SimplePoint> normals;
			if(circles_vertices[i].size()==circles_vertices[j].size())
			{
				for(std::size_t e=0;e<circles_vertices[i].size();e++)
				{
					vertices.push_back(circles_vertices[i][e]);
					vertices.push_back(circles_vertices[j][e]);
					normals.push_back(circles_normals[i][e]);
					normals.push_back(circles_normals[j][e]);
				}
				opengl_printer_m_surface.print_triangle_strip(vertices, normals);
				opengl_printer_m_trans_surface.print_triangle_strip(vertices, normals);
			}
		}

		for(int i=0;i<2;i++)
		{
			std::vector<apollota::SimplePoint> vertices_inner=circles_vertices[i==0 ? 0 : circles_vertices.size()-1];
			std::vector<apollota::SimplePoint> normals_inner=circles_normals[i==0 ? 0 : circles_normals.size()-1];
			std::vector<apollota::SimplePoint> vertices;
			std::vector<apollota::SimplePoint> normals;
			for(std::size_t j=0;j<vertices_inner.size();j++)
			{
				vertices.push_back(vertices_inner[j]);
				vertices.push_back(vertices_inner[j]+(normals_inner[j]*2.5));
				normals.push_back(tangent_planes[i].second);
				normals.push_back(tangent_planes[i].second);
			}
			opengl_printer_tangent_planes.print_color(i==0 ? 0xFFB673 : 0x64DE89);
			opengl_printer_tangent_planes.print_triangle_strip(vertices, normals);
		}

		{
			std::size_t i=0;
			while(i<(curve.size()/2))
			{
				opengl_printer_tangent_spheres.print_alpha(0.2);
				opengl_printer_tangent_spheres.print_color(0xFF9C40);
				opengl_printer_tangent_spheres.print_sphere(apollota::SimpleSphere(curve[i], radii[i]-0.01));
				opengl_printer_tangent_spheres.print_color(0x37DE6A);
				opengl_printer_tangent_spheres.print_sphere(apollota::SimpleSphere(curve[curve.size()-1-i], radii[curve.size()-1-i]-0.01));
				i+=30;
			}
		}

		if(!circles_vertices.empty())
		{
			std::vector< std::pair< std::vector<apollota::SimplePoint>, std::vector<apollota::SimplePoint> > > tps(3);
			const double gap_distance_threshold=apollota::distance_from_point_to_point(circles_vertices.front().at(0), circles_vertices.back().at(0))/8.0;
			double gap_distance=0.0;
			for(std::size_t i=0;i<circles_vertices.size()/2;i++)
			{
				bool draw_on=false;
				if(i==0 || i+1==circles_vertices.size()/2)
				{
					draw_on=true;
				}
				else
				{
					gap_distance+=apollota::distance_from_point_to_point(circles_vertices[i].at(0), circles_vertices[i+1].at(0));
					if(gap_distance>gap_distance_threshold)
					{
						gap_distance=0.0;
						draw_on=true;
					}
				}
				if(draw_on)
				{
					opengl_printer_m_contour.print_color(0x111111);
					opengl_printer_m_contour.print_line_strip(circles_vertices[i]);
					opengl_printer_m_contour.print_line_strip(circles_vertices[circles_vertices.size()-1-i]);

					opengl_printer_m_touch_points.print_color(0x111111);
					for(std::size_t j=0;j<circles_touches[i].size() && j<circles_touches[circles_vertices.size()-1-i].size() && j<tps.size();j++)
					{
						opengl_printer_m_touch_points.print_sphere(apollota::SimpleSphere(circles_touches[i][j], 0.05));
						opengl_printer_m_touch_points.print_sphere(apollota::SimpleSphere(circles_touches[circles_vertices.size()-1-i][j], 0.05));
						tps[j].first.push_back(circles_touches[i][j]);
						tps[j].second.push_back(circles_touches[circles_vertices.size()-1-i][j]);
					}

					opengl_printer_m_triangles.print_color(0x00FF00);
					opengl_printer_m_triangles.print_line_strip(circles_touches[i], true);
					opengl_printer_m_triangles.print_line_strip(circles_touches[circles_vertices.size()-1-i], true);
				}
			}

			opengl_printer_m_touch_curves.print_color(0x111111);
			for(std::size_t j=0;j<tps.size();j++)
			{
				tps[j].first.insert(tps[j].first.end(), tps[j].second.rbegin(), tps[j].second.rend());
				opengl_printer_m_touch_curves.print_line_strip(tps[j].first);
			}
		}
	}

	std::cout << "cmd.set('ray_shadows', 'off')\n\n";
	std::cout << "cmd.set('ray_shadow', 'off')\n\n";
	std::cout << "cmd.set('two_sided_lighting', 'on')\n\n";
	std::cout << "cmd.set('cgo_line_width', 3)\n\n";
	std::cout << "cmd.set('bg_rgb', [1,1,1])\n\n";
	std::cout << "cmd.set('ambient', 0.3)\n\n";
}

void print_demo_tangent_spheres()
{
	std::vector< std::vector<apollota::SimpleSphere> > generators_sets;

	{
		std::vector<apollota::SimpleSphere> generators;
		generators.push_back(apollota::SimpleSphere(2.0, 1.0, 0.1, 0.9));
		generators.push_back(apollota::SimpleSphere(-1.0, 2.0, -0.1, 1.2));
		generators.push_back(apollota::SimpleSphere(-1.0, -1.0, 0.1, 1.5));
		generators.push_back(apollota::SimpleSphere(1.0, 1.0, 4.5, 1.0));
		generators_sets.push_back(generators);
	}

	{
		std::vector<apollota::SimpleSphere> generators;
		generators.push_back(apollota::SimpleSphere(2.0, 1.0, 0.1, 0.9));
		generators.push_back(apollota::SimpleSphere(-1.0, 2.0, -0.1, 1.2));
		generators.push_back(apollota::SimpleSphere(-1.0, -1.0, 0.1, 1.5));
		generators.push_back(apollota::SimpleSphere(0.4, 0.4, 0.0, 0.5));
		generators_sets.push_back(generators);
	}

	apollota::OpenGLPrinter::print_setup(std::cout);

	for(std::size_t j=0;j<generators_sets.size();j++)
	{
		const std::vector<apollota::SimpleSphere>& generators=generators_sets[j];

		std::ostringstream obj_name;
		obj_name << "obj" << j;
		std::ostringstream cgo_name;
		cgo_name << "cgo" << j;
		apollota::OpenGLPrinter opengl_printer(std::cout, obj_name.str(), cgo_name.str());

		opengl_printer.print_color(0x36BBCE);
		opengl_printer.print_alpha(1.0);
		for(std::size_t i=0;i<generators.size();i++)
		{
			opengl_printer.print_sphere(generators[i]);
		}

		const std::vector<apollota::SimpleSphere> tangents=apollota::TangentSphereOfFourSpheres::calculate(generators[0], generators[1], generators[2], generators[3]);

		opengl_printer.print_color(0xFF5A40);
		opengl_printer.print_alpha(0.7);
		for(std::size_t i=0;i<tangents.size();i++)
		{
			opengl_printer.print_sphere(tangents[i]);
		}
	}

	std::cout << "cmd.set('bg_rgb', [1,1,1])\n\n";
	std::cout << "cmd.set('ambient', 0.3)\n\n";
}

void print_demo_tangent_planes()
{
	std::vector<apollota::SimpleSphere> generators;
	generators.push_back(apollota::SimpleSphere(2.0, 1.0, 0.1, 0.9));
	generators.push_back(apollota::SimpleSphere(-1.0, 2.0, -0.1, 1.2));
	generators.push_back(apollota::SimpleSphere(-1.0, -1.0, 0.1, 1.5));

	apollota::OpenGLPrinter::print_setup(std::cout);
	apollota::OpenGLPrinter opengl_printer(std::cout, "obj", "cgo");

	opengl_printer.print_color(0x36BBCE);
	opengl_printer.print_alpha(1.0);
	for(std::size_t i=0;i<generators.size();i++)
	{
		opengl_printer.print_sphere(generators[i]);
	}

	const std::vector< std::pair<apollota::SimplePoint, apollota::SimplePoint> > tangent_planes=apollota::TangentPlaneOfThreeSpheres::calculate(generators[0], generators[1], generators[2]);
	for(std::size_t i=0;i<tangent_planes.size();i++)
	{
		opengl_printer.print_color(0xFF5A40);
		opengl_printer.print_alpha(0.7);
		std::vector<apollota::SimplePoint> vertices(generators.size());
		std::vector<apollota::SimplePoint> normals(generators.size());
		for(std::size_t j=0;j<generators.size();j++)
		{
			vertices[j]=(apollota::SimplePoint(generators[j])+(tangent_planes[i].second.unit()*generators[j].r));
			normals[j]=tangent_planes[i].second;
		}
		{
			const std::vector<apollota::SimpleSphere> circles=apollota::TangentSphereOfThreeSpheres::calculate(apollota::SimpleSphere(vertices[0], 0.0), apollota::SimpleSphere(vertices[1], 0.0), apollota::SimpleSphere(vertices[2], 0.0));
			if(circles.size()==1)
			{
				const apollota::SimplePoint center(circles.front());
				for(std::size_t j=0;j<vertices.size();j++)
				{
					vertices[j]=vertices[j]+((vertices[j]-center)*1.5);
				}
			}
		}
		opengl_printer.print_triangle_strip(vertices, normals);
	}

	std::cout << "cmd.set('two_sided_lighting', 'on')\n\n";
	std::cout << "cmd.set('bg_rgb', [1,1,1])\n\n";
	std::cout << "cmd.set('ambient', 0.3)\n\n";
}

void print_demo_edges(const auxiliaries::ProgramOptionsHandler& poh)
{
	const double max_dist=poh.argument<double>("--max-dist", std::numeric_limits<double>::max());

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);
	const apollota::Triangulation::Result triangulation_result=apollota::Triangulation::construct_result(spheres, 3.5, true, false);
	apollota::Triangulation::NeighborsGraph graph=apollota::Triangulation::collect_neighbors_graph_from_neighbors_map(apollota::Triangulation::collect_neighbors_map_from_quadruples_map(triangulation_result.quadruples_map), spheres.size());

	apollota::OpenGLPrinter::print_setup(std::cout);
	apollota::OpenGLPrinter opengl_printer(std::cout, "obj_edges", "cgo_edges");
	opengl_printer.print_color(0x36BBCE);
	for(std::size_t i=0;i<graph.size();i++)
	{
		for(std::size_t j=0;j<graph[i].size();j++)
		{
			std::vector<apollota::SimpleSphere> neighbors(2);
			neighbors[0]=(spheres[i]);
			neighbors[1]=(spheres[graph[i][j]]);
			const double dist=apollota::minimal_distance_from_sphere_to_sphere(neighbors[0], neighbors[1]);
			if(dist<max_dist)
			{
				opengl_printer.print_line_strip(neighbors);
			}
		}
	}
}

void print_demo_splitting(const auxiliaries::ProgramOptionsHandler& poh)
{
	const int parts=poh.argument<double>("--parts", 2);

	std::vector<apollota::SimpleSphere> spheres;
	auxiliaries::read_lines_to_container(std::cin, "#", modes_commons::add_sphere_from_stream_to_vector<apollota::SimpleSphere>, spheres);

	const std::vector< std::vector<std::size_t> > ids=apollota::SplittingOfSpheres::split_for_number_of_parts(spheres, parts);

	apollota::OpenGLPrinter::print_setup(std::cout);
	apollota::OpenGLPrinter opengl_printer(std::cout, "obj_splitting", "cgo_splitting");

	for(std::size_t i=0;i<ids.size();i++)
	{
		opengl_printer.print_color(((0x36BBCE)*static_cast<int>(i+1))%(0xFFFFFF));
		for(std::size_t j=0;j<ids[i].size();j++)
		{
			opengl_printer.print_sphere(spheres[ids[i][j]]);
		}
	}
}

}

void print_demo(const auxiliaries::ProgramOptionsHandler& poh)
{
	if(poh.contains_option("--help") || poh.contains_option("--help-full"))
	{
		return;
	}

	const std::string scene=poh.argument<std::string>("--scene");

	if(scene=="bsh")
	{
		print_demo_bsh(poh);
	}
	else if(scene=="face")
	{
		print_demo_face();
	}
	else if(scene=="tangent-spheres")
	{
		print_demo_tangent_spheres();
	}
	else if(scene=="tangent-planes")
	{
		print_demo_tangent_planes();
	}
	else if(scene=="edges")
	{
		print_demo_edges(poh);
	}
	else if(scene=="splitting")
	{
		print_demo_splitting(poh);
	}
	else
	{
		throw std::runtime_error("Invalid scene name.");
	}
}
