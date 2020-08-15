#include "SceneRender.hpp"
#define _USE_MATH_DEFINES
#include <cmath> 

using namespace std;
const std::function<double(double, double)> MAX_OP = [](double d1, double d2) { return std::max(d1, d2); };

ParabolaGeometry g_parabola = ParabolaGeometry(2);
RectangleGeometry g_square = RectangleGeometry(0.2, 0.5);
RectangleGeometry g_mirror = RectangleGeometry(0.1, 0.5);
SphereGeometry g_sphere = SphereGeometry(0.1);

GeometryObject g_lens(&g_parabola, getTransformation(glm::vec2(0.45, 0.5), -M_PI / 2), true);
GeometryObject g_len2s(&g_parabola, getTransformation(glm::vec2(0.5, 0.0), 0), true);
GeometryObject g_len(&g_square, getTransformation(glm::vec2(0.5, 0.5),0), false);
GeometryObject g_side_mirror(&g_mirror, getTransformation(glm::vec2(0.8, 0.7), M_PI/4));
GeometryObject g_sphereobj(&g_sphere, getTransformation(glm::vec2(0.25, 0.25), 0), false);
ParabolaGeometry g_rparabola = ParabolaGeometry(2.9);
//#define g_thick_convex_lens g_len2s

GeometryObject g_concave_lens1(&g_parabola, getTransformation(glm::vec2(0.65, 0.3), -M_PI / 2), false);
GeometryObject g_concave_lens2(&g_parabola, getTransformation(glm::vec2(0.75, 0.3), M_PI / 2), false);

ComGeometry g_one_side_concave_lens(&g_lens, &g_len, [](float d1, float d2) { return std::max(d1, d2); });
OrGeometry g_side_mirror_com({&g_side_mirror}, [](float d1, float d2) { return std::max(d1, d2); });
ComGeometry g__double_side_concave_lens(&g_concave_lens1, &g_concave_lens2, [](float d1, float d2) { return std::max(d1, d2); });
const std::vector<Geometry*> g_all = { &g_one_side_concave_lens , &g__double_side_concave_lens, &g_side_mirror_com, &g_sphereobj };
OrGeometry g_lens_scene(g_all, [](float d1, float d2) { float k = 0.1f; auto h = std::max(k - abs(d1 - d2), 0.0f) / k; return std::min(d1, d2) - h * h * h * k / 6.0f; });
//ComGeometry g_thick_convex_lens(&g_side_mirror, &g_lens_scene, [](float d1, float d2) { return std::min(d1, d2); });
#define g_thick_convex_lens g_lens_scene
Geometry* g_selected_geometry;
void SceneRender_updateColor(float x, float y, sf::Color* color)
{
  auto d = g_thick_convex_lens.getSignedDistance(glm::vec2(x, y));
  if (d < 0.f && d > -0.003f)//(abs(cp.getDistance(x, y)) < 0.01)
  {
    if (g_selected_geometry && d == g_selected_geometry->getSignedDistance(glm::vec2(x, y)))
    {
      *color = sf::Color::Red;
    }
    else
    {
      *color = sf::Color::Blue;
      color->g = d > 0.3 ? 255 : 0;
    }
  }
  else if (d < -0.003f)
  {
    *color = sf::Color::White;
    color->r = 25;
    color->b = 25;
    color->g = 25;
  }
  else
  {
    *color = sf::Color::Black;
  }
}


bool g_selected = false;
struct Ray
{
public:
  Ray(glm::vec3 pos, glm::vec3 dir, float alpha, float color = 0)
  {
    m_position = pos;
    m_direction = dir;
    if (color == 0)
    {
      int r = rand() % 300;
      m_color = (float) r + 370.0f;
    }
    else
      m_color = color;
    n = 1.15 - (color - 300.0f) / 3000.0f;
    m_alpha = alpha;
  }
  glm::vec3 m_position, m_direction;
  float m_color;
  float n;
  float m_alpha;
};
void castRay(Ray* ray, int depth);

float getDist(glm::vec3 pos)
{
  return g_thick_convex_lens.getSignedDistance(glm::vec2(pos));
}

