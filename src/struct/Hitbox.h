#pragma once
#include "CoordinatePair.h"

struct Hitbox {
    double left;
    double top;
    double right;
    double bottom;

    Hitbox() {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
    }

    Hitbox(const Hitbox& other) {
        left = other.left;
        top = other.top;
        right = other.right;
        bottom = other.bottom;
    }

    template<typename T>
    Hitbox(const T& l, const T& t, const T& r, const T& b) {
        left = l;
        top = t;
        right = r;
        bottom = b;
    }

    template<typename T>
    Hitbox(CoordinatePair p, const T& w, const T& h, const bool& center = true) {
        if (center) {
            left = p.x - w / 2;
            top = p.y - h / 2;
        } else {
            left = p.x;
            top = p.y;
        }
        right = left + w;
        bottom = top + h;
    }

    template<typename T, typename U>
    Hitbox(sf::Vector2<U> p, const T& w, const T& h, const bool& center = true) {
        if (center) {
            left = p.x - w / 2;
            top = p.y - h / 2;
        } else {
            left = p.x;
            top = p.y;
        }
        right = left + w;
        bottom = top + h;
    }

    Hitbox(const CoordinatePair& tl, const CoordinatePair& br) {
        left = tl.x;
        top = tl.y;
        right = br.x;
        bottom = br.y;
    }

    template<typename T>
    Hitbox& add(const T& ox, const T& oy) {
        left += ox;
        top += oy;
        right += ox;
        bottom += oy;

        return *this;
    }

    Hitbox& add(const CoordinatePair& p) {
        return add(p.x, p.y);
    }

    template<typename T>
    Hitbox& add(const sf::Vector2<T>& p) {
        return add(p.x, p.y);
    }

    template<typename T>
    Hitbox& subtract(const T& ox, const T& oy) {
        return add(-ox, -oy);
    }

    Hitbox& subtract(const CoordinatePair& p) {
        return add(-p.x, -p.y);
    }

    template<typename T>
    Hitbox& subtract(const sf::Vector2<T>& p) {
        return add(-p.x, -p.y);
    }

    template<typename T>
    Hitbox& multiply(const T& ox, const T& oy) {
        CoordinatePair c = center();
        double w = width();
        double h = height();
        left = c.x - w * ox / 2;
        top = c.y - h * oy / 2;
        right = c.x + w * ox / 2;
        bottom = c.y + h * oy / 2;

        return *this;
    }

    Hitbox& multiply(const CoordinatePair& p) {
        return multiply(p.x, p.y);
    }

    template<typename T>
    Hitbox& multiply(const T& v) {
        return multiply(v, v);
    }

    Hitbox operator+(const CoordinatePair& p) {
        return Hitbox(*this).add(p);
    }

    Hitbox operator-(const CoordinatePair& p) {
        return Hitbox(*this).subtract(p);
    }

    Hitbox operator*(const CoordinatePair& p) {
        return Hitbox(*this).multiply(p);
    }

    template<typename T>
    Hitbox operator+(const sf::Vector2<T>& p) {
        return Hitbox(*this).add(p);
    }

    template<typename T>
    Hitbox operator-(const sf::Vector2<T>& p) {
        return Hitbox(*this).subtract(p);
    }

    template<typename T>
    Hitbox operator*(const sf::Vector2<T>& p) {
        return Hitbox(*this).multiply(p);
    }

    template<typename T>
    Hitbox operator*(const T& p) {
        return Hitbox(*this).multiply(p);
    }

    template<typename T>
    Hitbox& extend(const T& l, const T& t, const T& r, const T& b) {
        left -= l;
        right += r;
        top -= t;
        bottom += b;

        return *this;
    }

    template<typename T>
    Hitbox& extend(const T& ox, const T& oy) {
        return extend(ox, oy, ox, oy);
    }

    Hitbox extend(const CoordinatePair& p) {
        return extend(p.x, p.y, p.x, p.y);
    }

    template<typename T>
    Hitbox extend(const sf::Vector2<T>& p) {
        return extend(p.x, p.y, p.x, p.y);
    }

    template<typename T>
    Hitbox& extend(const T& v) {
        return extend(v, v, v, v);
    }

    double width() {
        return right - left;
    }

    double height() {
        return bottom - top;
    }

    CoordinatePair center() {
        return { (left + right) / 2, (top + bottom) / 2 };
    }

    template<typename T>
    bool contains(const T& ox, const T& oy) {
        return (ox >= left && ox <= right && oy >= top && oy <= bottom);
    }

    template<typename T>
    bool contains(const sf::Vector2<T>& p) {
        return contains(p.x, p.y);
    }

    bool contains(const CoordinatePair& p) {
        return contains(p.x, p.y);
    }

    bool overlaps(const Hitbox& other) {
        return left < other.right && right > other.left
            && top < other.bottom && bottom > other.top;
    }

    bool encloses(const Hitbox& other) {
        return contains(other.left, other.top) && contains(other.right, other.bottom);
    }

    sf::FloatRect toSfFloatRect() {
        return { (float)left, (float)top, (float)(right - left), (float)(bottom - top) };
    }
};
