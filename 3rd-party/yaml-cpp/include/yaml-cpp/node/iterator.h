/*
Copyright (c) 2008-2015 Jesse Beder.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef VALUE_ITERATOR_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define VALUE_ITERATOR_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) ||                                            \
    (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || \
     (__GNUC__ >= 4))  // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/detail/iterator_fwd.h"
#include "yaml-cpp/node/detail/iterator.h"
#include <list>
#include <utility>
#include <vector>

namespace YAML {
namespace detail {
struct iterator_value : public Node, std::pair<Node, Node> {
  iterator_value() {}
  explicit iterator_value(const Node& rhs)
      : Node(rhs),
        std::pair<Node, Node>(Node(Node::ZombieNode), Node(Node::ZombieNode)) {}
  explicit iterator_value(const Node& key, const Node& value)
      : Node(Node::ZombieNode), std::pair<Node, Node>(key, value) {}
};
}
}

#endif  // VALUE_ITERATOR_H_62B23520_7C8E_11DE_8A39_0800200C9A66
