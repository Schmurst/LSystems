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

    // l System variables
    int fileSize;
    octet::string message;            // contains the message
    dynarray<uint8_t> lSystemFile;    // containts the entire file
    dynarray<char> alphabet;          // contains the alphabet
    dynarray<char> axiom;             // contains the axiom
    char startingAxiom;               // axiom from file
    hash_map<char, string> rules;     // contains the rules
    float angle;                      // angle used for rotation
    int iteration_count;

    // drawing variables
    ref<scene_node> node;
    ref<mesh> _mesh;
    myVertex *vtx;
    int numVtxs;
    uint32_t *idx;
    dynarray<mat4t> placementStack;
    vec3 translateF = vec3(0, 1.0f, 0);
    float radius = 0.2f;
    const int VERTSPERFACE = 3;

    // random generation
    random randNumGen;
    bool isStochastic = false;
    float varience = 0.10f;   // maximum percentage variation from original value 
    int rand = 0;

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
      ++iteration_count;
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
          startingAxiom = lSystemFile[i];
      }

      axiom.push_back(startingAxiom);

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
    }

    /// this function returns a randomised slighlty altered copy of the original
    float mutateFloat(float input){
      float min = input * (1 - varience);
      float max = input * (1 + varience);
      return randNumGen.get(min, max);
    }

  public:

    /// default constructer
    L_system(){
      node = new scene_node();
      _mesh = new mesh();
      placementStack.push_back(mat4t());
      numVtxs = 0;
      iteration_count = 0;
      randNumGen.set_seed(1);
    }

    /// These functions are to be used with the new framework ----------------------

    /// this function calculates the vertices of a prism given a matrix from the matrix stack 
    /// This code has been taken from Andy's geometery example and modified
    void calculate_prism_vertices(mat4t &placement){

      vec3 pos0 = placement[3].xyz();
      if (isStochastic){
        vec3 temp = translateF;
        temp[1] = mutateFloat(temp[1]);
        placement.translate(temp);
      }
      else{
        placement.translate(translateF);
      }
      vec3 pos1 = placement[3].xyz();

      mat4t rotation = placement.xyz();

      for (size_t i = 0; i < VERTSPERFACE; ++i) {
        float r = 0.0f, g = 1.0f * i / VERTSPERFACE, b = 1.0f;
        float theta = i * 2.0f * 3.14159265f / VERTSPERFACE;
        vtx->pos = pos0 + vec3p(cosf(theta) * radius, 0, sinf(theta) * radius) * rotation;
        vtx->colour = make_color(r, g, b);
        vtx++;
        vtx->pos = pos1 + vec3p(cosf(theta) * radius, 0, sinf(theta) * radius) * rotation;
        vtx->colour = make_color(r, g, b);
        vtx++;
      }

 
      // make the triangles
      uint32_t vn = 0;
      for (size_t i = 0; i != VERTSPERFACE; ++i) {
        /*
        1---------3
        |       / |
        |    /    |   
        | /       |
        0---------2
        */   

        idx[0] = vn + 0 + numVtxs;
        idx[1] = ((vn + 3) % 6) + numVtxs;
        idx[2] = vn + 1 + numVtxs;
        idx += 3;
        // printf("%u,%u,%u :", idx[-3], idx[-2], idx[-1]);
        idx[0] = vn + 0 + numVtxs;
        idx[1] = ((vn + 2) % 6) + numVtxs;
        idx[2] = ((vn + 3) % 6) + numVtxs;
        idx += 3;
       // printf(" %u,%u,%u\n", idx[-3], idx[-2], idx[-1]);
        vn += 2;
      }
      // this is number of vertices added
      numVtxs += 6;

    }

    /// This function interprets the axiom to populate the mesh
    void interpret_axiom(){

      numVtxs = 0;
      mat4t matrix;
      
      // for each char in axiom do x
      for each (char c in axiom)
      {
        switch (c)
        {
        case 'F':
          // draw a prism
          calculate_prism_vertices(placementStack.back());
          break;
        case '[':
          // push a matrix onto the stack
          matrix = placementStack.back();
          placementStack.push_back(matrix);
          break;
        case ']':
          // pop a matrix off the stack
          placementStack.pop_back();
          break;
        case '+':
          // rotate around z +ve
          if (!isStochastic){
            placementStack.back().rotateZ(angle);
          }
          else{
            placementStack.back().rotateX(mutateFloat(angle));
            placementStack.back().rotateY(mutateFloat(angle));
            placementStack.back().rotateZ(mutateFloat(angle));
          }
          break;
        case '-':
          // rotate around z -ve
          if (!isStochastic){
            placementStack.back().rotateZ(-angle);
          }
          else{
            placementStack.back().rotateX(mutateFloat(-angle));
            placementStack.back().rotateY(mutateFloat(-angle));
            placementStack.back().rotateZ(mutateFloat(-angle));
          }
          break;
        default:
          break;
        }
        
      }
    }

    /// This fucntion sets up th mesh to be drawn, taken and edited from Andy's geometry example
    void initialiseDrawParams() {

      int num = 0;
      for (int i = 0; i < axiom.size(); ++i){
        if (axiom[i] == 'F') {
          ++num;
        }
      }

      _mesh->init();

      // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      // allocate vertices and indices into OpenGL buffers
      size_t num_vertices = num * 6;
      size_t num_indices = num * 6 * VERTSPERFACE;
      _mesh->allocate(sizeof(myVertex) * num_vertices, sizeof(uint32_t) * num_indices);
      _mesh->set_params(sizeof(myVertex), num_indices, num_vertices, GL_TRIANGLES, GL_UNSIGNED_INT);

      // describe the structure of my_vertex to OpenGL
      _mesh->add_attribute(attribute_pos, 3, GL_FLOAT, 0);
      _mesh->add_attribute(attribute_color, 4, GL_UNSIGNED_BYTE, 12, GL_TRUE);


      // these write-only locks give access to the vertices and indices.
      // they will be released at the next } (the end of the scope)
      gl_resource::wolock vl(_mesh->get_vertices());
      vtx = (myVertex *)vl.u8();
      gl_resource::wolock il(_mesh->get_indices());
      idx = il.u32();
    }

    /// ----------------------------------------------------------------------------

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

    /// Get functions below /// ==================================================================

    /// returns axiom's size
    int getAxiomSize(){
      return axiom.size();
    }

    /// returns the scene node
    scene_node* getNode() {
      return node;
    }

    /// returns the mesh 
    mesh* getMesh() {
      return _mesh;
    }

    /// increments current iterations
    void incrementIteration(){
      placementStack.reset();
      placementStack.push_back(mat4t());
      iterate();
      initialiseDrawParams();
      interpret_axiom();
    }

    /// increments current iterations
    void incrementRadius(){
      radius += 0.05f;
      placementStack.reset();
      placementStack.push_back(mat4t());
      initialiseDrawParams();
      interpret_axiom();
    }

    /// increments current iterations
    void decrementRadius(){
      radius -= 0.05f;
      placementStack.reset();
      placementStack.push_back(mat4t());
      initialiseDrawParams();
      interpret_axiom();
    }

    /// deccrements current iterations
    void decrementIteration(){
      int target = (iteration_count != 1) ? iteration_count - 1 : 0;
      iteration_count = 0;
      axiom.reset();
      axiom.resize(1);
      axiom.push_back(startingAxiom);
      placementStack.reset();
      placementStack.push_back(mat4t());
      iteration(target);
      initialiseDrawParams();
      interpret_axiom();
    }

    /// increment angle by 1degree
    void incrementAngle(){
      angle += 1.0f;
      placementStack.reset();
      placementStack.push_back(mat4t());
      initialiseDrawParams();
      interpret_axiom();
    }

    /// decrment angle by 1.0f degrees
    void decrementAngle() {
      angle -= 1.0f;
      placementStack.reset();
      placementStack.push_back(mat4t());
      initialiseDrawParams();
      interpret_axiom();
    }

    /// increment the translation magnitude
    void incrementTranslation(){
      translateF[1] += 0.02f;
      placementStack.reset();
      placementStack.push_back(mat4t());
      initialiseDrawParams();
      interpret_axiom();
    }

    /// increment the translation magnitude
    void decrementTranslation(){
      translateF[1] -= 0.02f;
      placementStack.reset();
      placementStack.push_back(mat4t());
      initialiseDrawParams();
      interpret_axiom();
    }

    /// change the generation to and from stochastic
    void altStochasticity(){
      isStochastic = !isStochastic;
      placementStack.reset();
      placementStack.push_back(mat4t());
      initialiseDrawParams();
      interpret_axiom();
    }
  };
}