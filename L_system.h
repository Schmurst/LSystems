////////////////////////////////////////////////////////////////////////////////
//
// Octet: (C) Andy Thomason 2012-2014
//
// L - System class by Sam Hayhurst
//
// note this build follows the typical L - System nomenclature
// F -> translate
// [ ] -> push and pop respectively
// + - increment or decrement angle around the z axis respectively
// other letters have no such special meanings

#include "../../octet.h"

namespace octet{
  class L_system : public resource{

    /// Debug bools
    enum { LS_DEBUG_PARSER = 0, LS_DEBUG_ITERATE = 0 };

    /// vertex structure
    struct myVertex{
      vec3p pos;
      uint32_t colour;
    };

  private:
    // char array containing the entire contents of the L systems instruction file.
    int fileSize;
    octet::string message;            // contains the message
    dynarray<uint8_t> lSystemFile;    // containts the entire file
    dynarray<char> alphabet;          // contains the alphabet
    dynarray<char> axiom;             // contains the axoim
    hash_map<char, string> rules;     // contains the rules
    float angle;                      // angle used for rotation
    int iterations;                   // number of iterations

    dynarray<dynarray<char>> axiomlist; // dynarray which stores previous axioms

    // drawing variables
    scene_node *node;
    mesh *_mesh;
    myVertex *vtx;
    dynarray<mat4t> placementStack;
    vec3 translateF = vec3(0, 0.2f, 0);

    // this function converts three floats into a RGBA 8 bit color, taken from Andy's geometry example
    static uint32_t make_color(float r, float g, float b) {
      return 0xff000000 + ((int)(r*255.0f) << 0) + ((int)(g*255.0f) << 8) + ((int)(b*255.0f) << 16);
    }

    /// finds the location of a given char inside a dynarray from a starting position
    int getPosition(dynarray<uint8_t> _array, char target, int startPos){
      for (int i = startPos; i < _array.size(); i++){
        if (_array[i] == target)
          return i;
      }
      return -1;
    }

    /// removes whitespace from a dynarray of chars, uses memcpy
    void removeWhiteSpace(dynarray<uint8_t> &_array){
      dynarray<char> rtn;
      for (int i = 0; i < _array.size(); i++){
        if (_array[i] != ' ' && _array[i] != '\n' && _array[i] != '\r' && _array[i] != '\t'){
          rtn.push_back(_array[i]);
        }
      }

      _array.resize(rtn.size());
      memcpy(_array.data(), rtn.data(), rtn.size()*sizeof(uint8_t));
    }

    /// This function iterates once over an L-system
    void iterate(){
      dynarray<char> new_array;
      string temp;
      int location;
      char c;
      if (LS_DEBUG_ITERATE) printf("Iterate started\n");

      for each (char c in axiom){
        if (rules.contains(c)){
          temp = rules[c];
          location = new_array.size();
          new_array.resize(new_array.size() + temp.size());
          memcpy(&new_array[location], temp.data(), temp.size());
        }
        else{
          location = new_array.size();
          new_array.resize(new_array.size() + 1);
          memcpy(&new_array[location], &c, 1);
        }

        if (LS_DEBUG_ITERATE){
          printf("temp: %s\n", temp);
          printf("Here is the current string: %.*s\n", new_array.size(), new_array.data());
        }
      }
      axiom.resize(new_array.size());
      memcpy(axiom.data(), new_array.data(), new_array.size());

      axiomlist.push_back(axiom);
    }

    /// This function constructs the rules for the Lsyatem from the file
    void constructLSystem(){
      octet::string check;
      check.set((char*)lSystemFile.data(), fileSize);

      // check for errors in the text file
      if (LS_DEBUG_PARSER) printf("\nContents of the check string are as follows:\n%s\n\n", check);
      if (check.find("Message") == -1 || check.find("Alphabet") == -1 || check.find("Axiom") == -1 || check.find("Rules") == -1 ||
        check.find("Angle") == -1 || check.find("Iterations") == -1){
        printf("Failed to find all keyWords, check your text file\n");
        assert(0);
      }
      else{
        printf("All Keywords have been found!\n");
      }

      // now put the lsystem rules into the containers
      // first the Message
      int a = 0, b = 0;
      a = getPosition(lSystemFile, ':', a) + 1;
      b = getPosition(lSystemFile, ';', a);
      message.set((char*)&lSystemFile[a], b - a);

      printf("Here is the message: %s\n", message);

      // Now the alphabet
      a = 0, b = 0;
      a = check.find("Alphabet");
      a += string("Alphabet").size() + 1;
      b = getPosition(lSystemFile, ';', a);

      for (int i = a; i < b; i++){
        if (lSystemFile[i] != ',')
          alphabet.push_back(lSystemFile[i]);
      }

      printf("Here is the Alphabet: %.*s\n", alphabet.size(), alphabet.data());

      // Now the Axiom
      a = 0, b = 0;
      a = check.find("Axiom");
      a += string("Axoim").size() + 1;
      b = getPosition(lSystemFile, ';', a);

      for (int i = a; i < b; i++){
        if (lSystemFile[i] != ',')
          axiom.push_back(lSystemFile[i]);
      }

      printf("Here is the Axiom: %.*s\n", axiom.size(), axiom.data());

      // Now the rules
      int noRules = 0;
      string ruleLHS, ruleRHS;
      a = 0, b = 0;
      a = check.find("Rules");
      a += string("Rules").size() + 1;
      noRules = atoi((char*)&lSystemFile[a]);
      if (LS_DEBUG_PARSER) printf("number of rules: %i\n", noRules);
      a += 2;

      // loop through the ammoutn of rules and add them to the hash map
      for (int i = 0; i < noRules; ++i){
        b = getPosition(lSystemFile, ';', a);
        ruleLHS.set((char*)&lSystemFile[a], 1);
        a += 2;
        ruleRHS.set((char*)&lSystemFile[a], b - a);
        if (LS_DEBUG_PARSER) printf("%s = %s\n", ruleLHS, ruleRHS);
        rules[ruleLHS[0]] = ruleRHS;
        a = b + 1;
      }

      // now find the Angle
      a = 0, b = 0;
      a = check.find("Angle");
      a += string("Angle").size() + 1;
      b = getPosition(lSystemFile, ';', a);
      string temp;
      temp.set((char*)&lSystemFile[a], b - a);
      angle = atof(temp);
      if (LS_DEBUG_PARSER) printf("The Angle is: %g\n", angle);

      // now find the number of iterations
      a = 0, b = 0;
      a = check.find("Iterations");
      a += string("Iterations").size() + 1;
      b = getPosition(lSystemFile, ';', a);
      temp.set((char*)&lSystemFile[a], b - a);
      temp += '\0';
      iterations = atof(temp);
      if (LS_DEBUG_PARSER) printf("The number of iterations is: %i\n", iterations);
    }

