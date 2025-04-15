#if !defined(SIMULATE_H)
#define SIMULATE_H

struct Reflection_Node {
  Reflection_Node *reflect_x = nullptr;
  Reflection_Node *reflect_z = nullptr;
  Mirror *mirror = nullptr;
};

struct Reflection_Graph {
  Auto_Array<Reflection_Node*> nodes;
  Auto_Array<Reflection_Node*> visiting;
};

#endif //SIMULATE_H
