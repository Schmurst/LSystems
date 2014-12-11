////////////////////////////////////////////////////////////////////////////////
//
// Octet: (C) Andy Thomason 2012-2014
//
// L - System generator by Sam Hayhurst
//

#include <fstream>
#include "L_system.h"

namespace octet {
  /// Scene containing a box with octet.
  class LSystems : public app {
  private:
    // Debug bools
    enum { LS_DEBUG_PARSER = 1, LS_DEBUG_ITERATE = 1};

    // scene for drawing box
    ref<visual_scene> app_scene;
    camera_instance *camera;
    material *mat;

    ref<L_system> tree;
    dynarray<string> FILENAMES;

    void setFileNames(){
      FILENAMES.push_back("assets/Lsystems/Tree1.txt");
      FILENAMES.push_back("assets/Lsystems/Tree2.txt");
      FILENAMES.push_back("assets/Lsystems/Tree3.txt");
      FILENAMES.push_back("assets/Lsystems/Tree4.txt");
      FILENAMES.push_back("assets/Lsystems/Tree5.txt");
      FILENAMES.push_back("assets/Lsystems/Tree6.txt");
      FILENAMES.push_back("assets/Lsystems/Tree7.txt");
      FILENAMES.push_back("assets/Lsystems/Tree8.txt");
    }

    void loadNewTree(int index){
      tree = new L_system();
      tree->loadFile(FILENAMES[index]);
      tree->iteration(1);
      tree->initialiseDrawParams();
      tree->interpret_axiom();
      scene_node * testNode = tree->getNode();
      app_scene->add_child(testNode);
      app_scene->get_mesh_instance(0)->set_mesh(tree->getMesh());
    }

  public:
    /// this is called when we construct the class before everything is initialised.
    LSystems(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();

      app_scene->create_default_camera_and_lights();
      camera = app_scene->get_camera_instance(0);
      camera->set_far_plane(1000.0f);
      camera->set_near_plane(0.01f);
      app_scene->get_camera_instance(0)->get_node()->access_nodeToParent().translate(vec3(0, 10, 10));

      param_shader *shader = new param_shader("shaders/default.vs", "shaders/simple_color.fs");
      mat = new material(vec4(1, 0, 0, 1), shader);

      setFileNames();

      tree = new L_system();
      tree->loadFile(FILENAMES[0]);
      tree->iteration(4);
      tree->initialiseDrawParams();
      tree->interpret_axiom();
      scene_node * testNode = tree->getNode();
      app_scene->add_child(testNode);
      app_scene->add_mesh_instance(new mesh_instance(testNode, tree->getMesh(), mat));

    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      node->rotate(1, vec3(0, 1, 0));
      node->transform(vec3(0));

      for (int i = 1; i < 8; ++i){
        if (is_key_going_down((char)(((int)'0') + i))){
          loadNewTree(i - 1);
        }
      }

      if (is_key_going_down('O')){
        tree->incrementIteration();
      }

      if (is_key_going_down('P')){
        tree->decrementIteration();
      }

      if (is_key_down('K')){
        tree->incrementAngle();
      }

      if (is_key_down('L')){
        tree->decrementAngle();
      }

      if (is_key_down('N')){
        tree->incrementTranslation();
      }

      if (is_key_down('M')){
        tree->decrementTranslation();
      }

      if (is_key_down('U')){
        tree->incrementRadius();
      }

      if (is_key_down('I')){
        tree->decrementRadius();
      }

      if (is_key_going_down('Z')){
        tree->altStochasticity();
      }

      // Camera controls
      if (is_key_down('Q')){
        camera->get_node()->translate(vec3(0, 1.0f, 0));
      }

      if (is_key_down('E')){
        camera->get_node()->translate(vec3(0, -1.0f, 0));
      }

      if (is_key_down('W')){
        camera->get_node()->translate(vec3(0, 0, -1.0f));
      }

      if (is_key_down('S')){
        camera->get_node()->translate(vec3(0, 0, 1.0f));
      }

      if (is_key_down('A')){
        camera->get_node()->translate(vec3(-1.0f, 0, 0));
      }

      if (is_key_down('D')){
        camera->get_node()->translate(vec3(1.0f, 0, 0));
      }
    }

  };
}
