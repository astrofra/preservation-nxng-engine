	/*----------------------------------------------------------------------------

		nXv9 by xBaRr/Syndrome 2000
			(nXs3) by xBaRr/Syndrome alias Emmanuel JULIEN
			All rights reserved 2000.

		Part of nXng.
		mailto://xbarr@pingoo.com
		http://xbarr.planet-d.net

		cMath implementation file

	----------------------------------------------------------------------------*/


	#pragma warning( disable : 4244 )

	#include	<string.h>
	#include	<math.h>
	#include	"nXng/cMath.h"


// cMath CLASS IMPLEMENTATION
// --------------------------

dXYZ::dXYZ(){}
dXYZ::dXYZ(double a, double b, double c)
{	x = a; y = b; z = c;	}
void	dXYZ::operator +(dXYZ *p)
{	x += p->x; y += p->y; z += p->z;	}
void	dXYZ::operator -(dXYZ *p)
{	x -= p->x; y -= p->y; z -= p->z;	}
void	dXYZ::operator *(dXYZ *p)
{	x *= p->x; y *= p->y; z *= p->z;	}
void	dXYZ::operator /(dXYZ *p)
{	x /= p->x; y /= p->y; z /= p->z;	}
void	dXYZ::operator +(fXYZ *p)
{	x += p->x; y += p->y; z += p->z;	}
void	dXYZ::operator -(fXYZ *p)
{	x -= p->x; y -= p->y; z -= p->z;	}
void	dXYZ::operator *(fXYZ *p)
{	x *= p->x; y *= p->y; z *= p->z;	}
void	dXYZ::operator /(fXYZ *p)
{	x /= p->x; y /= p->y; z /= p->z;	}
void	dXYZ::operator +(cM_Vector *p)
{	x += p->x; y += p->y; z += p->z;	}
void	dXYZ::operator -(cM_Vector *p)
{	x -= p->x; y -= p->y; z -= p->z;	}
void	dXYZ::operator *(cM_Vector *p)
{	x *= p->x; y *= p->y; z *= p->z;	}
void	dXYZ::operator /(cM_Vector *p)
{	x /= p->x; y /= p->y; z /= p->z;	}

fXYZ::fXYZ(){}
fXYZ::fXYZ(float a, float b, float c)
{	x = a; y = b; z = c;	}
void	fXYZ::operator +(dXYZ *p)
{	x += p->x; y += p->y; z += p->z;	}
void	fXYZ::operator -(dXYZ *p)
{	x -= p->x; y -= p->y; z -= p->z;	}
void	fXYZ::operator *(dXYZ *p)
{	x *= p->x; y *= p->y; z *= p->z;	}
void	fXYZ::operator /(dXYZ *p)
{	x /= p->x; y /= p->y; z /= p->z;	}
void	fXYZ::operator +(fXYZ *p)
{	x += p->x; y += p->y; z += p->z;	}
void	fXYZ::operator -(fXYZ *p)
{	x -= p->x; y -= p->y; z -= p->z;	}
void	fXYZ::operator *(fXYZ *p)
{	x *= p->x; y *= p->y; z *= p->z;	}
void	fXYZ::operator /(fXYZ *p)
{	x /= p->x; y /= p->y; z /= p->z;	}
void	fXYZ::operator +(cM_Vector *p)
{	x += p->x; y += p->y; z += p->z;	}
void	fXYZ::operator -(cM_Vector *p)
{	x -= p->x; y -= p->y; z -= p->z;	}
void	fXYZ::operator *(cM_Vector *p)
{	x *= p->x; y *= p->y; z *= p->z;	}
void	fXYZ::operator /(cM_Vector *p)
{	x /= p->x; y /= p->y; z /= p->z;	}


float cMath::Modulo(float a, float b)
{

	float	tmp;
	tmp = a / b;
	tmp -= float(floor(tmp));
	tmp *= b;
	return	tmp;

}

