#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include "SceneRender.hpp"

#include <iostream>

using namespace std;

int WIDTH = 1000, HEIGHT = 720;
float scale = 1.0f / WIDTH;
float s_x = 0.5f, s_y = -0.5f;
float angle = 0;
int light_mode = 1;
bool demo = true;
int sdep = 0;

sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT, 32), "Test");

inline float map_x(float x)
{
  return (x - s_x ) / scale + WIDTH / 2;
}
inline float map_y(float y)
{
  return (1.0f - y + s_y) / scale + HEIGHT / 2;
}
inline float unmap_x(float x)
{
  return (x - WIDTH / 2) * scale + s_x + 0.000007f;
}
inline float unmap_y(float y)
{
  return (1.0f - (y - HEIGHT / 2) * scale) + s_y + 0.000007f;
}

bool rerender = false;
int refra = 0;
float mouse_x, mouse_y;
sf::RenderTexture renderTexture;
float interpolate(float time, float x_start, float y_end)
{
  return time * y_end  + (1 - time) * x_start;
}
void drawLine(float x, float y, float x1, float y1)
{
  sf::Color color = sf::Color::White;
  color.a = 150;
  sf::Vertex line[2] =
  {
    sf::Vertex(sf::Vector2f(map_x(x), map_y(y)), color),
    sf::Vertex(sf::Vector2f(map_x(x1), map_y(y1)), color)
  };
  window.draw(line, 2, sf::Lines);
}
void drawLine(float x, float y, float x1, float y1, float w, float alp)
{
  sf::Color color = sf::Color::White;
  if (w >= 380 and w < 440)
  {
    color.r = -(w - 440.f) / (440.f - 380.f) * 255;
    color.g = 0;
    color.b = 255;
  }
  else if (w >= 440 and w < 490) {
    color.r = 0;
    color.g = (w - 440.) / (490. - 440.) * 255;
    color.b = 255;
  }
  else if (w >= 490 and w < 510) {
    color.r = 0;
    color.g = 255;
    color.b = -(w - 510.) / (510. - 490.) * 255;
  }
  else if (w >= 510 and w < 580) {
    color.r = (w - 510.) / (580. - 510.) * 255;
    color.g = 255;
    color.b = 0;
  }
  else if (w >= 580 and w < 645) {
    color.r = 255;
    color.g = -(w - 645.) / (645. - 580.) * 255;
    color.b = 0;
  }
  else if (w >= 645 and w <= 780) {
    color.r = 255;
    color.g = 0;
    color.b = 0;
  }
  else {
    color.r = 0;
    color.g = 0;
    color.b = 0;
  }
  color.a = 90;// 255 * alp;
  sf::Vertex line[2] =
  {
    sf::Vertex(sf::Vector2f(map_x(x), map_y(y)), color),
    sf::Vertex(sf::Vector2f(map_x(x1), map_y(y1)), color)
  };
  window.draw(line, 2, sf::Lines);
}
void drawCircle(float x, float y, float r)
{
  sf::CircleShape circle(r / scale);
  //cout << r << endl;
  circle.setPosition(sf::Vector2f(map_x (x - r), map_y(y + r)));
  //circle.setColor(sf::Color(0, 0, 0, 0));
  circle.setOutlineColor(sf::Color(250, 150, 100));
  circle.setFillColor(sf::Color(0, 0, 0, 0));
  circle.setOutlineThickness(1);
  window.draw(circle);
}
sf::Uint8* pixels;
sf::Image        image;
sf::Sprite       sprite;
sf::Texture backgroundScene;
void bake_bg(int step)
{
  for (int x = 0; x < WIDTH; x += 1)
  {
    for (int y = 0; y < HEIGHT; y += 1)
    {
      *(((sf::Color*) pixels) + y * WIDTH + x) = sf::Color::Black;
    }
  }
  for (int x = 0; x < WIDTH; x+= step)
  {
    for (int y = 0; y < HEIGHT; y+= step)
    {
      *(((sf::Color*) pixels) + y * WIDTH + x) = sf::Color::Black;
      SceneRender_updateColor(unmap_x(x), unmap_y(y), (sf::Color*) pixels + (y * WIDTH + x));
      for (int xx = x; xx < x + step; xx++)
        for (int yy = y; yy < y + step; yy++)
        {
          *((sf::Color*) pixels + (yy * WIDTH + xx)) = *((sf::Color*) pixels + (y * WIDTH + x));
        }
    }
  }
  backgroundScene.update(pixels);
  sprite.setTexture(backgroundScene);
};