float getAbsDist(glm::vec3 pos)
{
  return std::abs( g_thick_convex_lens.getSignedDistance(glm::vec2(pos)));
}

inline glm::vec2 Refract(const glm::vec2& incidentVec, const glm::vec2& a, float eta)
{
  float N_dot_I = glm::dot(a, incidentVec);
  auto normal = a;
  if (N_dot_I > 0)
  {
    normal = -normal;
    N_dot_I = glm::dot(normal, incidentVec);
  }
  float k = 1.f - eta * eta * (1.f - N_dot_I * N_dot_I);
  if (k < 0.f)
    return glm::vec2(0);
  else
    return eta * incidentVec - (eta * N_dot_I + sqrtf(k)) * normal;
}

void castRay(Ray *ray, int depth = 7, int sd = 100)
{
  if (sd < 100 - sdep && demo) return;
  //Ray
  glm::vec3 pos = ray->m_position;
  glm::vec3 dir = ray->m_direction;
  if (demo && depth > 1) depth = 1;
  if (depth <= 0) return;
  float dist = getAbsDist(pos);
  if (dist > 10.0)
  {
    return;
  }
  //drawCircle(pos.x, pos.y, dist);
  auto newPos = pos + glm::normalize(dir) * dist;
  if (dist < 0.0001)
  {
    auto newp = glm::vec2(newPos.x, newPos.y);
    glm::vec3 npv = glm::vec3(newp.x, newp.y, 0.0f);
    const float epsilon = 0.00006;
    auto dx = getDist(newPos + glm::vec3(epsilon, 0.00, 0.0))
      - getDist(newPos - glm::vec3(epsilon, 0.00, 0.0));
    auto dy = getDist(newPos + glm::vec3(0.0, epsilon, 0.0))
      - getDist(newPos - glm::vec3(0.0, epsilon, 0.0));
    auto normal = glm::normalize(glm::vec2(dx, dy));
    //drawLine(pos.x, pos.y, pos.x + normal.x, pos.y + normal.y);
    auto newDir = glm::reflect(glm::vec2(dir), normal);
    auto newDirRefract = Refract(glm::vec2(dir), normal, depth % 2 == 0 ? ray->n : 1.0f / ray->n);
    if (newDirRefract != glm::vec2(0) && (refra % 3 == 0 || refra % 3 == 1))
    {
      Ray newRay = Ray(newPos + glm::vec3(glm::normalize(newDirRefract), 0.0f) * 0.001f, glm::vec3(newDirRefract, 0.0f), ray->m_alpha, ray->m_color);
      castRay(&newRay, depth - 1, sd - 1);
    }
    if (refra % 3 == 0 || refra % 3 == 2)
    {
      Ray newRay = Ray(newPos + glm::vec3(glm::normalize(newDir), 0.0f) * 0.001f, glm::vec3(newDir, 0.0f), ray->m_alpha, ray->m_color);
      castRay(&newRay, depth - 1, sd - 1);
    }
    return;
  }
  if (demo)
  drawCircle(pos.x, pos.y, dist);
  drawLine(pos.x, pos.y, newPos.x, newPos.y, ray->m_color, ray->m_alpha);
  Ray newRay = Ray(newPos, dir, ray->m_alpha, ray->m_color);
  castRay(&newRay, depth, sd-1);
}

float old_x, old_y;

bool SceneRender_select_hover()
{
  if (g_selected && g_selected_geometry)
  {
    ((GeometryObject*)g_selected_geometry)->offset(mouse_x - old_x, mouse_y - old_y);
    old_x = mouse_x;
    old_y = mouse_y;
  }
  else
  {
    float closes = 0.0f;
    g_selected_geometry = NULL;
    bool re = false;
    for (auto geo : g_all)
    {
      float d = geo->getSignedDistance(glm::vec2(mouse_x, mouse_y));
      if (d < closes)
      {
        d = closes;
        g_selected_geometry = geo;
        re = true;
      }
    }
    return re;
  }
}
void SceneRender_select(bool selected)
{
  if (selected == false)
  {
    g_selected = false; return;
  }
  if (g_selected_geometry == NULL) return;
  g_selected = selected;
  if (selected)
  {
    old_x = mouse_x;
    old_y = mouse_y;
  }
}

