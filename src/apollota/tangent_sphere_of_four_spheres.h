#ifndef APOLLOTA_TANGENT_SPHERE_OF_FOUR_SPHERES_H_
#define APOLLOTA_TANGENT_SPHERE_OF_FOUR_SPHERES_H_

#include <vector>

#include "basic_operations_on_spheres.h"
#include "rotation.h"
#include "safe_quadratic_equation_root.h"

namespace apollota
{

class TangentSphereOfFourSpheres
{
public:
	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC, typename InputSphereTypeD>
	static inline std::vector<SimpleSphere> calculate(const InputSphereTypeA& sm, const InputSphereTypeB& s1, const InputSphereTypeC& s2, const InputSphereTypeD& s3)
	{
		{
			const double min_r=std::min(sm.r, std::min(s1.r, std::min(s2.r, s3.r)));
			if(sm.r!=min_r)
			{
				if(s1.r==min_r) return calculate(s1, sm, s2, s3);
				if(s2.r==min_r) return calculate(s2, sm, s1, s3);
				if(s3.r==min_r) return calculate(s3, sm, s1, s2);
			}
		}

		const unsigned int rotation_steps=2;
		const SimplePoint rotation_axis(1.0, 1.0, 1.0);
		const double rotation_step_angle=30.0;

		for(unsigned int rotation_step=0;rotation_step<=rotation_steps;rotation_step++)
		{
			SimpleSphere ts1(s1.x-sm.x, s1.y-sm.y, s1.z-sm.z, s1.r-sm.r);
			SimpleSphere ts2(s2.x-sm.x, s2.y-sm.y, s2.z-sm.z, s2.r-sm.r);
			SimpleSphere ts3(s3.x-sm.x, s3.y-sm.y, s3.z-sm.z, s3.r-sm.r);

			if(rotation_step>0)
			{
				const Rotation rotation(rotation_axis, rotation_step_angle*static_cast<double>(rotation_step));
				ts1=SimpleSphere(rotation.rotate<SimplePoint>(ts1), ts1.r);
				ts2=SimpleSphere(rotation.rotate<SimplePoint>(ts2), ts2.r);
				ts3=SimpleSphere(rotation.rotate<SimplePoint>(ts3), ts3.r);
			}

			const double x1=ts1.x;
			const double y1=ts1.y;
			const double z1=ts1.z;
			const double r1=ts1.r;

			const double x2=ts2.x;
			const double y2=ts2.y;
			const double z2=ts2.z;
			const double r2=ts2.r;

			const double x3=ts3.x;
			const double y3=ts3.y;
			const double z3=ts3.z;
			const double r3=ts3.r;

			const double a1=2*x1;
			const double b1=2*y1;
			const double c1=2*z1;
			const double d1=2*r1;
			const double o1=(r1*r1-x1*x1-y1*y1-z1*z1);

			const double a2=2*x2;
			const double b2=2*y2;
			const double c2=2*z2;
			const double d2=2*r2;
			const double o2=(r2*r2-x2*x2-y2*y2-z2*z2);

			const double a3=2*x3;
			const double b3=2*y3;
			const double c3=2*z3;
			const double d3=2*r3;
			const double o3=(r3*r3-x3*x3-y3*y3-z3*z3);

			const double w  = (a1*(b3*c2-b2*c3) + b1*(a2*c3-a3*c2) + c1*(a3*b2-a2*b3));

			if(w>0.0 || w<0.0)
			{
				const double u1 = -( b1*(c3*d2-c2*d3) + c1*(b2*d3-b3*d2) + d1*(b3*c2-b2*c3) ) / w;
				const double v1 = -( b1*(c3*o2-c2*o3) + c1*(b2*o3-b3*o2) + o1*(b3*c2-b2*c3) ) / w;

				const double u2 =  ( a1*(c3*d2-c2*d3) + c1*(a2*d3-a3*d2) + d1*(a3*c2-a2*c3) ) / w;
				const double v2 =  ( a1*(c3*o2-c2*o3) + c1*(a2*o3-a3*o2) + o1*(a3*c2-a2*c3) ) / w;

				const double u3 = -( a1*(b3*d2-b2*d3) + b1*(a2*d3-a3*d2) + d1*(a3*b2-a2*b3) ) / w;
				const double v3 = -( a1*(b3*o2-b2*o3) + b1*(a2*o3-a3*o2) + o1*(a3*b2-a2*b3) ) / w;

				const double a = u1*u1+u2*u2+u3*u3-1;
				const double b = 2*(u1*v1+u2*v2+u3*v3);

				if(check_if_quadratic_equation_is_solvable(a, b))
				{
					const double c = v1*v1+v2*v2+v3*v3;
					std::vector<SimpleSphere> results;
					std::vector<double> radiuses;
					if(solve_quadratic_equation(a, b, c, radiuses))
					{
						results.reserve(radiuses.size());
						for(std::size_t i=0;i<radiuses.size();i++)
						{
							const double r=radiuses[i];
							if(r>=0.0)
							{
								SimpleSphere candidate((u1*r+v1), (u2*r+v2), (u3*r+v3), r);
								if(rotation_step>0)
								{
									const Rotation rotation(rotation_axis, (0.0-rotation_step_angle)*static_cast<double>(rotation_step));
									candidate=SimpleSphere(rotation.rotate<SimplePoint>(candidate), candidate.r);
								}
								candidate.x+=sm.x;
								candidate.y+=sm.y;
								candidate.z+=sm.z;
								candidate.r-=sm.r;
								if(check_tangent_sphere(sm, s1, s2, s3, candidate))
								{
									results.push_back(candidate);
								}
							}
						}
					}
					return results;
				}
			}
		}

		return std::vector<SimpleSphere>();
	}

private:
	template<typename InputSphereTypeA, typename InputSphereTypeB, typename InputSphereTypeC, typename InputSphereTypeD, typename InputSphereTypeE>
	static inline bool check_tangent_sphere(const InputSphereTypeA& s1, const InputSphereTypeB& s2, const InputSphereTypeC& s3, const InputSphereTypeD& s4, const InputSphereTypeE& tangent)
	{
		return (sphere_touches_sphere(tangent, s1) &&
				sphere_touches_sphere(tangent, s2) &&
				sphere_touches_sphere(tangent, s3) &&
				sphere_touches_sphere(tangent, s4));
	}
};

}

#endif /* APOLLOTA_TANGENT_SPHERE_OF_FOUR_SPHERES_H_ */
