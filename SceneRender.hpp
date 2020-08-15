#pragma once

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/mat3x3.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include <SFML/Graphics.hpp>

#include <functional>
#include <math.h>

#include <iostream>
#include <vector>

#define M_PI 3.14159265358979323846

extern float mouse_x, mouse_y, angle;
extern int light_mode;
extern bool demo;
extern int refra;
extern int sdep;

void drawLine(float x, float y, float x1, float y1);
void drawLine(float x, float y, float x1, float y1, float, float);
void drawCircle(float x, float y, float r);

void SceneRender_updateColor(float x, float y, sf::Color* color);
void SceneRender_castRays();
void SceneRender_detailCast();
bool SceneRender_select_hover();
void SceneRender_select(bool);

double solveCubic(float, float, float, float, std::function<double(double)>);
inline float square(float a) { return a * a; }

#define MINF [](double d1, double d2) { return min (d1, d2); }


glm::mat3 getTransformation(glm::vec2 position, float angle);

//struct Distance
//{
//  Distance(float x0, float x1, float y0, float y1, bool inside)
//  {
//    m_dx = (x1 - x0);
//    m_dy = (y1 - y0);
//    m_inside = inside;
//  }
//
//  float getDistance()
//  {
//
//  }
//  float m_dx, m_dy;
//  bool m_inside;
////};
//struct ClosestPoint
//{
//  ClosestPoint(float x, float y, bool inside)
//  {
//    m_x = x;
//    m_y = y;
//    m_inside = inside;
//  }
//  float getDistance(float x, float y)
//  {
//    return sqrt(square(x - m_x) + square(y - m_y)) * (m_inside ? -1.0 : 1.0);
//  }
//  float m_x, m_y;
//  bool m_inside;
//};

float getDistanceR(float, float, glm::vec2);

class Geometry
{
public:
  //virtual ClosestPoint getClosestPoint(glm::vec2 pt) = 0;
  virtual float getSignedDistance(glm::vec2 pt) = 0;
};

class GeometryObject : public Geometry
{
public:
  GeometryObject(Geometry* geo)
  {
    auto otw = glm::mat3(1);
    m_otw = otw;
    m_wto = glm::inverse(otw);
    m_invert = false;
    m_geo = geo;
  }
  GeometryObject(Geometry *geo, glm::mat3 otw, bool invert = false)
  {
    m_otw = otw;
    m_wto = glm::inverse(otw);
    m_invert = invert;
    m_geo = geo;
  }

  float getSignedDistance(glm::vec2 pt)
  {
    return m_geo->getSignedDistance(m_wto * glm::vec3(pt, 1)) * (m_invert ? -1: 1);
  }

  void offset(float dx, float dy)
  {
    m_otw = glm::translate (m_otw, glm::vec2(dx, dy));
    m_wto = glm::inverse(m_otw);
  }
protected:
  glm::mat3 m_otw;
  glm::mat3 m_wto;
  Geometry *m_geo;
  bool m_invert = false;
};

//class EllipseGeometry : public Geometry
//{
//public:
//  EllipseGeometry(float dist = 0.2f)
//  {
//    m_dist = dist;
//  }
//  float getSignedDistance(glm::vec2 pt)
//  {
//    float ax = -abs(p.x);
//    double xx = solveCubic(2 * square(m_amp), 0, 1.f - 2.f * p.y * m_amp, -ax, [=](double d) {return square(ax - xx) + square(m_amp * square(xx) - p.y); });
//  }
//  ClosestPoint getClosestPoint(glm::vec2 p)
//  {
//
//    float ax = -abs(p.x);
//    double xx = solveCubic(2 * square(m_amp), 0, 1.f - 2.f * p.y * m_amp, -ax, [=](double d) {return square(ax - xx) + square(m_amp * square(xx) - p.y); });
//    return ClosestPoint(xx * (p.x < 0 ? 1.0f : -1.0f), m_amp * square(xx), p.y > m_amp * square(xx));
//  }
//  glm::vec3 getNormal(glm::vec2 pt)
//  {
//    return glm::vec3(0.0f, 1.0f, 0.0f);
//  }
//private:
//  float m_dist;
//};

