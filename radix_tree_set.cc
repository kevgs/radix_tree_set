#include <algorithm>
#include <cassert>
#include <fstream>
#include <memory>
#include <vector>

using namespace std;

class RadixTreeSet {
 public:
  size_t size() const { return size_; }

  bool Insert(const string &key) { return Insert(begin(key), end(key)); }

  bool Insert(string::const_iterator b, string::const_iterator e) {
    auto *node = &root_;
    while (node != nullptr) {
      auto it = lower_bound(begin(*node), end(*node), *b);

      // Nodes not matched. Add a new node to the right.
      if (it == end(*node)) {
        node->emplace_back(string(b, e), true);
        size_++;
        return true;
      }

      auto mis = mismatch(b, e, begin((*it)->key));

      // Not a single char matched. Add a new node here.
      if (mis.second == begin((*it)->key)) {
        node->emplace(it, string(b, e), true);
        size_++;
        return true;
      }

      // Node partially matched. Split it.
      if (mis.second != end((*it)->key)) {
        auto new_node = make_unique<Node>(string(mis.second, end((*it)->key)),
                                          (*it)->is_leaf);
        new_node->children = move((*it)->children);
        (*it)->key.erase(mis.second, end((*it)->key));
        (*it)->is_leaf = false;
        (*it)->push_back(move(new_node));

        // Added record by splitting an existent node.
        if (mis.first == e) {
          (*it)->is_leaf = true;
          size_++;
          return true;
        }
      }

      // Node matched. Move to the next one.
      b = mis.first;
      node = it->get();
    }

    return false;
  }

  bool Find(const string &key) const { return Find(begin(key), end(key)); }

  // Empty string is always inside.
  bool Find(string::const_iterator b, string::const_iterator e) const {
    auto *node = &root_;
    while (node != nullptr) {
      auto it = lower_bound(begin(*node), end(*node), *b);

      // No match.
      if (it == end(*node)) {
        return false;
      }

      auto mis = mismatch(b, e, begin((*it)->key));

      // Extra suffix in a tree node. No match.
      if (mis.second != end((*it)->key)) {
        return false;
      }

      // Key fully matched.
      if (mis.first == e && (*it)->is_leaf) {
        return true;
      }

      // Check the next node.
      b = mis.first;
      node = it->get();
    }

    return false;
  }

 private:
  struct Node {
    using iterator = vector<unique_ptr<Node>>::iterator;
    using const_iterator = vector<unique_ptr<Node>>::const_iterator;

    Node() {}
    Node(string key, bool is_leaf = false) : key{move(key)}, is_leaf{is_leaf} {}

    iterator begin() { return children.begin(); }
    iterator end() { return children.end(); }
    const_iterator begin() const { return children.begin(); }
    const_iterator end() const { return children.end(); }

    void push_back(unique_ptr<Node> &&value) {
      children.push_back(move(value));
    }

    template <class... Args>
    void emplace_back(Args &&... args) {
      children.push_back(make_unique<Node>(forward<Args>(args)...));
    }

    template <class... Args>
    iterator emplace(iterator pos, Args &&... args) {
      return children.emplace(pos, make_unique<Node>(forward<Args>(args)...));
    }

    string key;
    vector<unique_ptr<Node>> children;
    bool is_leaf;
  };

  Node root_;
  size_t size_{0};

  friend bool operator<(const unique_ptr<Node> &lhs, char rhs);
};

bool operator<(const unique_ptr<RadixTreeSet::Node> &lhs, char rhs) {
  return lhs->key[0] < rhs;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    return -1;
  }
  auto dictionary_path = argv[1];

  RadixTreeSet set;
  {
    ifstream in(dictionary_path);
    string s;
    while (getline(in, s)) {
      auto ret = set.Insert(s);
      assert(ret);
      (void)ret;
    }
  }
  {
    ifstream in(dictionary_path);
    string s;
    while (getline(in, s)) {
      auto ret = set.Find(s);
      assert(ret);
      (void)ret;
    }
  }

  return 0;
}
