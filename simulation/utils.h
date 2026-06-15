#ifndef FORCE_COMPUTERS_UTILS_H
#define FORCE_COMPUTERS_UTILS_H
#include "../math_utils.h"
#include "../simulation/bodies.h"
#include <memory>
#include <queue>
#include <vector>

// Use omp to search for min max x and y
template <typename T>
std::pair<Vec2<T>, Vec2<T>> findCorners(std::vector<CircleBody<T>> &bodies) {
  // TODO: later parallelize
  T pos_inf = std::numeric_limits<T>::infinity();
  T neg_inf = -std::numeric_limits<T>::infinity();

  double left_upper_y = std::numeric_limits<T>::infinity();
  double left_upper_x = std::numeric_limits<T>::infinity();

  double right_bottom_y = -std::numeric_limits<T>::infinity();
  double right_bottom_x = -std::numeric_limits<T>::infinity();

#pragma omp parallel for reduction(min : left_upper_x)                         \
    reduction(min : left_upper_y) reduction(max : right_bottom_y)              \
    reduction(max : right_bottom_x)
  for (auto &body : bodies) {
    Vec2<T> pos = body.getPos();
    if (pos.x < left_upper_x) {
      left_upper_x = pos.x;
    }
    if (pos.y < left_upper_y) {
      left_upper_y = pos.y;
    }
    if (pos.x > right_bottom_x) {
      right_bottom_x = pos.x;
    }
    if (pos.y > right_bottom_y) {
      right_bottom_y = pos.y;
    }
  }

  Vec2<T> left_upper{left_upper_x, left_upper_y};
  Vec2<T> right_bottom{right_bottom_x, right_bottom_y};

  return std::pair(left_upper, right_bottom);
};

template <typename T> struct QuadTree {
  enum Type { Aggregate, Empty, Leaf };

  Vec2<T> m_pos; // left upper corner (nw)
  T width;       // We assume it is always a square.
  Vec2<T> center_of_mass;
  std::vector<std::unique_ptr<QuadTree<T>>> children{}; // nw, ne, sw, se
  T total_mass{0};
  Type type{Empty}; // the type of the node.

  QuadTree(Vec2<T> pos, T width) : m_pos{pos}, width{width} {
    center_of_mass = m_pos + width / 2;
  }

  void insert(Vec2<T> new_pos, T mass,
              int depth = 0) { // we actually don't even need to
                               // store bodies here.

    // Upate the center of mass and total mass
    T prev_mass = total_mass;
    Vec2<T> prev_center_of_mass = center_of_mass;

    center_of_mass =
        (center_of_mass * total_mass + new_pos * mass) / (total_mass + mass);

    total_mass += mass;

    if (depth > 64)
      return;

    if (type == Empty) {
      type = Leaf;
    } else if (type == Leaf) {
      // split
      for (int y = 0; y <= 1;
           y++) { // Order is important for (nw, ne, sw, se) consistency
        for (int x = 0; x <= 1; x++) {
          Vec2<T> new_pos =
              Vec2<T>{m_pos.x + x * width / 2, m_pos.y + y * width / 2};
          children.push_back(std::make_unique<QuadTree<T>>(new_pos, width / 2));
        }
      }
      // find nearest node for both bodies.
      int y = prev_center_of_mass.y >= m_pos.y + width / 2 ? 1 : 0;
      int x = prev_center_of_mass.x >= m_pos.x + width / 2 ? 1 : 0;

      children.at(y * 2 + x)->insert(prev_center_of_mass, prev_mass, depth + 1);

      y = new_pos.y >= m_pos.y + width / 2 ? 1 : 0;
      x = new_pos.x >= m_pos.x + width / 2 ? 1 : 0;

      children.at(y * 2 + x)->insert(new_pos, mass, depth + 1);

      type = Aggregate;
    } else {
      // pass down
      int y = new_pos.y >= m_pos.y + width / 2 ? 1 : 0;
      int x = new_pos.x >= m_pos.x + width / 2 ? 1 : 0;
      children.at(y * 2 + x)->insert(new_pos, mass, depth + 1);
    }
  }

  Vec2<T> traverse(Vec2<T> check_pos,
                   double theta) { // basically calculates the field.
    // First check if it's the same body
    Vec2<T> direction{-center_of_mass + check_pos};
    T dist{direction.norm()};

    if (dist < 100) { // hardcoded for now. take radius in
      return {0, 0};
    }
    if (type == Empty) {
      return {0, 0};
    }

    T d3 = dist * dist * dist;

    if (type == Leaf) {
      return direction * total_mass / d3;
    }

    if (width / dist < theta)
      return direction * total_mass / d3;

    Vec2<T> field{0, 0};
    for (auto &child : children) {
      field += child->traverse(check_pos, theta);
    }
    return field;
  }
};

//---------------------------------------------------------------------------------------------
// Another implementation of QuadTree that uses arrays, may be useful in the
// future to improve performance
//---------------------------------------------------------------------------------------------

// template <typename T> struct QuadNode {
//   enum Type { Aggregate, Empty, Leaf };

