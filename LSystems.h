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
    enum { LS_DEBUG_PARSER = 1, LS_DEBUG_ITERATE = 1 };

    // scene for drawing box
    ref<visual_scene> app_scene;
    camera_instance *camera;
    material *red;

    // L system trees
    ref<L_system> tree_1, tree_2, tree_3, tree_4, tree_5, tree_6;

    dynarray<ref<L_system>> lsystemStack;

    int currentLSystem = 0;

    void load_L_Systems(){
      // Make the first tree
      tree_1 = new L_system();
      tree_1->loadFile("assets/Lsystems/Tree1.txt");
      tree_1->iteration(tree_1->getIterations());
      printf("The length of the final string is: %i\n", tree_1->getAxiomSize());
      tree_1->calculateVertices(tree_1->getAxiom(tree_1->getIterations() - 1));
      lsystemStack.push_back(tree_1);

      // Make the 2nd tree
      tree_2 = new L_system();
      tree_2->loadFile("assets/Lsystems/Tree2.txt");
      tree_2->iteration(tree_2->getIterations());
      printf("The length of the final string is: %i\n", tree_2->getAxiomSize());
      tree_2->calculateVertices(tree_2->getAxiom(tree_2->getIterations() - 1));
      lsystemStack.push_back(tree_2);

      // Make the 3nd tree
      tree_3 = new L_system();
      tree_3->loadFile("assets/Lsystems/Tree3.txt");
      tree_3->iteration(tree_3->getIterations());
      printf("The length of the final string is: %i\n", tree_3->getAxiomSize());
      tree_3->calculateVertices(tree_3->getAxiom(tree_3->getIterations() - 1));
      lsystemStack.push_back(tree_3);

      // Make the 4th tree
      tree_4 = new L_system();
      tree_4->loadFile("assets/Lsystems/Tree4.txt");
      tree_4->iteration(tree_4->getIterations());
      printf("The length of the final string is: %i\n", tree_4->getAxiomSize());
      tree_4->calculateVertices(tree_4->getAxiom(tree_4->getIterations() - 1));
      lsystemStack.push_back(tree_4);

      // Make the 5th tree
      tree_5 = new L_system();
      tree_5->loadFile("assets/Lsystems/Tree5.txt");
      tree_5->iteration(tree_5->getIterations());
      printf("The length of the final string is: %i\n", tree_5->getAxiomSize());
      tree_5->calculateVertices(tree_5->getAxiom(tree_5->getIterations() - 1));
      lsystemStack.push_back(tree_5);

      // Make the 6th tree
      tree_6 = new L_system();
      tree_6->loadFile("assets/Lsystems/Tree6.txt");
      tree_6->iteration(tree_6->getIterations());
      printf("The length of the final string is: %i\n", tree_6->getAxiomSize());
      tree_6->calculateVertices(tree_6->getAxiom(tree_6->getIterations() - 1));
      lsystemStack.push_back(tree_6);
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
      red = new material(vec4(1, 0, 0.6f, 1), shader);

      load_L_Systems();

      // Place the target tree in the scene
      app_scene->add_child(tree_1->getNode());
      app_scene->add_mesh_instance(new mesh_instance(tree_1->getNode(), tree_1->getMesh(), red));


      // set the line width
      glLineWidth(1.0f);
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

      // mvoe the camera using the keys
      if (is_key_down('Q')){
        camera->get_node()->translate(vec3(0, 1.0f, 0));
      }

      if (is_key_down('E')){
        camera->get_node()->translate(vec3(0, -1.0f, 0));
      }

      if (is_key_down('W')){
        camera->get_node()->translate(vec3(0, 0, 1.0f));
      }

      if (is_key_down('S')){
        camera->get_node()->translate(vec3(0, 0, -1.0f));
      }

      if (is_key_down('A')){
        camera->get_node()->translate(vec3(-1.0f, 0, 0));
      }

      if (is_key_down('D')){
        camera->get_node()->translate(vec3(1.0f, 0, 0));
      }

      if (is_key_going_down('R')){
        if (currentLSystem != lsystemStack.size() - 1){
          ++currentLSystem;
          printf("current system: %i\n", currentLSystem);
        }
        else{
          currentLSystem--;
        }
        app_scene->add_child(lsystemStack[currentLSystem]->getNode());
        app_scene->get_mesh_instance(0)->set_mesh(lsystemStack[currentLSystem]->getMesh());
      }

      // rotate the tree
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      node->rotate(1.0f, vec3(0, 1, 0));
    }
  };
}