double cMath::DotProd(cM_Vector *u, cM_Vector *v)
{
	return	u->x * v->x + u->y * v->y + u->z * v->z;
}
double cMath::DotProd(cM_Vector *u, dXYZ *v)
{
	return	u->x * v->x + u->y * v->y + u->z * v->z;
}
double cMath::DotProd(dXYZ *u, cM_Vector *v)
{
	return	u->x * v->x + u->y * v->y + u->z * v->z;
}
double cMath::DotProd(dXYZ *u, dXYZ *v)
{
	return	u->x * v->x + u->y * v->y + u->z * v->z;
}

void cMath::CrossProd(cM_Vector *w, cM_Vector *u, cM_Vector *v)
{
	w->x = u->y * v->z - u->z * v->y;
	w->y = u->z * v->x - u->x * v->z;
	w->z = u->x * v->y - u->y * v->x;
}


// cM_Vector CLASS IMPLEMENTATION
// ------------------------------


// Cm_Vector::Constructors

cM_Vector::cM_Vector()
{
	memset(this, 0, sizeof(cM_Vector));
}

cM_Vector::cM_Vector(double u, double v, double w)
{
	memset(this, 0, sizeof(cM_Vector));
	x = u; y = v; z = w;	
}

// Cm_Vector::ComputeLength

double cM_Vector::ComputeLength()
{
	l = sqrt (x * x + y * y + z * z); 
	return l;
};

// Cm_Vector::Normalize

void cM_Vector::Normalize()
{
	double	il = 1.0 / l;
	x *= il; y *= il; z *= il;
}

void cM_Vector::operator +(cM_Vector *v)
{	x += v->x; y += v->y; z += v->z;	}
void cM_Vector::operator -(cM_Vector *v)
{	x -= v->x; y -= v->y; z -= v->z;	}
void cM_Vector::operator /(cM_Vector *v)
{	x /= v->x; y /= v->y; z /= v->z;	}
void cM_Vector::operator *(cM_Vector *v)
{	x *= v->x; y *= v->y; z *= v->z;	}
void cM_Vector::operator +(dXYZ *v)
{	x += v->x; y += v->y; z += v->z;	}
void cM_Vector::operator -(dXYZ *v)
{	x -= v->x; y -= v->y; z -= v->z;	}
void cM_Vector::operator /(dXYZ *v)
{	x /= v->x; y /= v->y; z /= v->z;	}
void cM_Vector::operator *(dXYZ *v)
{	x *= v->x; y *= v->y; z *= v->z;	}
void cM_Vector::operator +(fXYZ *v)
{	x += v->x; y += v->y; z += v->z;	}
void cM_Vector::operator -(fXYZ *v)
{	x -= v->x; y -= v->y; z -= v->z;	}
void cM_Vector::operator /(fXYZ *v)
{	x /= v->x; y /= v->y; z /= v->z;	}
void cM_Vector::operator *(fXYZ *v)
{	x *= v->x; y *= v->y; z *= v->z;	}

void cM_Vector::operator =(cM_Vector *v)
{
	x = v->x; y = v->y; z = v->z;
	l = v->l;
}


// cM_Plane CLASS IMPLEMENTATION
// ------------------------------


cM_Plane::cM_Plane()
{
	memset(this, 0, sizeof(cM_Plane));
}

cM_Plane::cM_Plane(double ua, double ub, double uc, double ud)
{
	memset(this, 0, sizeof(cM_Plane));
	a = ua; b = ub; c = uc; d = ud;
}

void cM_Plane::ComputeScalar(dXYZ *pip)
{
	d = - (a * pip->x + b * pip->y + c * pip->z);
}

void cM_Plane::ComputeScalar(fXYZ *pip)
{
	d = - (a * pip->x + b * pip->y + c * pip->z);
}

// RAYTRACE CLASS IMPLEMENTATION
// -----------------------------