int amo = 200;

void SceneRender_castRays()
{
  //cout << "casting" << amo << endl;
  for (int i = 0; i < amo; i++)
  {
    auto a = 0 + angle;// (i / (float)(amo - 1)) * 0.04f * (float)M_PI + angle;
    switch (light_mode)
    {
    case 1:
      a = (float)M_PI + angle;
      break;
    case 2:
      a = (i / (float)(amo - 1)) * 0.1f * (float)M_PI + angle; break;
    case 3:
      a = (i / (float)(amo - 1)) * 1.0f * (float)M_PI + angle; break;
    case 4:
      a = (i / (float)(amo - 1)) * 2.0f * (float)M_PI + angle; break;
    default:
      break;
    }
    Ray ray = Ray(glm::vec3(mouse_x, mouse_y, 1), glm::vec3(-sin(a), cos(a), 0.0), 5000.0f / amo);
    castRay(&ray);
  }
}
void SceneRender_detailCast()
{
  auto old = amo;
  amo = 12000;
  SceneRender_castRays();
  //cout << "Done" << endl;
  amo = old;
}

glm::mat3 getTransformation(glm::vec2 position, float angle)
{
  glm::mat3 m_otw = glm::translate(glm::mat3(1), position);
  m_otw = glm::rotate(m_otw, angle);
  return m_otw;
}

float getDistanceR(float x, float y, glm::vec2 point)
{
  return sqrt(square(x - point.x) + square(y - point.y));
}

double solveCubic(float aa, float ba, float ca, float da, std::function<double(double)> f)
{
  double a = aa, b = ba, c = ca, d = da;
  b /= a;
  c /= a;
  d /= a;

  double disc, q, r, rwmo, s, t, t1, r13;
  q = (3.0 * c - (b * b)) / 9.0;
  r = -(27.0 * d) + b * (9.0 * c - 2.0 * (b * b));
  r /= 54.0;
  disc = q * q * q + r * r;
  t1 = (b / 3.0);

  double x1_real, x2_real, x3_real;
  double x2_imag, x3_imag;
  if (disc > 0)
  {
    s = r + sqrt(disc);
    s = s < 0 ? -cbrt(-s) : cbrt(s);
    t = r - sqrt(disc);
    t = t < 0 ? -cbrt(-t) : cbrt(t);
    x1_real = -t1 + s + t;
    t1 += (s + t) / 2.0;
    x3_real = x2_real = -t1;
    t1 = sqrt(3.0) * (-t + s) / 2;
    return x1_real;
    if (f(x1_real) < f(x2_real))
      return x1_real;
    return x2_real;
  }
  else if (disc == 0)
  {
    //std::cout << "HElp!" << std::endl;
    x3_imag = x2_imag = 0;
    r13 = r < 0 ? -cbrt(-r) : cbrt(r);
    x1_real = -t1 + 2.0 * r13;
    x3_real = x2_real = -(r13 + t1);
    if (f(x1_real) < f(x2_real))
      return x1_real;
    return x2_real;
  }
  // Only option left is that all roots are real and unequal (to get here, q < 0)
  else
  {
    //std::cout << "HElp2!" << std::endl;
    x3_imag = x2_imag = 0;
    q = -q;
    rwmo = q * q * q;
    rwmo = acos(r / sqrt(rwmo));
    r13 = 2.0 * sqrt(q);
    x1_real = -t1 + r13 * cos(rwmo / 3.0);
    x2_real = -t1 + r13 * cos((rwmo + 2.0 * M_PI) / 3.0);
    x3_real = -t1 + r13 * cos((rwmo + 4.0 * M_PI) / 3.0);
    if (f(x1_real) < f(x2_real))
    {
      if (f(x3_real) < f(x1_real))
        return x3_real;
      return x1_real;
    }
    else
    {
      if (f(x3_real) < f(x2_real))
        return x3_real;
      return x2_real;
    }
  }
}