class ParabolaGeometry : public Geometry
{
public:
  ParabolaGeometry(float amplitude = 1.0f)
  {
    m_amp = amplitude;
  }
  float getSignedDistance(glm::vec2 p)
  {
    float ax = -abs(p.x);
    double xx = solveCubic(2 * square(m_amp), 0, 1.f - 2.f * p.y * m_amp, -ax, [=](double d) {return square(ax - xx) + square(m_amp * square(xx) - p.y); });
    return getDistanceR (xx * (p.x < 0 ? 1.0f : -1.0f), m_amp * square(xx), p) * (p.y > m_amp * square(xx) ? -1 : 1);
  }
private:
  float m_amp;
};

class RectangleGeometry : public Geometry
{
public:
  RectangleGeometry(float width=1.0f, float height = 1.0f)
  {
    m_width_half = width / 2.0;
    m_height_half = height / 2.0;
  }

  float getSignedDistance(glm::vec2 p)
  {
    float x = abs(p.x);
    float y = abs(p.y);
    float xx = p.x > 0 ? 1.0 : -1.0;
    float yy = p.y > 0 ? 1.0 : -1.0;
    bool inside = x < m_width_half && y < m_height_half;
    if (x > m_width_half && y > m_height_half)
    {
      return getDistanceR(m_width_half * xx, m_height_half * yy, p);
    }
    else if (x - m_width_half > y - m_height_half)
    {
      return getDistanceR(m_width_half * xx, p.y, p) * (x > m_width_half ? 1 : -1);
    }
    else
    {
      return getDistanceR(p.x, m_height_half * yy, p) * (y > m_height_half ? 1 : -1);
    }
  }
private:
  float m_width_half, m_height_half;
};

class SphereGeometry : public Geometry
{
public:
  SphereGeometry(float radius)
  {
    m_r = radius;
  }
  float getSignedDistance(glm::vec2 pt)
  {
    float d = getDistanceR(0, 0, pt);
    return d - m_r;
  }
private:
  float m_r;
};

class OrGeometry : public GeometryObject
{
public:
  OrGeometry (std::vector<Geometry*> v, std::function<double(double, double)> f) : GeometryObject(NULL)
  {
    func = f;
    m_geometries = v;
  }

  float getSignedDistance(glm::vec2 pt)
  {
    auto p = m_wto * glm::vec3(pt, 1);
    bool inited = false;
    float dist = 0;
    for (auto g : m_geometries)
    {
      if (!inited)
      {
        dist = g->getSignedDistance(p);
        inited = true;
        continue;
      }
      dist = func(dist, g->getSignedDistance(p));
    }
    return dist;
  }

private:
  std::function<double(double, double)> func;
  std::vector<Geometry*> m_geometries;
};

class ComGeometry : public GeometryObject
{
public:
  ComGeometry(Geometry* g1, Geometry* g2, std::function<double(double, double)> f) : GeometryObject(NULL)
  {
    func = f;
    m_g1 = g1;
    m_g2 = g2;
  }

  float getSignedDistance(glm::vec2 pt)
  {
    auto p = m_wto * glm::vec3(pt, 1);
    return func(m_g1->getSignedDistance(p), m_g2->getSignedDistance(p)) * (m_invert ? -1 : 1);
  }
private:
  std::function<double(double, double)> func;
  Geometry* m_g1, * m_g2;
};

//class ContainerGeometry : public PrimitiveGeometry
//{
//
//  ContainerGeometry(PrimitiveGeometry *primitive, glm::mat3 trans, std::function<double(double, double)> f, bool invert = false)
//  {
//    m_otw = trans;
//    m_wto = glm::inverse(m_otw);
//    m_scale = glm::determinant(m_otw);
//    m_invert = invert;
//    func = f;
//    m_primitive = primitive;
//  }
//
//  float getSignedDistance(glm::vec2 pt)
//  {
//    return m_primitive->getSignedDistance(pt);
//  }
//private:
//  PrimitiveGeometry *m_primitive;
//  std::function<double(double, double)> func;
//};