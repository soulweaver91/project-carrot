#pragma once

#include <memory>
#include <utility>

template <typename T>
class SequentialIDList {
public:
    typedef QMap<int, std::shared_ptr<T>> listType;
    typedef typename listType::iterator iterator;
    typedef typename listType::const_iterator constIter;

    SequentialIDList() : nextID(0) {

    };

    template<typename... P>
    int insertAndGetID(P&&... args) {
        int id = nextID;
        items.insert(nextID++, std::make_shared<T>(std::forward<P>(args)...));
        return id;
    };

    template<typename... P>
    std::shared_ptr<T> insertAndGetItem(P&&... args) {
        return items.insert(nextID++, std::make_shared<T>(std::forward<P>(args)...));
    };

    std::shared_ptr<T> at(int i) {
        return items.value(i);
    };

    void remove(int i) {
        items.remove(i);
    };

    iterator begin() {
        return items.begin();
    }

    iterator end() {
        return items.end();
    }

    constIter begin() const {
        return items.begin();
    }

    constIter end() const {
        return items.end();
    }

    constIter cbegin() const {
        return items.cbegin();
    }

    constIter cend() const {
        return items.cend();
    }

private:
    int nextID;
    listType items;
};