double cMath::dPointToPlane(cM_Plane *p, dXYZ *from)
{
	return (p->a * from->x + p->b * from->y + p->c * from->z + p->d);
}

bool cMath::RayIntersectPlane(dXYZ *from, cM_Vector *view, cM_Plane *p, dXYZ *inter)
{

	// Denominator
	double	denom = (p->a * view->x + p->b * view->y + p->c * view->z);
	if (denom == 0.0)
		return false;		// poly is // to the ray

	// Determinant
	double	t = - (p->a * from->x + p->b * from->y + p->c * from->z + p->d) / denom;
	if (t < 0.0)
		return false;		// poly is behind or on ray origin ):

	// The following is the intersection point
	inter->x = from->x + view->x * t;
	inter->y = from->y + view->y * t;
	inter->z = from->z + view->z * t;
	return true;

}


#define EPSILON 0.1

bool cMath::IdentityPlane(cM_Plane *pa, cM_Plane *pb)
{

	double	da, db, dc, dd;
	bool	ra;

	da = fabs(pa->a + pb->a);
	db = fabs(pa->b + pb->b);
	dc = fabs(pa->c + pb->c);
	dd = fabs(pa->d + pb->d);
	if (da > EPSILON || db > EPSILON || dc > EPSILON || dd > EPSILON * 10.0)
		ra = false;	else ra = true;

	return ra;

}

/*
	cMath::BaryCoord()
	------------------
	Returns in *bcoord the barycentric coordinates of p in triangle {tri0, tri1, tri2}
*/


void cMath::BaryCoord(dXYZ *tri1, dXYZ *tri2, dXYZ *tri3, dXYZ *p, dXYZ *bcoord)
{

	cM_Vector	trin;
	cM_Vector	u, v, w, f;
	
	// Compute triangle normal
	u.x = tri2->x - tri1->x;
	u.y = tri2->y - tri1->y;
	u.z = tri2->z - tri1->z;
	v.x = tri3->x - tri1->x;
	v.y = tri3->y - tri1->y;
	v.z = tri3->z - tri1->z;

	CrossProd(&trin, &u, &v);
	trin.ComputeLength();
	trin.Normalize();

	// Get barycentric coordinate

	w.x = p->x - tri1->x;
	w.y = p->y - tri1->y;
	w.z = p->z - tri1->z;

	CrossProd(&f, &w, &v);
	bcoord->y = DotProd(&f, &trin) / trin.l;		// s

	CrossProd(&f, &u, &w);
	bcoord->z = DotProd(&f, &trin) / trin.l;		// t

	bcoord->x = 1.0 - (bcoord->y + bcoord->z);			// r

}

/*
// Return area of an arbitrary triangle (not fast!)

double inline TriArea(dXYZ *a, dXYZ *b, dXYZ *c)
{

	cM_Vector		u, v, w;

	// Orthogonal projection top->base
	u.x = c->x - a->x; u.y = c->y - a->y; u.z = c->z - a->z;
	w.x = u.x; w.y = u.y; w.z = u.z;
	u.ComputeLength();
	u.Normalize();
	v.x = b->x - a->x; v.y = b->y - a->y; v.z = b->z - a->z;
	v.ComputeLength();
	v.Normalize();

	// w is c1 orthogonaly projected onto vector u
	float	dot = lMath.DotProd(&u, &v);
	w.x *= dot; w.y *= dot; w.z *= dot;
	w.x += a->x; w.y += a->y; w.z += a->z;

	// Compute two-tri area...
	v.x = b->x - w.x; v.y = b->y - w.y; v.z = b->z - w.z;
	v.ComputeLength();

	u.x = w.x - a->x; u.y = w.y - a->y; u.z = w.z - a->z;
	u.ComputeLength();
	double area = (v.l * u.l) / 2;
	u.x = w.x - c->x; u.y = w.y - c->y;
	u.z = w.z - c->z; u.ComputeLength();
	area += (v.l * u.l) / 2;

	return area;

}
*/