//   Vec2<T> m_pos; // left upper corner (nw)
//   T width;       // We assume it is always a square.
//   Vec2<T> center_of_mass;
//   T total_mass{0};
//   Type type{Empty};
//   std::array<int, 4> children = {-1, -1, -1,
//                                  -1}; // The indexes of children nodes.
//   QuadNode() = default;
//   QuadNode(Vec2<T> pos, T width) : m_pos{pos}, width{width} {
//     center_of_mass = m_pos + width / 2;
//   }
// };

// template <typename T> struct QuadTree {
//   int m_root = 0;
//   int m_next = 1;
//   std::vector<QuadNode<T>>
//       m_nodes; // This is better for cache locality than just having
//       pointers.

//   QuadTree(int n, Vec2<T> upper_left_corner, T width) {
//     m_nodes.resize(4 * n);
//     // create root node at 0
//     m_nodes[m_root] = QuadNode<T>{upper_left_corner, width};
//   }

//   int newNodeIndex(Vec2<T> pos, T width) {
//     if (m_next >= m_nodes.size()) {
//       m_nodes.resize(m_nodes.size() * 2);
//     }
//     m_nodes[m_next] = QuadNode<T>{pos, width};
//     return m_next++;
//   }

//   void insert(Vec2<T> new_pos, T mass, int index = 0, int depth = 0) {
//     // QuadNode<T> &node = m_nodes[index];
//     T prev_mass = m_nodes[index].total_mass;
//     Vec2<T> prev_center_of_mass = m_nodes[index].center_of_mass;

//     m_nodes[index].center_of_mass =
//         (m_nodes[index].center_of_mass * m_nodes[index].total_mass +
//          new_pos * mass) /
//         (m_nodes[index].total_mass + mass);

//     m_nodes[index].total_mass += mass;

//     if (depth > 64)
//       return;

//     if (m_nodes[index].type == QuadNode<T>::Type::Empty) {
//       m_nodes[index].type = QuadNode<T>::Type::Leaf;
//     } else if (m_nodes[index].type == QuadNode<T>::Type::Leaf) {
//       // split
//       for (int y = 0; y <= 1;
//            y++) { // Order is important for (nw, ne, sw, se) consistency
//         for (int x = 0; x <= 1; x++) {
//           Vec2<T> new_pos =
//               Vec2<T>{m_nodes[index].m_pos.x + x * m_nodes[index].width / 2,
//                       m_nodes[index].m_pos.y + y * m_nodes[index].width / 2};

//           m_nodes[index].children[y * 2 + x] =
//               newNodeIndex(new_pos, m_nodes[index].width / 2);
//         }
//       }
//       // find nearest node for both bodies.
//       int y = prev_center_of_mass.y >=
//                       m_nodes[index].m_pos.y + m_nodes[index].width / 2
//                   ? 1
//                   : 0;
//       int x = prev_center_of_mass.x >=
//                       m_nodes[index].m_pos.x + m_nodes[index].width / 2
//                   ? 1
//                   : 0;

//       int idx = m_nodes[index].children[y * 2 + x];
//       insert(prev_center_of_mass, prev_mass, idx, depth + 1);

//       y = new_pos.y >= m_nodes[index].m_pos.y + m_nodes[index].width / 2 ? 1
//                                                                          : 0;
//       x = new_pos.x >= m_nodes[index].m_pos.x + m_nodes[index].width / 2 ? 1
//                                                                          : 0;

//       idx = m_nodes[index].children[y * 2 + x];

//       insert(new_pos, mass, idx, depth + 1);

//       m_nodes[index].type = QuadNode<T>::Type::Aggregate;
//     } else {
//       // pass down
//       int y = new_pos.y >= m_nodes[index].m_pos.y + m_nodes[index].width / 2
//                   ? 1
//                   : 0;
//       int x = new_pos.x >= m_nodes[index].m_pos.x + m_nodes[index].width / 2
//                   ? 1
//                   : 0;
//       int idx = m_nodes[index].children[y * 2 + x];

//       insert(new_pos, mass, idx, depth + 1);
//     }
//   }

//   Vec2<T> traverse(Vec2<T> check_pos,
//                    int index = 0) { // basically calculates the field.
//     // First check if it's the same body
//     if (index == -1) {
//       return {0, 0};
//     }
//     // QuadNode<T> node = m_nodes.at(index);
//     Vec2<T> direction{-m_nodes[index].center_of_mass + check_pos};
//     T dist{direction.norm()};

//     if (dist < 100) { // hardcoded for now. take radius in
//       return {0, 0};
//     }
//     if (m_nodes[index].type == QuadNode<T>::Type::Empty) {
//       return {0, 0};
//     }

//     T d3 = dist * dist * dist;

//     if (m_nodes[index].type == QuadNode<T>::Type::Leaf) {
//       return direction * m_nodes[index].total_mass / d3;
//     }

//     if (m_nodes[index].width / dist < 0.5)
//       return direction * m_nodes[index].total_mass / d3;

//     Vec2<T> field{0, 0};
//     for (auto &child : m_nodes[index].children) {
//       field += traverse(check_pos, child);
//     }
//     return field;
//   }
// };

#endif