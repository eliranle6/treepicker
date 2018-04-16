namespace treelib {

template <class T>
class Node
{
public:
    Node(std::string _tag, std::string _identifier, T _data, std::string parent);

    std::string tag;
    std::string identifier;
    std::string parent;
    T data;
    std::vector< Node<T> * > children;
    void add_child(Node<T> *node);

private:
    Node *parent_node;
};

template <class T>
Node<T>::Node(std::string _tag, std::string _identifier, T _data, std::string _parent) {
    tag = _tag;
    identifier = _identifier;
    data = _data;
    parent = _parent;
}

template <class T>
void Node<T>::add_child(Node<T> *child) {
    children.insert(children.begin(), child);
}

}