bool r = true;
int main()
{
  cout << "saved alivdfge filsdfe " << endl;
  //tgui::Gui gui{ window };
  /* Baking background scene start */
  pixels = new sf::Uint8[WIDTH * HEIGHT * 4];
  if (!backgroundScene.create(WIDTH, HEIGHT))
  {
    cout << "Texture creation failed!\n";
  }
  if (!renderTexture.create(WIDTH, HEIGHT)) {
    cout << "Texture render reation failed\n";
  }
  /* Baking background scene end */
  // ...
  float a = 0;
  bool should_bake = true;
  int step_size = 4;
  //bool update = false;
  bool select = false;
  sf::View view = window.getDefaultView();
  static bool selected = false;
  bool detailed_cast = false;
  while (window.isOpen ())
  {
    a += 0.01;
    sf::Event event;
    static bool update = true;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Resized) {
        // resize my view
        view.setSize({
                static_cast<float>(event.size.width),
                static_cast<float>(event.size.height)
          });
        WIDTH = event.size.width;
        HEIGHT = event.size.height;
        window.setView(view);
        pixels = new sf::Uint8[WIDTH * HEIGHT * 4];
        if (!backgroundScene.create(WIDTH, HEIGHT))
        {
          cout << "Texture creation failed!\n";
        }
        if (!renderTexture.create(WIDTH, HEIGHT)) {
          cout << "Texture render reation failed\n";
        }
        scale = 1.0f / WIDTH;
      }
      if (event.type == sf::Event::Closed)
        window.close();
      if (event.type == sf::Event::MouseButtonReleased)
      {
        SceneRender_select(false);
        selected = false;
      }
      else if (event.type == sf::Event::MouseButtonPressed)
      {
        SceneRender_select(true);
        selected = true;
      }
      switch (event.type)
      {

        // key pressed
      case sf::Event::MouseWheelScrolled:
        if (event.mouseWheelScroll.delta)
        {
          float s = event.mouseWheelScroll.delta > 0 ? 1/1.1f : 1.1f;
          for (int i = 0; i < abs(event.mouseWheelScroll.delta); i++)
          {
            scale *= s;
          }
          should_bake = true;
          rerender = true;
        }
        break;
      case sf::Event::KeyPressed:
        if (event.key.code == sf::Keyboard::Escape)
        {
          window.close();
        }else if (event.key.code == sf::Keyboard::E)
        {
          update ^= true;
        }
        else if (event.key.code == sf::Keyboard::C)
        {
          detailed_cast = true;
          rerender = true;
        }
        else if (event.key.code == sf::Keyboard::B)
        {
          step_size = step_size == 4 ? 1 : 4;
          should_bake = true;
          rerender = true;
        }
        else if (event.key.code == sf::Keyboard::R)
        {
          r = true;
          should_bake = true;
          rerender = true;
        }
        else if (event.key.code == sf::Keyboard::Up)
        {
          sdep += 1;
          rerender = true;
        }
        else if (event.key.code == sf::Keyboard::O)
        {
          r = true;
          rerender = true;
          demo ^= true;
        }
        else if (event.key.code == sf::Keyboard::R)
        {
          refra += 1;
          rerender = true;
        }
        else
        {
          switch (event.key.code)
          {
          case sf::Keyboard::Num1:
            light_mode = 1; rerender = true; break;
          case sf::Keyboard::Num2:
            light_mode = 2; rerender = true; break;
          case sf::Keyboard::Num3:
            light_mode = 3; rerender = true; break;
          case sf::Keyboard::Num4:
            light_mode = 4; rerender = true; break;
          default: break;
          }
          rerender = true;
        }
        break; //light_mode
        //case sf::Event::MouseButtonEvent
      }
      //gui.handleEvent(event);
    }
    /*if (sf::Keyboard::isKeyPressed(sf:/*:Keyboard::M) && !selected)
    {
      SceneRender_select();
    } else if (sf::Keyboard::isKeyReleased(sf::Keyboard::M) && selected)
    {
      SceneRender_select();
    }*/
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
      s_y += 0.05;
      should_bake = true;
      rerender = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
      s_y -= 0.05;
      should_bake = true;
      rerender = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
      s_x += 0.05;
      should_bake = true;
      rerender = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
      s_x -= 0.05;
      should_bake = true;
      rerender = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
      angle += 0.03;
      rerender = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
      angle -= 0.03;
      rerender = true;
    }

    if (SceneRender_select_hover())
    {
      select ^= true;
      should_bake = true;
    }
    if (should_bake || selected)
    {
      bake_bg(step_size);
      should_bake = false;
      rerender = true;
      //cout << "shoudl bake" << endl;
    }
    static sf::Vector2i position = sf::Vector2i(0, 0);
    if (update)
    {
      auto newpos = sf::Mouse::getPosition(window);
      if (position != newpos) rerender = true;
      position = newpos;
      mouse_x = unmap_x(position.x);
      mouse_y = unmap_y(position.y);
      //cout << mouse_x << ", " << mouse_y << endl;
    }
    // ...
    // ...
    //cout << sprite.getScale().x << endl;
    static int frame = 32;
    if (false)
    {
      static float i = 32 * 0.016f;

      
      if (i < 1.0f)
      {
        float n = i/1.0f;
        mouse_x = interpolate(n, 0.107f, 0.393f);
        mouse_y = interpolate(n, 0.505f, 0.804f);
        rerender = true;
      }
      else if (i < 2.0f)
      {
        float n = (i- 1.0f) / 1.0f;
        mouse_x = interpolate(n, 0.393f, 0.6f);
        mouse_y = interpolate(n, 0.804f, 0.79f);
        rerender = true;
      }
      else if (i < 3.0f)
      {
        float n = (i - 2.0f) / 1.0f;
        mouse_x = interpolate(n, 0.6f, 0.556);
        mouse_y = interpolate(n, 0.79f, 0.417);
        rerender = true;
      }
      else if (i < 4.0f)
      {
        float n = (i - 3.0f) / 1.0f;
        mouse_x = interpolate(n, 0.556, 0.809);
        mouse_y = interpolate(n, 0.417, 0.574);
        rerender = true;
      }
      i += 0.016f;
      //cout << mouse_x << ", " << mouse_y << endl;
    }
    if (rerender)
    {
      rerender = false;
      renderTexture.clear();
      window.draw(sprite);
      //SceneRender_detailCast();
      if (detailed_cast)
      {
        detailed_cast = false;
        SceneRender_detailCast();
      }
      else
      {
        try
        {
          SceneRender_castRays();
        }
        catch (int e)
        {
          cout << "An exception occurred. Exception Nr. " << e << '\n';
        }
      }
      //window.draw(s);
      //gui.draw();
      if (r && false)
      {
        //float x_s, y_s, x_e, y_e = ;
        sf::Vector2u windowSize = window.getSize();
        sf::Texture texture;
        texture.create(windowSize.x, windowSize.y);
        texture.update(window);
        sf::Image screenshot = texture.copyToImage();
        std::string str = "pics/";
        str += std::to_string(frame);
        str += ".png";
        cout << frame << endl;
        screenshot.saveToFile(str);
        cout << "saved to file " << str << endl;
        if (frame == 4*60+5)
        {
          exit(0);
        }
        frame++;
      }
      else
      {
        window.display();
      }
      window.clear();
    }
  }

  delete[] pixels;
  return 0;
}