    /// This fucntion sets up th mesh to be drawn, taken and edited from Andy's geometry example
    void initialiseDrawParams() {
      // scene node
      node = new scene_node();

      // allocate vertices and indices into OpenGL buffers
      _mesh = new mesh();
      _mesh->allocate(sizeof(myVertex) * 100000, 0);
      _mesh->set_params(sizeof(myVertex), 0, 100000, GL_LINES, NULL);

      // describe the structure of my_vertex to OpenGL
      _mesh->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      _mesh->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, 12, GL_TRUE);

      // these write-only locks give access to the vertices and indices
      gl_resource::wolock vertexLock(_mesh->get_vertices());
      vtx = (myVertex *)vertexLock.u8();

      // insert some deafult drawing values
      mat4t matrix;
      placementStack.push_back(matrix);
    }

  public:

    /// default constructer
    L_system() = default;

    /// Parser function, used to read all info from txt file
    void loadFile(string name){
      app_utils::get_url(lSystemFile, name);

      printf("The file has been read\n");
      printf("%.*s\n", lSystemFile.size(), (char*)lSystemFile.data());
      fileSize = lSystemFile.size();

      removeWhiteSpace(lSystemFile);

      constructLSystem();
    }

    /// This function iterates the axiom the number of times given
    void iteration(int numb){
      for (int i = 0; i < numb; ++i){
        iterate();
      }
    }

    /// This function loops through the current axiom and adds vertexs to the mesh
    void calculateVertices(dynarray<char> &_axiom){

      initialiseDrawParams();

      mat4t matrix;
      for (int i = 0; i < _axiom.size(); i++)
      {
        switch (_axiom[i])
        {
        case 'F':
          if (LS_DEBUG_ITERATE) printf("Letter recognised as %c\n", _axiom[i]);
          vtx->pos = placementStack.back()[3].xyz();
          vtx++;
          placementStack.back().translate(translateF);
          vtx->pos = placementStack.back()[3].xyz();
          vtx++;
          break;
        case '[':
          if (LS_DEBUG_ITERATE) printf("Letter recognised as %c\n", _axiom[i]);
          matrix = placementStack.back();
          placementStack.push_back(matrix);
          break;
        case ']':
          if (LS_DEBUG_ITERATE) printf("Letter recognised as %c\n", _axiom[i]);
          placementStack.pop_back();
          break;
        case '+':
          if (LS_DEBUG_ITERATE) printf("Letter recognised as %c\n", _axiom[i]);
          placementStack.back().rotateZ(angle);
          break;
        case '-':
          if (LS_DEBUG_ITERATE) printf("Letter recognised as %c\n", _axiom[i]);
          placementStack.back().rotateZ(-angle);
          break;
        default:
          if (LS_DEBUG_ITERATE) printf("Letter not recognised: %c, therefore doing nothing\n", _axiom[i]);
          break;
        }
      }
    }

    /// Get functions below /// ---------------------------------------------------
    
    /// returns axiom's size
    int getAxiomSize(){
      return axiom.size();
    }

    /// returns the number of iterations
    int getIterations(){
      return iterations;
    }

    /// returns the scene node
    scene_node* getNode() {
      return node;
    }

    /// returns the mesh 
    mesh* getMesh() {
      return _mesh;
    }

    /// returns axiom if the index is bad then return the most up to date axiom
    dynarray<char> getAxiom(int index){
      if (index <= axiomlist.size() && index >= 0){
        return axiomlist[index];
      }
      return axiom;
    }
  };
}